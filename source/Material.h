#pragma once
#include "Math.h"
#include "DataTypes.h"
#include "BRDFs.h"

namespace dae
{
#pragma region Material BASE
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;

		Material(const Material&) = delete;
		Material(Material&&) noexcept = delete;
		Material& operator=(const Material&) = delete;
		Material& operator=(Material&&) noexcept = delete;

		/**
		 * \brief Function used to calculate the correct color for the specific material and its parameters
		 * \param hitRecord current hitrecord
		 * \param l light direction
		 * \param v view direction
		 * \return color
		 */
		virtual ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) = 0;
	};
#pragma endregion

#pragma region Material SOLID COLOR
	//SOLID COLOR
	//===========
	class Material_SolidColor final : public Material
	{
	public:
		Material_SolidColor(const ColorRGB& color) : m_Color(color)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord, const Vector3& l, const Vector3& v) override
		{
			return m_Color;
		}

	private:
		ColorRGB m_Color{ colors::White };
	};
#pragma endregion

#pragma region Material LAMBERT
	//LAMBERT
	//=======
	class Material_Lambert final : public Material
	{
	public:
		Material_Lambert(const ColorRGB& diffuseColor, float diffuseReflectance) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(diffuseReflectance) {}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 1.f }; //kd
	};
#pragma endregion

#pragma region Material LAMBERT PHONG
	//LAMBERT-PHONG
	//=============
	class Material_LambertPhong final : public Material
	{
	public:
		Material_LambertPhong(const ColorRGB& diffuseColor, float kd, float ks, float phongExponent) :
			m_DiffuseColor(diffuseColor), m_DiffuseReflectance(kd), m_SpecularReflectance(ks),
			m_PhongExponent(phongExponent)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			// Why is l negated and not v?
			return BRDF::Lambert(m_DiffuseReflectance, m_DiffuseColor) + BRDF::Phong(m_SpecularReflectance, m_PhongExponent, l, -v, hitRecord.normal);
		}

	private:
		ColorRGB m_DiffuseColor{ colors::White };
		float m_DiffuseReflectance{ 0.5f }; //kd
		float m_SpecularReflectance{ 0.5f }; //ks
		float m_PhongExponent{ 1.f }; //Phong Exponent
	};
#pragma endregion

#pragma region Material COOK TORRENCE
	//COOK TORRENCE
	class Material_CookTorrence final : public Material
	{
	public:
		Material_CookTorrence(const ColorRGB& albedo, float metalness, float roughness) :
			m_Albedo(albedo), m_Metalness(metalness), m_Roughness(roughness)
		{
		}

		ColorRGB Shade(const HitRecord& hitRecord = {}, const Vector3& l = {}, const Vector3& v = {}) override
		{
			const ColorRGB f0{ (AreEqual(m_Metalness, 0.f)) ? ColorRGB{ 0.04f, 0.04f, 0.04f } : m_Albedo };
			const Vector3 halfVector{ (l - v).Normalized() };

			const ColorRGB fresnel{ BRDF::FresnelFunction_Schlick(halfVector, -v, f0) };
			const float normalDistribution{ BRDF::NormalDistribution_GGX(hitRecord.normal, halfVector, m_Roughness) };
			const float geometry{ BRDF::GeometryFunction_Smith(hitRecord.normal, -v, l, m_Roughness) };

			ColorRGB fng{ fresnel * normalDistribution * geometry };
			const float denominator{ 4.f * (Vector3::Dot(-v, hitRecord.normal) * Vector3::Dot(l, hitRecord.normal)) };
			const ColorRGB specular{ fng / denominator };

			const ColorRGB kd{ (AreEqual(m_Metalness, 0.f)) ? ColorRGB{1.f, 1.f, 1.f } : ColorRGB{ 0.f, 0.f, 0.f } };
			const ColorRGB diffuse{ BRDF::Lambert(kd, m_Albedo) };

			return diffuse + specular;

			//// Determine F0 value -> (0.04, 0.04, 0.04) or Albedo based on Metalness
			//const ColorRGB f0{ m_Metalness * m_Albedo + (1.f - m_Metalness) * ColorRGB { 0.04f, 0.04f, 0.04f } };
			//// Calculate half vector between view direction and light direction
			//// Why the fuck is l inverted?!
			//const Vector3 h{ (l + -v).Normalized() };
			//// Calculate fresnel (F)
			//const ColorRGB f{ BRDF::FresnelFunction_Schlick(h, -v, f0) };
			//// Calculate Normal Distribution (D)
			//const float d{ BRDF::NormalDistribution_GGX(hitRecord.normal, h, m_Roughness) };
			//// Calculate Geometry (G)
			//// Again why -v??
			//const float g{ BRDF::GeometryFunction_Smith(hitRecord.normal, -v, l, m_Roughness) };
			//// Calculate specular -> Cook-Torrance -> (DFG)/4(dot(v,n)dot(l,n))
			////return { g,g,g };
			//const ColorRGB specular{ ColorRGB{ d * f * g } / (4.f * Vector3::Dot(-v, hitRecord.normal) * Vector3::Dot(l, hitRecord.normal)) };
			////return specular;

			//// Determine kd -> 1 - Fresnel, cancel out if it�s a metal (kd = 0)
			//// Calculate Diffuse = > BRDF::Lambert using the kd
			//// Return final color -> diffuse + specular
			//float kd{ 1.f - f.r };
			//if (m_Metalness > 0.f) kd = 0;
			//const ColorRGB diffuse{ BRDF::Lambert(kd, m_Albedo) };

			//return diffuse + specular;


		}

	private:
		ColorRGB m_Albedo{ 0.955f, 0.637f, 0.538f }; //Copper
		float m_Metalness{ 1.0f };
		float m_Roughness{ 0.1f }; // [1.0 > 0.0] >> [ROUGH > SMOOTH]
	};
#pragma endregion
}
