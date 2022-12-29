#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"
#include "Mesh.h"

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
	SDL_DestroyWindow(pWindow);
	SDL_Quit();
}

int main(int argc, char* args[])
{
	//Unreferenced parameters
	(void)argc;
	(void)args;

	//Create window + surfaces
	SDL_Init(SDL_INIT_VIDEO);

	constexpr uint32_t width{ 640 };
	constexpr uint32_t height{ 480 };

	SDL_Window* pWindow = SDL_CreateWindow(
		"DirectX - **Lucas Kinoo (2DAE15)**",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		width, height, 0);

	if (!pWindow)
		return 1;

	//Initialize "framework"
	const auto pTimer{ new Timer() };
	const auto pRenderer{ new Renderer(pWindow) };

	//Start loop
	pTimer->Start();
	float printTimer{ .0f };
	bool isLooping{ true };
	bool printFPW{ false };
	while (isLooping)
	{
		//--------- Get input events ---------
		SDL_Event e;
		while (SDL_PollEvent(&e))
		{
			switch (e.type)
			{
			case SDL_QUIT:
				isLooping = false;
				break;
			case SDL_KEYUP:
				if (e.key.keysym.scancode == SDL_SCANCODE_F2)
				{
					pRenderer->ToggleMeshRotation();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F3)
				{
					pRenderer->ToggleFireFXMesh();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F4)
				{
					for (auto pMesh : pRenderer->GetMeshes())
					{
						pMesh->CycleTechniques();
					}
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F9)
				{
					pRenderer->CycleCullMode();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F10)
				{
					pRenderer->ToggleClearColor();
				}
				if (e.key.keysym.scancode == SDL_SCANCODE_F11)
				{
					printFPW = !printFPW;
				}
			default:;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();
		printTimer += pTimer->GetElapsed();
		if (printFPW && printTimer >= 1.f)
		{
			printTimer = .0f;
			std::cout << "dFPS: " << pTimer->GetdFPS() << '\n';
		}
	}
	pTimer->Stop();

	//Shutdown "framework"
	delete pRenderer;
	delete pTimer;

	ShutDown(pWindow);
	return 0;
}
