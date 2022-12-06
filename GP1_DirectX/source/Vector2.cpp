#include "pch.h"

#include "Vector2.h"
#include <cassert>

namespace dae {
	const Vector2 Vector2::UnitX = Vector2{ 1, 0 };
	const Vector2 Vector2::UnitY = Vector2{ 0, 1 };
	const Vector2 Vector2::Zero = Vector2{ 0, 0 };

	Vector2::Vector2(float _x, float _y) : x(_x), y(_y) {}

	Vector2::Vector2(const Vector2& from, const Vector2& to) : x(to.x - from.x), y(to.y - from.y) {}

	float Vector2::Magnitude() const
	{
		return sqrtf(x * x + y * y);
	}

	float Vector2::SqrMagnitude() const
	{
		return x * x + y * y;
	}

	float Vector2::Normalize()
	{
		const float m = Magnitude();
		x /= m;
		y /= m;

		return m;
	}

	Vector2 Vector2::Normalized() const
	{
		const float m = Magnitude();
		return { x / m, y / m };
	}

	float Vector2::Dot(const Vector2& v1, const Vector2& v2)
	{
		return v1.x * v2.x + v1.y * v2.y;
	}

	float Vector2::Cross(const Vector2& v1, const Vector2& v2)
	{
		return v1.x * v2.y - v1.y * v2.x;
	}

#pragma region Operator Overloads
	Vector2 Vector2::operator*(float scale) const
	{
		return { x * scale, y * scale };
	}

	Vector2 Vector2::operator/(float scale) const
	{
		return { x / scale, y / scale };
	}

	Vector2 Vector2::operator+(const Vector2& v) const
	{
		return { x + v.x, y + v.y };
	}

	Vector2 Vector2::operator-(const Vector2& v) const
	{
		return { x - v.x, y - v.y };
	}

	Vector2 Vector2::operator-() const
	{
		return { -x ,-y };
	}

	Vector2& Vector2::operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		return *this;
	}

	Vector2& Vector2::operator/=(float scale)
	{
		x /= scale;
		y /= scale;
		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2& v)
	{
		x -= v.x;
		y -= v.y;
		return *this;
	}

	Vector2& Vector2::operator+=(const Vector2& v)
	{
		x += v.x;
		y += v.y;
		return *this;
	}

	float& Vector2::operator[](int index)
	{
		assert(index <= 1 && index >= 0);
		return index == 0 ? x : y;
	}

	float Vector2::operator[](int index) const
	{
		assert(index <= 1 && index >= 0);
		return index == 0 ? x : y;
	}
#pragma endregion
}
