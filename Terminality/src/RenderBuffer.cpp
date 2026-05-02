module terminality;

import std;
import std.compat;

using namespace terminality;

const wchar_t* RenderBuffer::GetAsniBg(Color color)
{
	switch (color)
	{
		case Color::BLACK:          return L"\x1b[40m";
		case Color::DARK_CYAN:      return L"\x1b[46m";
		case Color::CYAN:           return L"\x1b[106m";
		case Color::GREEN:          return L"\x1b[42m";
		case Color::DARK_YELLOW:    return L"\x1b[43m";
		case Color::YELLOW:         return L"\x1b[103m";
		case Color::MAGENTA:        return L"\x1b[45m";
		case Color::WHITE:          return L"\x1b[47m";
		case Color::DARK_GRAY:      return L"\x1b[100m";
		default:                    return L"\x1b[40m";
	}
}

const wchar_t* RenderBuffer::GetAsniFg(Color color)
{
    switch (color)
    {
        case Color::BLACK:          return L"\x1b[30m";
        case Color::DARK_CYAN:      return L"\x1b[36m";
        case Color::CYAN:           return L"\x1b[96m";
        case Color::GREEN:          return L"\x1b[32m";
        case Color::DARK_YELLOW:    return L"\x1b[33m";
        case Color::YELLOW:         return L"\x1b[93m";
        case Color::MAGENTA:        return L"\x1b[95m";
        case Color::WHITE:          return L"\x1b[37m";
        case Color::DARK_GRAY:      return L"\x1b[90m";
        default:                    return L"\x1b[37m";
    }
}

RenderBuffer::RenderBuffer(uint32_t initialWidth, uint32_t initialHeight)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	buffer.assign(MAX_WIDTH * MAX_HEIGHT, CellInfo());
	snapshotBuffer.assign(MAX_WIDTH * MAX_HEIGHT, CellInfo());

	width = std::min(initialWidth, static_cast<uint32_t>(MAX_WIDTH));
	height = std::min(initialHeight, static_cast<uint32_t>(MAX_HEIGHT));

	snapshotWidth = width;
	snapshotHeight = height;
	dirtyRect = Rect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
}

void RenderBuffer::Resize(uint32_t newWidth, uint32_t newHeight)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	width = std::min(newWidth, static_cast<uint32_t>(MAX_WIDTH));
	height = std::min(newHeight, static_cast<uint32_t>(MAX_HEIGHT));

	dirtyRect = Rect(0, 0, static_cast<int32_t>(width), static_cast<int32_t>(height));
	hasDirtyRect = true;
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
	if (target != cell)
	{
		target = cell;
		MarkDirty(Rect(static_cast<int32_t>(x), static_cast<int32_t>(y), 1, 1));
	}
}

const CellInfo& RenderBuffer::GetCell(uint32_t x, uint32_t y) const
{
	if (x >= width || y >= height)
		throw std::out_of_range("RenderBuffer::GetCell coordinates out of range");

	return buffer.at(GetIndex(x, y));
}

void RenderBuffer::Snapshot()
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	snapshotBuffer = buffer;
	snapshotWidth = width;
	snapshotHeight = height;
	hasDirtyRect = false;
	dirtyRect = Rect();
}

void RenderBuffer::BulkRender(std::wostream& out)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);

	if (!hasDirtyRect)
		return;

	std::wstring output;
	output.reserve(width * height * 24);

	output += L"\x1b[H";
	std::optional<Color> currentFore;
	std::optional<Color> currentBack;

	for (uint32_t y = 0; y < height; ++y)
	{
		for (uint32_t x = 0; x < width; ++x)
		{
			const size_t idx = GetIndex(x, y);
			const CellInfo& cell = buffer[idx];

			if (!currentFore.has_value() || *currentFore != cell.Fore)
			{
				output += GetAsniFg(cell.Fore);
				currentFore = cell.Fore;
			}
			
			if (!currentBack.has_value() || *currentBack != cell.Back)
			{
				output += GetAsniBg(cell.Back);
				currentBack = cell.Back;
			}
			
			output += cell.Symbol;
			snapshotBuffer[idx] = cell;
		}

		if (y < height - 1)
			output += L"\n";
	}

	out << std::nounitbuf;
	out.write(output.data(), output.size());
	out.flush();

	Snapshot();
	hasDirtyRect = false;
	dirtyRect = Rect();
}

void RenderBuffer::DiffRender(std::wostream& out)
{
	std::lock_guard<std::recursive_mutex> guard(renderMutex);
	if (snapshotWidth != width || snapshotHeight != height)
	{
		BulkRender(out);
		return;
	}

	if (!hasDirtyRect)
		return;

	const uint32_t startX = (hasDirtyRect) ? static_cast<uint32_t>(std::max(0, dirtyRect.X)) : 0U;
	const uint32_t startY = (hasDirtyRect) ? static_cast<uint32_t>(std::max(0, dirtyRect.Y)) : 0U;
	const uint32_t endX = (hasDirtyRect) ? static_cast<uint32_t>(std::min<int32_t>(width, dirtyRect.Right())) : width;
	const uint32_t endY = (hasDirtyRect) ? static_cast<uint32_t>(std::min<int32_t>(height, dirtyRect.Bottom())) : height;

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
				output += GetAsniFg(cell.Fore);
				currentFore = cell.Fore;
			}
			
			if (!currentBack.has_value() || *currentBack != cell.Back)
			{
				output += GetAsniBg(cell.Back);
				currentBack = cell.Back;
			}
			
			output += cell.Symbol;
			snapshotBuffer[idx] = cell;
			
			expectedX = x + 1;
			expectedY = y;
		}
	}

	out << std::nounitbuf;
	out.write(output.data(), output.size());
	out.flush();

	Snapshot();
	hasDirtyRect = false;
	dirtyRect = Rect();
}

size_t RenderBuffer::GetIndex(uint32_t x, uint32_t y) const
{
	return static_cast<size_t>(y) * MAX_WIDTH + x;
}

void RenderBuffer::MarkDirty(const Rect& rect)
{
	if (!hasDirtyRect)
	{
		dirtyRect = rect;
		hasDirtyRect = true;
		return;
	}

	dirtyRect = Rect::Union(dirtyRect, rect);
}
