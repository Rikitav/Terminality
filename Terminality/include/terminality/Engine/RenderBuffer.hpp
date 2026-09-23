#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <optional>

#include <terminality/Core/Color.hpp>
#include <terminality/Core/Geometry.hpp>

namespace terminality
{
	// Laid out as a 16-byte POD (Symbol + Fore + Back + zeroed padding) so the
	// renderer can compare whole cells with a single SIMD register.
	struct alignas(16) CellInfo
	{
		wchar_t Symbol = L' ';
		Color Fore = Color::WHITE;
		Color Back = Color::BLACK;

		// Never read by user code; always zero so raw 16-byte SIMD compares
		// (see RenderBuffer::DiffRender) match member-wise equality.
		uint32_t Padding = 0;

		CellInfo() = default;

		CellInfo(wchar_t symbol, Color fore = Color::WHITE, Color back = Color::BLACK)
			: Symbol(symbol), Fore(fore), Back(back) { }

		bool operator==(const CellInfo& other) const
		{
			return Symbol == other.Symbol && Fore == other.Fore && Back == other.Back;
		}

		bool operator!=(const CellInfo& other) const
		{
			return !(*this == other);
		}
	};

	class RenderBuffer
	{
		friend class RenderContext;

		mutable std::recursive_mutex renderMutex;

		uint32_t snapshotWidth = 0;
		uint32_t snapshotHeight = 0;
		std::vector<CellInfo> snapshotBuffer;

		uint32_t width = 0;
		uint32_t height = 0;
		std::vector<CellInfo> buffer;

		std::optional<Rect> dirtyRect;

		std::size_t GetIndex(uint32_t x, uint32_t y) const;
		void MarkDirty(const Rect& rect);

	public:
		static constexpr std::size_t MAX_WIDTH = 512;
		static constexpr std::size_t MAX_HEIGHT = 256;

		RenderBuffer(uint32_t initialWidth, uint32_t initialHeight);

		uint32_t Width() const { return width; }
		uint32_t Height() const { return height; }
		void Resize(uint32_t newWidth, uint32_t newHeight);
		void Clear(const CellInfo& cell = CellInfo());

		void SetCell(uint32_t x, uint32_t y, const CellInfo& cell);
		const CellInfo& GetCell(uint32_t x, uint32_t y) const;

		void Snapshot();
		void DiffRender(std::wostream& out);
		void BulkRender(std::wostream& out);

		static void AppendAnsiBg(std::wstring& out, const Color& color);
		static void AppendAnsiFg(std::wstring& out, const Color& color);
	};
}
