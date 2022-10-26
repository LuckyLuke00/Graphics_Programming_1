//External includes
#include "SDL.h"
#include "SDL_surface.h"

//Project includes
#include "Material.h"
#include "Math.h"
#include "Matrix.h"
#include "Renderer.h"
#include "Scene.h"
#include "Utils.h"

// Standard includes
#include <future> //Async
#include <ppl.h> //Parallel_for

//#define ASYNC
#define PARALLEL_FOR

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
	Camera& camera{ pScene->GetCamera() };
	camera.CalculateCameraToWorld();

	const float fovAngle{ camera.fovAngle * TO_RADIANS };
	const float fov{ tan(fovAngle / 2.f) };

	const float aspectRatio{ m_Width / static_cast<float>(m_Height) };

	auto& materials{ pScene->GetMaterials() };
	auto& lights{ pScene->GetLights() };

	const uint32_t numPixels{ static_cast<uint32_t>(m_Width * m_Height) };


#if defined(ASYNC)
	//Async Logic
	//+++++++++++

	const uint32_t numCores{ std::thread::hardware_concurrency() };
	const uint32_t numPixelsPerTask{ numPixels / numCores };

	std::vector<std::future<void>> async_futures{};

	uint32_t numUnassignedPixels{ numPixels % numCores };
	uint32_t currPixelIndex{ 0 };

	for (uint32_t coreId{ 0 }; coreId < numCores; ++coreId)
	{
		uint32_t taskSize{ numPixelsPerTask };
		if (numUnassignedPixels > 0)
		{
			++taskSize;
			--numUnassignedPixels;
		}

		async_futures.push_back(std::async(std::launch::async, [=, this]
			{
				//Render all pixels for this task (currPixelIndex > currPixelIndex + taskSize)
				const uint32_t pixelIndexEnd{ currPixelIndex + taskSize };
				for (uint32_t pixelIndex{ currPixelIndex }; pixelIndex < pixelIndexEnd; ++pixelIndex)
				{
					RenderPixel(pScene, pixelIndex, fov, aspectRatio, camera, lights, materials);
				}
			}));

		currPixelIndex += taskSize;
	}

	//Wait for async completion of all tasks
	for (const std::future<void>& f : async_futures)
	{
		f.wait();
	}

#elif defined(PARALLEL_FOR)
	//Parallel-For Logic
	//++++++++++++++++++
	concurrency::parallel_for(0u, numPixels, [=, this](int i)
		{
			RenderPixel(pScene, i, fov, aspectRatio, camera, lights, materials);
		});

#else
	//Synchronous Logic (no threading)
	//++++++++++++++++++++++++++++++++
	for (uint32_t i{ 0 }; i < numPixels; ++i)
	{
		RenderPixel(pScene, i, fov, aspectRatio, pScene->GetCamera(), lights, materials);
	}

#endif

	//@END
	//Update SDL Surface
	SDL_UpdateWindowSurface(m_pWindow);
}

void Renderer::RenderPixel(Scene* pScene, uint32_t pixelIndex, float fov, float aspectRatio, const Camera& camera, const std::vector<Light>& lights, const std::vector<Material*>& materials) const
{
	const int px{ static_cast<int>(pixelIndex % m_Width) };
	const int py{ static_cast<int>(pixelIndex / m_Width) };

	float rx{ px + .5f };
	float ry{ py + .5f };

	float cx{ (2.f * (rx / static_cast<float>(m_Width)) - 1.f) * aspectRatio * fov };
	float cy{ (1.f - 2 * (ry / static_cast<float>(m_Height))) * fov };

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

	if (!closestHit.didHit) return;

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

			if (pScene->DoesHit(lightRay)) continue;
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

bool Renderer::SaveBufferToImage() const
{
	return SDL_SaveBMP(m_pBuffer, "RayTracing_Buffer.bmp");
}

void Renderer::CycleLightingMode()
{
	static constexpr int enumSize{ sizeof(LightingMode) };
	m_CurrentLightingMode = static_cast<LightingMode>((static_cast<int>(m_CurrentLightingMode) + 1) % enumSize);

	// Print current m_CurrentLightingMode
	switch (m_CurrentLightingMode)
	{
	case LightingMode::ObservedArea:
		std::cout << "\nLIGHTING MODE: OBSERVED AREA\n\n";
		break;
	case LightingMode::Radiance:
		std::cout << "\nLIGHTING MODE: RADIANCE\n\n";
		break;
	case LightingMode::BRDF:
		std::cout << "\nLIGHTING MODE: BRDF\n\n";
		break;
	case LightingMode::Combined:
		std::cout << "\nLIGHTING MODE: COMBINED\n\n";
		break;
	}
}
