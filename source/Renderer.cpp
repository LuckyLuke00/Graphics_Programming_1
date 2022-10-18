//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Renderer.h"
#include "Math.h"
#include "Matrix.h"
#include "Material.h"
#include "Scene.h"
#include "Utils.h"

using namespace dae;

Renderer::Renderer(SDL_Window* pWindow) :
	m_pWindow(pWindow),
	m_pBuffer(SDL_GetWindowSurface(pWindow))
{
	//Initialize
	SDL_GetWindowSize(pWindow, &m_Width, &m_Height);
	m_pBufferPixels = static_cast<uint32_t*>(m_pBuffer->pixels);
	m_AspectRatio = static_cast<float>(m_Width) / static_cast<float>(m_Height);
}

void Renderer::Render(Scene* pScene) const
{
	Camera& camera = pScene->GetCamera();
	auto& materials = pScene->GetMaterials();
	auto& lights = pScene->GetLights();

	camera.CalculateCameraToWorld();

	for (int px{ 0 }; px < m_Width; ++px)
	{
		for (int py{ 0 }; py < m_Height; ++py)
		{
			Vector3 rayDirection
			{
				(2.f * (static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width) - 1.f) * m_AspectRatio * camera.fov,
				(1.f - 2.f * (static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height)) * camera.fov,
				1.f
			};

			// Transform rayDirection with cameraToWorld
			rayDirection = camera.cameraToWorld.TransformVector(rayDirection).Normalized();

			const Ray viewRay{ camera.origin, rayDirection };

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (!closestHit.didHit) continue;

			//Color to write to the color buffer
			ColorRGB finalColor{};

			// For each light
			for (const Light& light : lights)
			{
				const Vector3 lightDirection{ LightUtils::GetDirectionToLight(light, closestHit.origin).Normalized() };
				const float observedArea{ Vector3::Dot(closestHit.normal, lightDirection) };

				// We need to check the LightingMode because 
				if (m_ShadowsEnabled)
				{
					const Ray lightRay
					{
						closestHit.origin + closestHit.normal * 0.0001f,
						lightDirection,
						0.0001f,
						(light.origin - closestHit.origin).Magnitude()
					};

					if (pScene->DoesHit(lightRay)) // Is the light occluded?
					{
						continue;
					}
				}

				switch (m_CurrentLightingMode)
				{
					case LightingMode::ObservedArea:
						if (observedArea < 0.f) break;
						finalColor += {observedArea, observedArea, observedArea };
						break;
					case LightingMode::Radiance:
						finalColor += LightUtils::GetRadiance(light, closestHit.origin);
						break;
					case LightingMode::BRDF:
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, viewRay.direction);
						break;
					case LightingMode::Combined:
						if (observedArea < 0.f) break;
						finalColor += materials[closestHit.materialIndex]->Shade(closestHit, lightDirection, viewRay.direction) * LightUtils::GetRadiance(light, closestHit.origin) * observedArea;
						break;
				}
			}

			//Update Color in Buffer
			finalColor.MaxToOne();

			m_pBufferPixels[px + (py * m_Width)] = SDL_MapRGB(m_pBuffer->format,
				static_cast<uint8_t>(finalColor.r * 255),
				static_cast<uint8_t>(finalColor.g * 255),
				static_cast<uint8_t>(finalColor.b * 255));
		}
	}
	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	static constexpr int enumSize{ sizeof(LightingMode) };
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % enumSize);
}