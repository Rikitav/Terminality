export module terminality:RenderBuffer;

import std;
import std.compat;
import :Geometry;

export namespace terminality
{
	struct CellInfo
	{
		wchar_t Symbol = L' ';
		Color Fore = Color::WHITE;
		Color Back = Color::BLACK;

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

		std::recursive_mutex renderMutex;

		uint32_t snapshotWidth = 0;
		uint32_t snapshotHeight = 0;
		std::vector<CellInfo> snapshotBuffer;

		uint32_t width = 0;
		uint32_t height = 0;
		std::vector<CellInfo> buffer;

		bool hasDirtyRect = true;
		Rect dirtyRect;

		size_t GetIndex(uint32_t x, uint32_t y) const;
		void MarkDirty(const Rect& rect);

	public:
		static constexpr size_t MAX_WIDTH = 512;
		static constexpr size_t MAX_HEIGHT = 256;

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

		const char* GetAsniBg(Color color);
		const char* GetAsniFg(Color color);
	};
}