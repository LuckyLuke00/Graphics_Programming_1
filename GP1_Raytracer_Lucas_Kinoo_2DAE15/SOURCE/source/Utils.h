#pragma once
#include <fstream>
#include "Math.h"
#include "DataTypes.h"

namespace dae
{
	namespace GeometryUtils
	{
#pragma region Sphere HitTest
		//SPHERE HIT-TESTS
		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
#pragma region Geometric Solution
			const Vector3 l{ sphere.origin - ray.origin };
			const float tca{ Vector3::Dot(l, ray.direction) };

			//if (tca < 0.f) return false; // Faster if we leave this out
			const float d2{ Vector3::Dot(l, l) - tca * tca };

			const float r2{ sphere.radius * sphere.radius };

			if (d2 > r2) return false;

			const float thc{ sqrtf(r2 - d2) };

			float t0{ tca - thc };
#pragma endregion
#pragma region Analytic Solution
			//const Vector3 l{ ray.origin - sphere.origin };
			//const float a{ Vector3::Dot(ray.direction, ray.direction) };
			//const float b{ 2.f * Vector3::Dot(ray.direction, l) };
			//const float c{ Vector3::Dot(l, l) - sphere.radius * sphere.radius };

			//float t0{};
			//float t1{};
			//const float discr{ b * b - 4.f * a * c };
			//if (discr < 0.f) return false;
			//else if (discr == 0.f) t0 = t1 = -.5f * b / a;
			//else
			//{
			//	const float q{ (b > 0.f) ? -.5f * (b + sqrt(discr)) : -.5f * (b - sqrt(discr)) };
			//	t0 = q / a;
			//	t1 = c / q;
			//}
			//if (t0 > t1) std::swap(t0, t1);
#pragma endregion
			if (t0 < 0.f || t0 > ray.max) return false;

			if (ignoreHitRecord) return true;

			hitRecord.didHit = true;
			hitRecord.materialIndex = sphere.materialIndex;
			hitRecord.origin = ray.origin + ray.direction * t0;
			hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			hitRecord.t = t0;

			return true;
		}

		inline bool HitTest_Sphere(const Sphere& sphere, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Sphere(sphere, ray, temp, true);
		}
#pragma endregion
#pragma region Plane HitTest
		//PLANE HIT-TESTS
		inline bool HitTest_Plane(const Plane& plane, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// Check plane intersection with ray
			// If intersection, return true and update hitRecord
			// If no intersection, return false and keep hitRecord unchanged

			const float denominator{ Vector3::Dot(plane.normal, ray.direction) };

			if (denominator > -ray.min && denominator < ray.min) return false;

			const float t{ Vector3::Dot(plane.origin - ray.origin, plane.normal) / denominator };

			if (t < ray.min || t > ray.max) return false;

			if (ignoreHitRecord) return true;

			hitRecord.didHit = true;
			hitRecord.materialIndex = plane.materialIndex;
			hitRecord.origin = ray.origin + ray.direction * t;
			hitRecord.normal = plane.normal;
			hitRecord.t = t;

			return true;
		}

		inline bool HitTest_Plane(const Plane& plane, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Plane(plane, ray, temp, true);
		}
#pragma endregion
#pragma region Triangle HitTest
		//TRIANGLE HIT-TESTS
		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
#pragma region Moller-Trumbore
			const Vector3 edge1{ triangle.v1 - triangle.v0 };
			const Vector3 edge2{ triangle.v2 - triangle.v0 };

			const Vector3 h{ Vector3::Cross(ray.direction, edge2) };
			const float a{ Vector3::Dot(edge1, h) };

			if (a > -ray.min && a < ray.min) return false;

			if (!ignoreHitRecord)
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && a < ray.min) return false;
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && a > ray.min) return false;
			}
			else
			{
				if (triangle.cullMode == TriangleCullMode::BackFaceCulling && a > ray.min) return false;
				if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && a < ray.min) return false;
			}

			const float f{ 1.f / a };
			const Vector3 s{ ray.origin - triangle.v0 };
			const float u{ f * Vector3::Dot(s, h) };

			if (u < 0.f || u > 1.f) return false;

			const Vector3 q{ Vector3::Cross(s, edge1) };
			const float v{ f * Vector3::Dot(ray.direction, q) };

			if (v < 0.f || u + v > 1.f) return false;

			const float t{ f * Vector3::Dot(edge2, q) };

			if (t > 0.f && t < ray.max)
			{
				if (ignoreHitRecord) return true;

				hitRecord.didHit = true;
				hitRecord.materialIndex = triangle.materialIndex;
				hitRecord.origin = ray.origin + ray.direction * t;
				hitRecord.normal = Vector3::Cross(edge1, edge2).Normalized();
				hitRecord.t = t;

				return true;
			}

			return false;
#pragma endregion
#pragma region Old Triangle HitTest
			////1. Check if ray is perpendicular to triangle
			//const Vector3 a{ triangle.v1 - triangle.v0 };
			//const Vector3 b{ triangle.v2 - triangle.v0 };
			//const Vector3 normal{ Vector3::Cross(a, b).Normalized() };

			//const float denominator{ Vector3::Dot(normal, ray.direction) };

			//if (AreEqual(denominator, 0.f)) return false;

			////2. Check if triangle is visible
			//if (!ignoreHitRecord)
			//{
			//	if (triangle.cullMode == TriangleCullMode::BackFaceCulling && denominator > ray.min) return false;
			//	if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && denominator < ray.min) return false;
			//}
			//else
			//{
			//	if (triangle.cullMode == TriangleCullMode::BackFaceCulling && denominator < ray.min) return false;
			//	if (triangle.cullMode == TriangleCullMode::FrontFaceCulling && denominator > ray.min) return false;
			//}

			////3. Ray-Plane test (plane defined by Triangle) + T range check
			//const Vector3 center{ (triangle.v0 + triangle.v1 + triangle.v2) / 3.f };
			//const float t{ Vector3::Dot(center - ray.origin, normal) / denominator };

			//if (t < 0.f || t > ray.max) return false;

			//const Vector3 hitPoint{ ray.origin + t * ray.direction };

			////4. Check if hitpoint is inside the Triangle
			//const Vector3 edgeA{ triangle.v1 - triangle.v0 };
			//Vector3 pointToSide{ hitPoint - triangle.v0 };
			//if (Vector3::Dot(normal, Vector3::Cross(edgeA, pointToSide)) < ray.min) return false;

			//const Vector3 edgeB{ triangle.v2 - triangle.v1 };
			//pointToSide = hitPoint - triangle.v1;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeB, pointToSide)) < ray.min) return false;

			//const Vector3 edgeC{ triangle.v0 - triangle.v2 };
			//pointToSide = hitPoint - triangle.v2;
			//if (Vector3::Dot(normal, Vector3::Cross(edgeC, pointToSide)) < ray.min) return false;

			////5. Fill-in HitRecord (if required)
			//if (ignoreHitRecord) return true;

			//hitRecord.didHit = true;
			//hitRecord.materialIndex = triangle.materialIndex;
			//hitRecord.origin = hitPoint;
			//hitRecord.normal = normal;
			//hitRecord.t = t;

			//return true;
#pragma endregion
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh SlabTest
		inline bool SlabTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			const float tx1{ (mesh.transformedMinAABB.x - ray.origin.x) / ray.direction.x };
			const float tx2{ (mesh.transformedMaxAABB.x - ray.origin.x) / ray.direction.x };

			float tmin{ std::min(tx1, tx2) };
			float tmax{ std::max(tx1, tx2) };

			const float ty1{ (mesh.transformedMinAABB.y - ray.origin.y) / ray.direction.y };
			const float ty2{ (mesh.transformedMaxAABB.y - ray.origin.y) / ray.direction.y };

			tmin = std::max(tmin, std::min(ty1, ty2));
			tmax = std::min(tmax, std::max(ty1, ty2));

			const float tz1{ (mesh.transformedMinAABB.z - ray.origin.z) / ray.direction.z };
			const float tz2{ (mesh.transformedMaxAABB.z - ray.origin.z) / ray.direction.z };

			tmin = std::max(tmin, std::min(tz1, tz2));
			tmax = std::min(tmax, std::max(tz1, tz2));

			return tmax > 0.f && tmax >= tmin;
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			// SlabTest
			if (!SlabTest_TriangleMesh(mesh, ray)) return false;
			// Each set of 3 indices represents a Triangle � use HitTest_Triangle to find the triangle of the TriangleMesh with the (!) closest hit
			// Use the �transformedPositions� & �transformedNormals� to define each individual triangle!

			HitRecord temp{};

			for (size_t i{ 0 }; i < mesh.indices.size(); i += 3)
			{
				Triangle triangle
				{
					mesh.transformedPositions[mesh.indices[i]],
					mesh.transformedPositions[mesh.indices[i + 1]],
					mesh.transformedPositions[mesh.indices[i + 2]],
					mesh.normals[mesh.indices[i]]
				};

				triangle.materialIndex = mesh.materialIndex;
				triangle.cullMode = mesh.cullMode;

				if (HitTest_Triangle(triangle, ray, temp, ignoreHitRecord))
				{
					if (ignoreHitRecord) return true;

					if (hitRecord.t > temp.t)
					{
						hitRecord = temp;
					}
				}
			}
			return hitRecord.didHit;
		}

		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_TriangleMesh(mesh, ray, temp, true);
		}
#pragma endregion
	}
	namespace LightUtils
	{
		//Direction from target to light
		inline Vector3 GetDirectionToLight(const Light& light, const Vector3 origin)
		{
			// this function should return a vector going from
			// the origin to the light's origin.

			return Vector3{ light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			if (light.type == LightType::Directional)
			{
				return light.color * light.intensity;
			}

			return light.color * (light.intensity / (light.origin - target).SqrMagnitude());
		}
	}

	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vector3>& positions, std::vector<Vector3>& normals, std::vector<int>& indices)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>)
				file >> sCommand;
				//use conditional statements to process the different commands
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex
					float x, y, z;
					file >> x >> y >> z;
					positions.push_back({ x, y, z });
				}
				else if (sCommand == "f")
				{
					float i0, i1, i2;
					file >> i0 >> i1 >> i2;

					indices.push_back(static_cast<int>(i0) - 1);
					indices.push_back(static_cast<int>(i1) - 1);
					indices.push_back(static_cast<int>(i2) - 1);
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');

				if (file.eof())
					break;
			}

			//Precompute normals
			for (uint64_t index = 0; index < indices.size(); index += 3)
			{
				const uint32_t i0 = indices[index];
				const uint32_t i1 = indices[index + 1];
				const uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				normal.Normalize();
				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}