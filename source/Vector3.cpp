#include "Vector3.h"

#include <cassert>

#include "Vector4.h"
#include <cmath>

namespace dae {
	const Vector3 Vector3::UnitX = Vector3{ 1, 0, 0 };
	const Vector3 Vector3::UnitY = Vector3{ 0, 1, 0 };
	const Vector3 Vector3::UnitZ = Vector3{ 0, 0, 1 };
	const Vector3 Vector3::Zero = Vector3{ 0, 0, 0 };

	Vector3::Vector3(float _x, float _y, float _z) : x(_x), y(_y), z(_z){}

	Vector3::Vector3(const Vector4& v) : x(v.x), y(v.y), z(v.z){}

	Vector3::Vector3(const Vector3& from, const Vector3& to) : x(to.x - from.x), y(to.y - from.y), z(to.z - from.z){}

	float Vector3::Magnitude() const
	{
		return sqrtf(x * x + y * y + z * z);
	}

	float Vector3::SqrMagnitude() const
	{
		return x * x + y * y + z * z;
	}

	float Vector3::Normalize()
	{
		const float m = Magnitude();
		x /= m;
		y /= m;
		z /= m;

		return m;
	}

	Vector3 Vector3::Normalized() const
	{
		const float m = Magnitude();
		return { x / m, y / m, z / m };
	}

	float Vector3::Dot(const Vector3& v1, const Vector3& v2)
	{
		//todo W1
		assert(false && "Not Implemented Yet");
		return {};
	}

	Vector3 Vector3::Cross(const Vector3& v1, const Vector3& v2)
	{
		//todo W1
		assert(false && "Not Implemented Yet");
		return {};
	}

	Vector3 Vector3::Project(const Vector3& v1, const Vector3& v2)
	{
		return (v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reject(const Vector3& v1, const Vector3& v2)
	{
		return (v1 - v2 * (Dot(v1, v2) / Dot(v2, v2)));
	}

	Vector3 Vector3::Reflect(const Vector3& v1, const Vector3& v2)
	{
		return v1 - (2.f * Vector3::Dot(v1, v2) * v2);
	}

	Vector4 Vector3::ToPoint4() const
	{
		return { x, y, z, 1 };
	}

	Vector4 Vector3::ToVector4() const
	{
		return { x, y, z, 0 };
	}

#pragma region Operator Overloads
	Vector3 Vector3::operator*(float scale) const
	{
		return { x * scale, y * scale, z * scale };
	}

	Vector3 Vector3::operator/(float scale) const
	{
		return { x / scale, y / scale, z / scale };
	}

	Vector3 Vector3::operator+(const Vector3& v) const
	{
		return { x + v.x, y + v.y, z + v.z };
	}

	Vector3 Vector3::operator-(const Vector3& v) const
	{
		return { x - v.x, y - v.y, z - v.z };
	}

	Vector3 Vector3::operator-() const
	{
		return { -x ,-y,-z };
	}

	Vector3& Vector3::operator*=(float scale)
	{
		x *= scale;
		y *= scale;
		z *= scale;
		return *this;
	}

	Vector3& Vector3::operator/=(float scale)
	{
		x /= scale;
		y /= scale;
		z /= scale;
		return *this;
	}

	Vector3& Vector3::operator-=(const Vector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	Vector3& Vector3::operator+=(const Vector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	float& Vector3::operator[](int index)
	{
		assert(index <= 2 && index >= 0);

		if (index == 0) return x;
		if (index == 1) return y;
		return z;
	}

	float Vector3::operator[](int index) const
	{
		assert(index <= 2 && index >= 0);

		if (index == 0) return x;
		if (index == 1) return y;
		return z;
	}
#pragma endregion
}