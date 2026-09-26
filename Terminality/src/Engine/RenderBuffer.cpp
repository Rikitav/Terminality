
#include <cstdint>
#include <string>
#include <algorithm>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <mutex>

#include <terminality/Engine/RenderBuffer.hpp>

#if defined(__SSE2__) || defined(_M_X64) || (defined(_M_IX86_FP) && _M_IX86_FP >= 2)
	#define TERMINALITY_SIMD_SSE2 1
	#include <emmintrin.h>
#else
	#define TERMINALITY_SIMD_SSE2 0
#endif

using namespace terminality;

namespace
{
	// Written as an expression (not Color::TRANSPARENT) because the Windows
	// GDI header defines TRANSPARENT as a macro; in the single-header
	// amalgamation Windows.h precedes this code in the same translation unit.
	// AsTransparent() is private, so flip the flag directly.
	const Color TransparentCellColor = []
	{
		Color color(0, 0, 0);
		color.Transparent = true;
		return color;
	}();
}

bool RenderBuffer::TrueColorOutput = true;

CellInfo::CellInfo(wchar_t symbol, Color fore, Color back)
	: Symbol(symbol), Fore(fore), Back(back) { }

bool CellInfo::operator==(const CellInfo& other) const
{
	return Symbol == other.Symbol && Fore == other.Fore && Back == other.Back;
}

bool CellInfo::operator!=(const CellInfo& other) const
{
	return !(*this == other);
}

namespace
{
	// Maps an RGB color to the 256-color palette (6x6x6 cube + gray ramp),
	// the same scheme btop uses for its non-truecolor themes.
	int ColorTo256(const Color& color)
	{
		if (color.R == color.G && color.G == color.B)
			return 232 + (color.R + 5) / 11;

		return 16 + ((color.R + 25) / 51) * 36 + ((color.G + 25) / 51) * 6 + (color.B + 25) / 51;
	}
}

#if TERMINALITY_SIMD_SSE2
// Raw 16-byte compare of two cells. Sound because CellInfo::Padding is
// value-initialized to zero on every construction and never written afterward,
// so the padding lanes are always equal on both sides.
static inline bool CellsEqualSimd(const CellInfo& a, const CellInfo& b)
{
	const __m128i va = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&a));
	const __m128i vb = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&b));
	return _mm_movemask_epi8(_mm_cmpeq_epi32(va, vb)) == 0xFFFF;
}

// True when four consecutive cells are all unchanged vs. the snapshot.
static inline bool CellsUnchanged4(const CellInfo* cell, const CellInfo* snap)
{
	const __m128i d0 = _mm_cmpeq_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(cell + 0)), _mm_loadu_si128(reinterpret_cast<const __m128i*>(snap + 0)));
	const __m128i d1 = _mm_cmpeq_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(cell + 1)), _mm_loadu_si128(reinterpret_cast<const __m128i*>(snap + 1)));
	const __m128i d2 = _mm_cmpeq_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(cell + 2)), _mm_loadu_si128(reinterpret_cast<const __m128i*>(snap + 2)));
	const __m128i d3 = _mm_cmpeq_epi32(_mm_loadu_si128(reinterpret_cast<const __m128i*>(cell + 3)), _mm_loadu_si128(reinterpret_cast<const __m128i*>(snap + 3)));
	const __m128i all = _mm_and_si128(_mm_and_si128(d0, d1), _mm_and_si128(d2, d3));
	return _mm_movemask_epi8(all) == 0xFFFF;
}
#endif

void RenderBuffer::AppendAnsiBg(std::wstring& out, const Color& color)
{
	if (color.Transparent)
		return;

	if (!TrueColorOutput)
	{
		out += L"\x1b[48;5;";
		out += std::to_wstring(ColorTo256(color));
		out += L'm';
		return;
	}

	out += L"\x1b[48;2;";
	out += std::to_wstring(color.R);
	out += L';';
	out += std::to_wstring(color.G);
	out += L';';
	out += std::to_wstring(color.B);
	out += L'm';
}

void RenderBuffer::AppendAnsiFg(std::wstring& out, const Color& color)
{
	if (color.Transparent)
		return;

	if (!TrueColorOutput)
	{
		out += L"\x1b[38;5;";
		out += std::to_wstring(ColorTo256(color));
		out += L'm';
		return;
	}

	out += L"\x1b[38;2;";
	out += std::to_wstring(color.R);
	out += L';';
	out += std::to_wstring(color.G);
	out += L';';
	out += std::to_wstring(color.B);
	out += L'm';
}

RenderBuffer::RenderBuffer(uint32_t initialWidth, uint32_t initialHeight)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	buffer.assign(MAX_WIDTH * MAX_HEIGHT, CellInfo(L' ', TransparentCellColor, TransparentCellColor));
	snapshotBuffer.assign(MAX_WIDTH * MAX_HEIGHT, CellInfo(L' ', TransparentCellColor, TransparentCellColor));

	width = std::min(initialWidth, static_cast<uint32_t>(MAX_WIDTH));
	height = std::min(initialHeight, static_cast<uint32_t>(MAX_HEIGHT));

	snapshotWidth = width;
	snapshotHeight = height;
	dirtyRect = Rect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
}

uint32_t RenderBuffer::Width() const
{
	return width;
}

uint32_t RenderBuffer::Height() const
{
	return height;
}

void RenderBuffer::Resize(uint32_t newWidth, uint32_t newHeight)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	width = std::min(newWidth, static_cast<uint32_t>(MAX_WIDTH));
	height = std::min(newHeight, static_cast<uint32_t>(MAX_HEIGHT));

	dirtyRect = Rect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
}

void RenderBuffer::Clear(const CellInfo& cell)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	std::fill(buffer.begin(), buffer.end(), cell);
	MarkDirty(Rect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height)));
}

void RenderBuffer::SetCell(uint32_t x, uint32_t y, const CellInfo& cell)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	if (x >= width || y >= height)
		return;

	CellInfo& target = buffer[GetIndex(x, y)];
	bool changed = false;

	if (cell.Symbol != L'\0' && target.Symbol != cell.Symbol)
	{
		target.Symbol = cell.Symbol;
		changed = true;
	}

	if (!cell.Fore.Transparent && target.Fore != cell.Fore)
	{
		target.Fore = cell.Fore;
		changed = true;
	}

	if (!cell.Back.Transparent && target.Back != cell.Back)
	{
		target.Back = cell.Back;
		changed = true;
	}

	if (changed)
		MarkDirty(Rect(static_cast<int32_t>(x), static_cast<int32_t>(y), 1, 1));
}

const CellInfo& RenderBuffer::GetCell(uint32_t x, uint32_t y) const
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	if (x >= width || y >= height)
		throw std::out_of_range("RenderBuffer::GetCell coordinates out of range");

	return buffer.at(GetIndex(x, y));
}

void RenderBuffer::Snapshot()
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	// Copy only the used rows (GetIndex strides by MAX_WIDTH) instead of the
	// whole MAX_WIDTH x MAX_HEIGHT backing store.
	if (width > 0 && height > 0)
	{
		const std::size_t usedCells =
			(static_cast<std::size_t>(height) - 1) * MAX_WIDTH + width;
		std::memcpy(snapshotBuffer.data(), buffer.data(), usedCells * sizeof(CellInfo));
	}

	snapshotWidth = width;
	snapshotHeight = height;
	dirtyRect.reset();
}

void RenderBuffer::BulkRender(std::wostream& out)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	if (!dirtyRect)
		return;

	std::wstring output;
	output.reserve(width * height * 24);

	output += L"\x1b[H";
	std::optional<Color> currentFore;
	std::optional<Color> currentBack;

	for (uint32_t y = 0; y < height; ++y)
	{
		output += L"\x1b[";
		output += std::to_wstring(y + 1);
		output += L";1H";

		for (uint32_t x = 0; x < width; ++x)
		{
			const size_t idx = GetIndex(x, y);
			const CellInfo& cell = buffer[idx];

			if (!currentFore.has_value() || *currentFore != cell.Fore)
			{
				AppendAnsiFg(output, cell.Fore);
				currentFore = cell.Fore;
			}

			if (!currentBack.has_value() || *currentBack != cell.Back)
			{
				AppendAnsiBg(output, cell.Back);
				currentBack = cell.Back;
			}

			output += cell.Symbol;
		}

		if (y < height - 1)
			output += L"\r\n";
	}

	out.write(output.data(), output.size());
	out.flush();

	Snapshot();
	dirtyRect.reset();
}

void RenderBuffer::DiffRender(std::wostream& out)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	if (snapshotWidth != width || snapshotHeight != height)
	{
		BulkRender(out);
		return;
	}

	if (!dirtyRect || dirtyRect->IsEmpty())
		return;

	const uint32_t startX = static_cast<uint32_t>(std::max(0, dirtyRect->X));
	const uint32_t startY = static_cast<uint32_t>(std::max(0, dirtyRect->Y));
	const uint32_t endX = static_cast<uint32_t>(std::min<int32_t>(width, dirtyRect->Right()));
	const uint32_t endY = static_cast<uint32_t>(std::min<int32_t>(height, dirtyRect->Bottom()));

	std::wstring output;
	output.reserve(8192);

	std::optional<Color> currentFore;
	std::optional<Color> currentBack;
	uint32_t expectedX = static_cast<uint32_t>(-1);
	uint32_t expectedY = static_cast<uint32_t>(-1);

	for (uint32_t y = startY; y < endY; ++y)
	{
		for (uint32_t x = startX; x < endX; ++x)
		{
			const size_t idx = GetIndex(x, y);

#if TERMINALITY_SIMD_SSE2
			// Fast-skip runs of unchanged cells four at a time.
			if (x + 4 <= endX && CellsUnchanged4(&buffer[idx], &snapshotBuffer[idx]))
			{
				x += 3; // loop increment lands on x + 4
				continue;
			}
#endif

			const CellInfo& cell = buffer[idx];
			if (cell == snapshotBuffer[idx])
				continue;

			if (expectedX != x || expectedY != y)
			{
				output += L"\x1b[";
				output += std::to_wstring(y + 1);
				output += L";";
				output += std::to_wstring(x + 1);
				output += L"H";
			}

			if (!currentFore.has_value() || *currentFore != cell.Fore)
			{
				AppendAnsiFg(output, cell.Fore);
				currentFore = cell.Fore;
			}

			if (!currentBack.has_value() || *currentBack != cell.Back)
			{
				AppendAnsiBg(output, cell.Back);
				currentBack = cell.Back;
			}

			output += cell.Symbol;
			
			expectedX = x + 1;
			expectedY = y;
		}
	}

	out << std::nounitbuf;
	out.write(output.data(), output.size());
	out.flush();

	Snapshot();
	dirtyRect.reset();
}

size_t RenderBuffer::GetIndex(uint32_t x, uint32_t y) const
{
	return static_cast<std::size_t>(y) * MAX_WIDTH + x;
}

void RenderBuffer::MarkDirty(const Rect& rect)
{
	if (rect.IsEmpty())
		return;

	if (!dirtyRect)
	{
		dirtyRect = rect;
		return;
	}

	dirtyRect = Rect::Union(*dirtyRect, rect);
}

#undef TERMINALITY_SIMD_SSE2
