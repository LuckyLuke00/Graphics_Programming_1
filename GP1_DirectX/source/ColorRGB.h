#pragma once
#include "MathHelpers.h"

namespace dae
{
	struct ColorRGB
	{
		float r{};
		float g{};
		float b{};

		void MaxToOne()
		{
			const float maxValue = std::max(r, std::max(g, b));
			if (maxValue > 1.f)
				*this /= maxValue;
		}

		static ColorRGB Lerp(const ColorRGB& c1, const ColorRGB& c2, float factor)
		{
			return { Lerpf(c1.r, c2.r, factor), Lerpf(c1.g, c2.g, factor), Lerpf(c1.b, c2.b, factor) };
		}

#pragma region ColorRGB (Member) Operators
		const ColorRGB& operator+=(const ColorRGB& c)
		{
			r += c.r;
			g += c.g;
			b += c.b;

			return *this;
		}

		ColorRGB operator+(const ColorRGB& c) const
		{
			return { r + c.r, g + c.g, b + c.b };
		}

		const ColorRGB& operator-=(const ColorRGB& c)
		{
			r -= c.r;
			g -= c.g;
			b -= c.b;

			return *this;
		}

		ColorRGB operator-(const ColorRGB& c) const
		{
			return { r - c.r, g - c.g, b - c.b };
		}

		const ColorRGB& operator*=(const ColorRGB& c)
		{
			r *= c.r;
			g *= c.g;
			b *= c.b;

			return *this;
		}

		ColorRGB operator*(const ColorRGB& c) const
		{
			return { r * c.r, g * c.g, b * c.b };
		}

		const ColorRGB& operator/=(const ColorRGB& c)
		{
			r /= c.r;
			g /= c.g;
			b /= c.b;

			return *this;
		}

		const ColorRGB& operator*=(float s)
		{
			r *= s;
			g *= s;
			b *= s;

			return *this;
		}

		ColorRGB operator*(float s) const
		{
			return { r * s, g * s,b * s };
		}

		const ColorRGB& operator/=(float s)
		{
			r /= s;
			g /= s;
			b /= s;

			return *this;
		}

		ColorRGB operator/(float s) const
		{
			return { r / s, g / s,b / s };
		}
#pragma endregion
	};

	//ColorRGB (Global) Operators
	inline ColorRGB operator*(float s, const ColorRGB& c)
	{
		return c * s;
	}

	namespace colors
	{
		constexpr static ColorRGB Red{ 1.f, .0f, .0f };
		constexpr static ColorRGB Blue{ .0f, .0f, 1.f };
		constexpr static ColorRGB Green{ .0f, 1.f, .0f };
		constexpr static ColorRGB Yellow{ 1.f, 1.f, .0f };
		constexpr static ColorRGB Cyan{ .0f, 1.f, 1.f };
		constexpr static ColorRGB Magenta{ 1.f, .0f, 1.f };
		constexpr static ColorRGB White{ 1.f, 1.f, 1.f };
		constexpr static ColorRGB Black{ .0f, .0f, .0f };
		constexpr static ColorRGB Gray{ .5f, .5f, .5f };
	}
}
