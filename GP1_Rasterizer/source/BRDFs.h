#pragma once
#include "Math.h"

namespace dae::BRDF
{
	/**
	* \param kd Diffuse Reflection Coefficient
	* \param cd Diffuse Color
	* \return Lambert Diffuse Color
	*/
	static ColorRGB Lambert(const ColorRGB& kd, const ColorRGB& cd)
	{
		return kd * cd / PI;
	}

	/**
	 * \param ks Specular Reflection Coefficient
	 * \param exp Phong Exponent
	 * \param l Incoming (incident) Light Direction
	 * \param v View Direction
	 * \param n Normal of the Surface
	 * \return Phong Specular Color
	 */
	static ColorRGB Phong(const ColorRGB& ks, const ColorRGB& exp, const Vector3& l, const Vector3& v, const Vector3& n)
	{
		return { ks * std::powf(std::max(.0f, Vector3::Dot(Vector3::Reflect(n, l), v)), exp.r) };
	}
}