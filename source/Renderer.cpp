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

	//Calculate FOV
	static const float fov{ tanf(TO_RADIANS * camera.fovAngle / 2.f) };

	for (int px{ 0 }; px < m_Width; ++px)
	{
		for (int py{ 0 }; py < m_Height; ++py)
		{
			Vector3 rayDirection
			{
				(2.f * (static_cast<float>(px) + 0.5f) / static_cast<float>(m_Width) - 1.f) * m_AspectRatio * fov,
				(1.f - 2.f * (static_cast<float>(py) + 0.5f) / static_cast<float>(m_Height)) * fov,
				1.f
			};

			// Transform rayDirection with cameraToWorld
			rayDirection = camera.cameraToWorld.TransformVector(rayDirection);

			const Ray viewRay{ camera.origin, rayDirection };

			//Color to write to the color buffer
			ColorRGB finalColor{};

			HitRecord closestHit{};
			pScene->GetClosestHit(viewRay, closestHit);

			if (closestHit.didHit)
			{
				finalColor = materials[closestHit.materialIndex]->Shade();

				// Check if pixel is shadowed
				// For each light
				//for (const Light& light : lights)
				//{
				//	// Calculate Hit towards light RAY
				//	// Use small offset for the ray origin (self-shadowing)
				//	// Ray.max > Magnitude of vector between hit & light
				//}
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