#pragma once
#include <cassert>

#include "Math.h"
#include "vector"

namespace dae
{
#pragma region GEOMETRY
	struct Sphere
	{
		Vector3 origin{};
		float radius{};

		unsigned char materialIndex{ 0 };
	};

	struct Plane
	{
		Vector3 origin{};
		Vector3 normal{};

		unsigned char materialIndex{ 0 };
	};

	enum class TriangleCullMode
	{
		FrontFaceCulling,
		BackFaceCulling,
		NoCulling
	};

	struct Triangle
	{
		Triangle() = default;
		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2, const Vector3& _normal) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }, normal{ _normal.Normalized() } {}

		Triangle(const Vector3& _v0, const Vector3& _v1, const Vector3& _v2) :
			v0{ _v0 }, v1{ _v1 }, v2{ _v2 }
		{
			const Vector3 edgeV0V1 = v1 - v0;
			const Vector3 edgeV0V2 = v2 - v0;
			normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();
		}

		Vector3 v0{};
		Vector3 v1{};
		Vector3 v2{};

		Vector3 normal{};

		TriangleCullMode cullMode{};
		unsigned char materialIndex{};
	};

	struct TriangleMesh
	{
		TriangleMesh() = default;
		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, TriangleCullMode _cullMode) :
			positions(_positions), indices(_indices), cullMode(_cullMode)
		{
			//Calculate Normals
			CalculateNormals();

			//Update Transforms
			UpdateTransforms();
		}

		TriangleMesh(const std::vector<Vector3>& _positions, const std::vector<int>& _indices, const std::vector<Vector3>& _normals, TriangleCullMode _cullMode) :
			positions(_positions), normals(_normals), indices(_indices), cullMode(_cullMode)
		{
			UpdateTransforms();
		}

		std::vector<Vector3> positions{};
		std::vector<Vector3> normals{};
		std::vector<int> indices{};
		unsigned char materialIndex{};

		TriangleCullMode cullMode{ TriangleCullMode::BackFaceCulling };

		Matrix rotationTransform{};
		Matrix translationTransform{};
		Matrix scaleTransform{};

		std::vector<Vector3> transformedPositions{};
		std::vector<Vector3> transformedNormals{};

		void Translate(const Vector3& translation)
		{
			translationTransform = Matrix::CreateTranslation(translation);
		}

		void RotateY(float yaw)
		{
			rotationTransform = Matrix::CreateRotationY(yaw);
		}

		void Scale(const Vector3& scale)
		{
			scaleTransform = Matrix::CreateScale(scale);
		}

		void AppendTriangle(const Triangle& triangle, bool ignoreTransformUpdate = false)
		{
			int startIndex = static_cast<int>(positions.size());

			positions.push_back(triangle.v0);
			positions.push_back(triangle.v1);
			positions.push_back(triangle.v2);

			indices.push_back(startIndex);
			indices.push_back(++startIndex);
			indices.push_back(++startIndex);

			normals.push_back(triangle.normal);

			//Not ideal, but making sure all vertices are updated
			if (!ignoreTransformUpdate)
				UpdateTransforms();
		}

		//void CalculateNormals()
		//{
		//	normals.resize(indices.size() / 3);

		//	Vector3 v0{}, v1{}, v2{};

		//	for (int index{}; index < normals.size(); index += 3)
		//	{
		//		v0 = positions[indices[index]];
		//		v1 = positions[indices[index + 1]];
		//		v2 = positions[indices[index + 2]];

		//		normals[index] = Vector3::Cross(v1 - v0, v2 - v0).Normalized();
		//	}
		//}

		//void UpdateTransforms()
		//{
		//	//Calculate Final Transform 
		//	auto transformMatrix = scaleTransform * rotationTransform * translationTransform;

		//	transformedPositions.reserve(positions.size());
		//	transformedNormals.reserve(normals.size());

		//	//Transform Positions (positions > transformedPositions)
		//	for (int index{}; index < positions.size(); ++index)
		//	{
		//		transformedPositions[index] = transformMatrix.TransformPoint(positions[index]);
		//	}

		//	//Transform Normals (normals > transformedNormals)
		//	for (int index{}; index < normals.size(); ++index)
		//	{
		//		transformedNormals[index] = transformMatrix.TransformVector(normals[index]);
		//	}
		//}

		void CalculateNormals()
		{
			normals.resize(indices.size() / 3);

			Vector3 v0{}, v1{}, v2{};

			for (int index{}; index < normals.size(); ++index)
			{
				const int indicesIndex{ index * 3 };

				v0 = positions[indices[indicesIndex]];
				v1 = positions[indices[indicesIndex + 1]];
				v2 = positions[indices[indicesIndex + 2]];

				normals[index] = Vector3::Cross(v1 - v0, v2 - v0).Normalized();
			}

			//// Should calculate the normal for each triangle defined by the Positions & Indices buffers, store the results in ‘normals’
			//for (size_t i = 0; i < indices.size(); i += 3)
			//{
			//	const Vector3 edgeV0V1 = positions[indices[i + 1]] - positions[indices[i]];
			//	const Vector3 edgeV0V2 = positions[indices[i + 2]] - positions[indices[i]];
			//	const Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2).Normalized();

			//	normals.push_back(normal);
			//}
		}

		void UpdateTransforms()
		{
			const Matrix transformMatrix = scaleTransform * rotationTransform * translationTransform;

			//Transform Positions
			transformedPositions.clear();
			for (const auto& position : positions)
			{
				transformedPositions.emplace_back(transformMatrix.TransformPoint(position));
			}

			//Transform Normals
			transformedNormals.clear();
			for (const auto& normal : normals)
			{
				transformedNormals.emplace_back(transformMatrix.TransformVector(normal));
			}
		}
	};
#pragma endregion
#pragma region LIGHT
	enum class LightType
	{
		Point,
		Directional
	};

	struct Light
	{
		Vector3 origin{};
		Vector3 direction{};
		ColorRGB color{};
		float intensity{};

		LightType type{};
	};
#pragma endregion
#pragma region MISC
	struct Ray
	{
		Vector3 origin{};
		Vector3 direction{};

		float min{ 0.0001f };
		float max{ FLT_MAX };
	};

	struct HitRecord
	{
		Vector3 origin{};
		Vector3 normal{};
		float t = FLT_MAX;

		bool didHit{ false };
		unsigned char materialIndex{ 0 };
	};
#pragma endregion
}