#pragma once

#include <cstdint>
#include <string>

#include <terminality/Core/Color.hpp>

// Colors and gradients ported from btop's built-in "Default" theme.
namespace btop
{
	using terminality::Color;

	struct Gradient
	{
		Color Start;
		Color Mid;
		Color End;

		// Value is expected in the 0..100 range (clamped).
		Color At(int64_t value) const
		{
			value = value < 0 ? 0 : (value > 100 ? 100 : value);
			if (value <= 50)
				return Color::Lerp(Start, Mid, value / 50.0f);

			return Color::Lerp(Mid, End, (value - 50) / 50.0f);
		}
	};

	namespace theme
	{
		inline const Color mainBg{ 0, 0, 0 };
		inline const Color mainFg{ 0xcc, 0xcc, 0xcc };
		inline const Color title{ 0xee, 0xee, 0xee };
		inline const Color hiFg{ 0xb5, 0x40, 0x40 };
		inline const Color selectedBg{ 0x6a, 0x2f, 0x2f };
		inline const Color selectedFg{ 0xee, 0xee, 0xee };
		inline const Color inactiveFg{ 0x40, 0x40, 0x40 };
		inline const Color graphText{ 0x60, 0x60, 0x60 };
		inline const Color meterBg{ 0x40, 0x40, 0x40 };
		inline const Color procMisc{ 0x0d, 0xe7, 0x56 };
		inline const Color cpuBox{ 0x55, 0x6d, 0x59 };
		inline const Color memBox{ 0x6c, 0x6c, 0x4b };
		inline const Color netBox{ 0x5c, 0x58, 0x8d };
		inline const Color procBox{ 0x80, 0x52, 0x52 };
		inline const Color divLine{ 0x30, 0x30, 0x30 };

		inline const Gradient temp{ { 0x48, 0x97, 0xd4 }, { 0x54, 0x74, 0xe8 }, { 0xff, 0x40, 0xb6 } };
		inline const Gradient cpu{ { 0x77, 0xca, 0x9b }, { 0xcb, 0xc0, 0x6c }, { 0xdc, 0x4c, 0x4c } };
		inline const Gradient free{ { 0x38, 0x4f, 0x21 }, { 0xb5, 0xe6, 0x85 }, { 0xdc, 0xff, 0x85 } };
		inline const Gradient cached{ { 0x16, 0x33, 0x50 }, { 0x74, 0xe6, 0xfc }, { 0x26, 0xc5, 0xff } };
		inline const Gradient available{ { 0x4e, 0x3f, 0x0e }, { 0xff, 0xd7, 0x7a }, { 0xff, 0xb8, 0x14 } };
		inline const Gradient used{ { 0x59, 0x2b, 0x26 }, { 0xd9, 0x62, 0x6d }, { 0xff, 0x47, 0x69 } };
		inline const Gradient download{ { 0x29, 0x1f, 0x75 }, { 0x4f, 0x43, 0xa3 }, { 0xb0, 0xa9, 0xde } };
		inline const Gradient upload{ { 0x62, 0x06, 0x65 }, { 0x7d, 0x41, 0x80 }, { 0xdc, 0xaf, 0xde } };
		inline const Gradient process{ { 0x80, 0xd0, 0xa3 }, { 0xdc, 0xd1, 0x79 }, { 0xd4, 0x54, 0x54 } };
	}

	// btop's Tools::floating_humanizer() for base-2 sizes, without fmt.
	// "458.2 GiB", "1.0 MiB", "42 Byte"; with perSecond -> "1.5 MiB/s";
	// with shorten -> "1.5M", "10K".
	inline std::wstring humanize(uint64_t value, bool shorten = false, bool perSecond = false)
	{
		static const wchar_t* units[] = { L"Byte", L"KiB", L"MiB", L"GiB", L"TiB", L"PiB", L"EiB", L"ZiB", L"YiB" };
		size_t start = 0;

		value *= 100; // fixed-point with two decimals
		while (value >= 102400 && start < 8)
		{
			value >>= 10;
			++start;
		}

		std::wstring out = std::to_wstring(value);
		if (out.size() == 4 && start > 0)
		{
			out.pop_back();
			out.insert(out.begin() + 2, L'.');
		}
		else if (out.size() == 3 && start > 0)
		{
			out.insert(out.begin() + 1, L'.');
		}
		else if (out.size() >= 2)
		{
			out.resize(out.size() - 2);
		}

		if (shorten)
		{
			const auto dot = out.find(L'.');
			if (dot != std::wstring::npos)
				out = out.substr(0, dot + 2);
			
			if (out.size() > 3)
			{
				if (out.find(L'.') != std::wstring::npos)
				{
					out = out.substr(0, out.find(L'.'));
				}
				else
				{
					out = std::wstring(1, out[0]) + L".0";
					++start;
				}
			}

			out.push_back(units[start][0]);
		}
		else
		{
			out += L' ';
			out += units[start];
		}

		if (perSecond)
			out += L"/s";

		return out;
	}

	// btop's Tools::sec_to_dhms(): "2d 04:15:36".
	inline std::wstring secToDhms(uint64_t seconds)
	{
		const uint64_t d = seconds / 86400;
		const uint64_t h = (seconds % 86400) / 3600;
		const uint64_t m = (seconds % 3600) / 60;
		const uint64_t s = seconds % 60;

		wchar_t buf[32];
		if (d > 0)
			swprintf(buf, 32, L"%llud %02llu:%02llu:%02llu", d, h, m, s);
		else
			swprintf(buf, 32, L"%02llu:%02llu:%02llu", h, m, s);

		return buf;
	}
}
