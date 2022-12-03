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

		const ColorRGB& operator+(const ColorRGB& c)
		{
			return *this += c;
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

		const ColorRGB& operator-(const ColorRGB& c)
		{
			return *this -= c;
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

		const ColorRGB& operator*(const ColorRGB& c)
		{
			return *this *= c;
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

		const ColorRGB& operator/(const ColorRGB& c)
		{
			return *this /= c;
		}

		const ColorRGB& operator*=(float s)
		{
			r *= s;
			g *= s;
			b *= s;

			return *this;
		}

		const ColorRGB& operator*(float s)
		{
			return *this *= s;
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

		const ColorRGB& operator/(float s)
		{
			return *this /= s;
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
		const static ColorRGB Black{ 0.f, 0.f, 0.f };
		const static ColorRGB Blue{ 0.f, 0.f, 1.f };
		const static ColorRGB Cyan{ 0.f, 1.f, 1.f };
		const static ColorRGB Gray{ 0.5f, 0.5f, 0.5f };
		const static ColorRGB Green{ 0.f, 1.f, 0.f };
		const static ColorRGB Magenta{ 1.f, 0.f, 1.f };
		const static ColorRGB Red{ 1.f, 0.f, 0.f };
		const static ColorRGB White{ 1.f, 1.f, 1.f };
		const static ColorRGB Yellow{ 1.f, 1.f, 0.f };

		const static ColorRGB Specular{ 0.04f, 0.04f, 0.04f };
	}
}
