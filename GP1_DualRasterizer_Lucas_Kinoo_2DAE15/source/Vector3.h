#pragma once

namespace dae
{
	struct Vector2;
	struct Vector4;
	struct Vector3
	{
		float x{};
		float y{};
		float z{};

		Vector3() = default;
		Vector3(float _x, float _y, float _z);
		Vector3(const Vector3& from, const Vector3& to);
		Vector3(const Vector4& v);

		float Magnitude() const;
		float SqrMagnitude() const;
		float Normalize();
		Vector3 Normalized() const;

		static float Dot(const Vector3& v1, const Vector3& v2);
		static Vector3 Cross(const Vector3& v1, const Vector3& v2);
		static Vector3 Project(const Vector3& v1, const Vector3& v2);
		static Vector3 Reject(const Vector3& v1, const Vector3& v2);
		static Vector3 Reflect(const Vector3& v1, const Vector3& v2);

		Vector4 ToPoint4() const;
		Vector4 ToVector4() const;

		Vector2 GetXY() const;

		//Member Operators
		Vector3 operator*(float scale) const;
		Vector3 operator/(float scale) const;
		Vector3 operator+(const Vector3& v) const;
		Vector3 operator-(const Vector3& v) const;
		Vector3 operator-() const;
		//Vector3& operator-();
		Vector3& operator+=(const Vector3& v);
		Vector3& operator-=(const Vector3& v);
		Vector3& operator/=(float scale);
		Vector3& operator*=(float scale);
		float& operator[](int index);
		float operator[](int index) const;

		static const Vector3 UnitX;
		static const Vector3 UnitY;
		static const Vector3 UnitZ;
		static const Vector3 Zero;
	};

	//Global Operators
	inline Vector3 operator*(float scale, const Vector3& v)
	{
		return { v.x * scale, v.y * scale, v.z * scale };
	}
}
