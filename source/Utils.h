#pragma once
#include <cassert>
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
			// Check sphere intersection with ray
			// If intersection, return true and update hitRecord
			// If no intersection, return false and keep hitRecord unchanged

			const Vector3 sphereToRay{ ray.origin - sphere.origin };

			const float a{ Vector3::Dot(ray.direction, ray.direction) };
			const float b{ 2.f * Vector3::Dot(sphereToRay, ray.direction) };
			const float c{ Vector3::Dot(sphereToRay, sphereToRay) - sphere.radius * sphere.radius };

			float discriminant{ b * b - 4.f * a * c };

			if (discriminant < 0.f) return false;

			discriminant = sqrtf(discriminant);

			const float t0{ (-b + discriminant) / (2.f * a) };
			const float t1{ (-b - discriminant) / (2.f * a) }; // Will always be smaller than t0

			const float t{ t1 < 0.f ? t0 : t1 };

			if (t < 0.f) return false;

			if (t > ray.max) return false;

			if (ignoreHitRecord) return true;

			hitRecord.didHit = true;
			hitRecord.materialIndex = sphere.materialIndex;
			hitRecord.origin = ray.origin + ray.direction * t;
			hitRecord.normal = (hitRecord.origin - sphere.origin).Normalized();
			hitRecord.t = t;

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

			if (denominator == 0.f) return false;

			const float t{ Vector3::Dot(plane.origin - ray.origin, plane.normal) / denominator };

			if (t < 0.f) return false;

			if (t > ray.max) return false;

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
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
		}

		inline bool HitTest_Triangle(const Triangle& triangle, const Ray& ray)
		{
			HitRecord temp{};
			return HitTest_Triangle(triangle, ray, temp, true);
		}
#pragma endregion
#pragma region TriangeMesh HitTest
		inline bool HitTest_TriangleMesh(const TriangleMesh& mesh, const Ray& ray, HitRecord& hitRecord, bool ignoreHitRecord = false)
		{
			//todo W5
			assert(false && "No Implemented Yet!");
			return false;
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
			// the origin to the light’s origin.

			return Vector3{ light.origin - origin };
		}

		inline ColorRGB GetRadiance(const Light& light, const Vector3& target)
		{
			//todo W3
			assert(false && "No Implemented Yet!");
			return {};
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
				uint32_t i0 = indices[index];
				uint32_t i1 = indices[index + 1];
				uint32_t i2 = indices[index + 2];

				Vector3 edgeV0V1 = positions[i1] - positions[i0];
				Vector3 edgeV0V2 = positions[i2] - positions[i0];
				Vector3 normal = Vector3::Cross(edgeV0V1, edgeV0V2);

				if (isnan(normal.x))
				{
					int k = 0;
				}

				normal.Normalize();
				if (isnan(normal.x))
				{
					int k = 0;
				}

				normals.push_back(normal);
			}

			return true;
		}
#pragma warning(pop)
	}
}