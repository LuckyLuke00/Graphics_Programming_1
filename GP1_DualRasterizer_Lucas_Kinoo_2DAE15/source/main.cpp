#include "pch.h"

#if defined(_DEBUG)
#include "vld.h"
#endif

#undef main
#include "Renderer.h"

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
		"Dual Rasterizer - ***Lucas Kinoo (2DAE15)***",
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
	bool printFPS{ false };
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
				switch (e.key.keysym.sym)
				{
				case SDLK_F11:
					// Change console text color to yellow
					SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
					std::cout << "**(SHARED) Print FPS " << (printFPS ? "OFF" : "ON") << '\n';
					printFPS = !printFPS;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}

		//--------- Update ---------
		pRenderer->Update(pTimer);

		//--------- Render ---------
		pRenderer->Render();

		//--------- Timer ---------
		pTimer->Update();

		if (!printFPS)
		{
			printTimer = 1.f;
			continue;
		}

		printTimer += pTimer->GetElapsed();
		if (printTimer >= 1.f)
		{
			printTimer = .0f;

			// Change console text color to gray
			SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
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
