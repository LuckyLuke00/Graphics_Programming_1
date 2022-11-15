#include "Matrix.h"

#include <cassert>

#include "MathHelpers.h"
#include <cmath>

namespace dae {
	Matrix::Matrix(const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis, const Vector3& t) :
		Matrix({ xAxis, 0 }, { yAxis, 0 }, { zAxis, 0 }, { t, 1 })
	{
	}

	Matrix::Matrix(const Vector4& xAxis, const Vector4& yAxis, const Vector4& zAxis, const Vector4& t)
	{
		data[0] = xAxis;
		data[1] = yAxis;
		data[2] = zAxis;
		data[3] = t;
	}

	Matrix::Matrix(const Matrix& m)
	{
		data[0] = m[0];
		data[1] = m[1];
		data[2] = m[2];
		data[3] = m[3];
	}

	Vector3 Matrix::TransformVector(const Vector3& v) const
	{
		return TransformVector(v.x, v.y, v.z);
	}

	Vector3 Matrix::TransformVector(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z,
			data[0].y * x + data[1].y * y + data[2].y * z,
			data[0].z * x + data[1].z * y + data[2].z * z
		};
	}

	Vector3 Matrix::TransformPoint(const Vector3& p) const
	{
		return TransformPoint(p.x, p.y, p.z);
	}

	Vector3 Matrix::TransformPoint(float x, float y, float z) const
	{
		return Vector3{
			data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
			data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
			data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
		};
	}

	Vector4 Matrix::TransformPoint(const Vector4& p) const
	{
		return TransformPoint(p.x, p.y, p.z, p.w);
	}

	Vector4 Matrix::TransformPoint(float x, float y, float z, float w) const
	{
		return Vector4{
			data[0].x * x + data[1].x * y + data[2].x * z + data[3].x,
			data[0].y * x + data[1].y * y + data[2].y * z + data[3].y,
			data[0].z * x + data[1].z * y + data[2].z * z + data[3].z,
			data[0].w * x + data[1].w * y + data[2].w * z + data[3].w
		};
	}

	const Matrix& Matrix::Transpose()
	{
		Matrix result{};
		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = data[c][r];
			}
		}

		data[0] = result[0];
		data[1] = result[1];
		data[2] = result[2];
		data[3] = result[3];

		return *this;
	}

	const Matrix& Matrix::Inverse()
	{
		//Optimized Inverse as explained in FGED1 - used widely in other libraries too.
		const Vector3& a = data[0];
		const Vector3& b = data[1];
		const Vector3& c = data[2];
		const Vector3& d = data[3];

		const float x = data[0][3];
		const float y = data[1][3];
		const float z = data[2][3];
		const float w = data[3][3];

		Vector3 s = Vector3::Cross(a, b);
		Vector3 t = Vector3::Cross(c, d);
		Vector3 u = a * y - b * x;
		Vector3 v = c * w - d * z;

		float det = Vector3::Dot(s, v) + Vector3::Dot(t, u);
		assert((!AreEqual(det, 0.f)) && "ERROR: determinant is 0, there is no INVERSE!");
		float invDet = 1.f / det;

		s *= invDet; t *= invDet; u *= invDet; v *= invDet;

		Vector3 r0 = Vector3::Cross(b, v) + t * y;
		Vector3 r1 = Vector3::Cross(v, a) - t * x;
		Vector3 r2 = Vector3::Cross(d, u) + s * w;
		Vector3 r3 = Vector3::Cross(u, c) - s * z;

		data[0] = Vector4{ r0.x, r1.x, r2.x, 0.f };
		data[1] = Vector4{ r0.y, r1.y, r2.y, 0.f };
		data[2] = Vector4{ r0.z, r1.z, r2.z, 0.f };
		data[3] = { { -Vector3::Dot(b, t)},{Vector3::Dot(a, t)},{-Vector3::Dot(d, s)},{Vector3::Dot(c, s)} };

		return *this;
	}

	Matrix Matrix::Transpose(const Matrix& m)
	{
		Matrix out{ m };
		out.Transpose();

		return out;
	}

	Matrix Matrix::Inverse(const Matrix& m)
	{
		Matrix out{ m };
		out.Inverse();

		return out;
	}

	Matrix Matrix::CreateLookAtLH(const Vector3& origin, const Vector3& forward, const Vector3& up)
	{
		//TODO W1

		return {};
	}

	Matrix Matrix::CreatePerspectiveFovLH(float fov, float aspect, float zn, float zf)
	{
		//TODO W2

		return {};
	}

	Vector3 Matrix::GetAxisX() const
	{
		return data[0];
	}

	Vector3 Matrix::GetAxisY() const
	{
		return data[1];
	}

	Vector3 Matrix::GetAxisZ() const
	{
		return data[2];
	}

	Vector3 Matrix::GetTranslation() const
	{
		return data[3];
	}

	Matrix Matrix::CreateTranslation(float x, float y, float z)
	{
		return CreateTranslation({ x, y, z });
	}

	Matrix Matrix::CreateTranslation(const Vector3& t)
	{
		return { Vector3::UnitX, Vector3::UnitY, Vector3::UnitZ, t };
	}

	Matrix Matrix::CreateRotationX(float pitch)
	{
		return {
			{1, 0, 0, 0},
			{0, cos(pitch), -sin(pitch), 0},
			{0, sin(pitch), cos(pitch), 0},
			{0, 0, 0, 1}
		};
	}

	Matrix Matrix::CreateRotationY(float yaw)
	{
		return {
			{cos(yaw), 0, -sin(yaw), 0},
			{0, 1, 0, 0},
			{sin(yaw), 0, cos(yaw), 0},
			{0, 0, 0, 1}
		};
	}

	Matrix Matrix::CreateRotationZ(float roll)
	{
		return {
			{cos(roll), sin(roll), 0, 0},
			{-sin(roll), cos(roll), 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1}
		};
	}

	Matrix Matrix::CreateRotation(float pitch, float yaw, float roll)
	{
		return CreateRotation({ pitch, yaw, roll });
	}

	Matrix Matrix::CreateRotation(const Vector3& r)
	{
		return CreateRotationX(r[0]) * CreateRotationY(r[1]) * CreateRotationZ(r[2]);
	}

	Matrix Matrix::CreateScale(float sx, float sy, float sz)
	{
		return { {sx, 0, 0}, {0, sy, 0}, {0, 0, sz}, Vector3::Zero };
	}

	Matrix Matrix::CreateScale(const Vector3& s)
	{
		return CreateScale(s[0], s[1], s[2]);
	}

#pragma region Operator Overloads
	Vector4& Matrix::operator[](int index)
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Vector4 Matrix::operator[](int index) const
	{
		assert(index <= 3 && index >= 0);
		return data[index];
	}

	Matrix Matrix::operator*(const Matrix& m) const
	{
		Matrix result{};
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				result[r][c] = Vector4::Dot(data[r], m_transposed[c]);
			}
		}

		return result;
	}

	const Matrix& Matrix::operator*=(const Matrix& m)
	{
		Matrix copy{ *this };
		Matrix m_transposed = Transpose(m);

		for (int r{ 0 }; r < 4; ++r)
		{
			for (int c{ 0 }; c < 4; ++c)
			{
				data[r][c] = Vector4::Dot(copy[r], m_transposed[c]);
			}
		}

		return *this;
	}
#pragma endregion
}