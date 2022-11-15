#pragma once
#include "Vector3.h"
#include "Vector4.h"

namespace dae {
	struct Matrix
	{
		Matrix() = default;
		Matrix(
			const Vector3& xAxis,
			const Vector3& yAxis,
			const Vector3& zAxis,
			const Vector3& t);

		Matrix(
			const Vector4& xAxis,
			const Vector4& yAxis,
			const Vector4& zAxis,
			const Vector4& t);

		Matrix(const Matrix& m);

		Vector3 TransformVector(const Vector3& v) const;
		Vector3 TransformVector(float x, float y, float z) const;
		Vector3 TransformPoint(const Vector3& p) const;
		Vector3 TransformPoint(float x, float y, float z) const;

		Vector4 TransformPoint(const Vector4& p) const;
		Vector4 TransformPoint(float x, float y, float z, float w) const;

		const Matrix& Transpose();
		const Matrix& Inverse();

		Vector3 GetAxisX() const;
		Vector3 GetAxisY() const;
		Vector3 GetAxisZ() const;
		Vector3 GetTranslation() const;

		static Matrix CreateTranslation(float x, float y, float z);
		static Matrix CreateTranslation(const Vector3& t);
		static Matrix CreateRotationX(float pitch);
		static Matrix CreateRotationY(float yaw);
		static Matrix CreateRotationZ(float roll);
		static Matrix CreateRotation(float pitch, float yaw, float roll);
		static Matrix CreateRotation(const Vector3& r);
		static Matrix CreateScale(float sx, float sy, float sz);
		static Matrix CreateScale(const Vector3& s);
		static Matrix Transpose(const Matrix& m);
		static Matrix Inverse(const Matrix& m);

		static Matrix CreateLookAtLH(const Vector3& origin, const Vector3& forward, const Vector3& up);
		static Matrix CreatePerspectiveFovLH(float fovy, float aspect, float zn, float zf);

		Vector4& operator[](int index);
		Vector4 operator[](int index) const;
		Matrix operator*(const Matrix& m) const;
		const Matrix& operator*=(const Matrix& m);

	private:

		//Row-Major Matrix
		Vector4 data[4]
		{
			{1,0,0,0}, //xAxis
			{0,1,0,0}, //yAxis
			{0,0,1,0}, //zAxis
			{0,0,0,1}  //T
		};

		// v0x v0y v0z v0w
		// v1x v1y v1z v1w
		// v2x v2y v2z v2w
		// v3x v3y v3z v3w
	};
}