#include "pch.h"
#include "Renderer.h"

namespace dae {
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		if (InitializeDirectX() == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		PrintKeybinds();
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::Update(const Timer* pTimer)
	{
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;
	}

	void Renderer::PrintKeybinds() const
	{
		// Print SHARED keybinds
		// Change console text color to yellow
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
		std::cout << "[Key Bindings - SHARED]\n"
			<< "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n"
			<< "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n"
			<< "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n"
			<< "   [F10] Toggle Uniform ClearColor (ON/OFF)\n"
			<< "   [F11] Toggle Print FPS (ON/OFF)\n\n";

		// Print HARDWARE keybinds
		// Change console text color to green
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
		std::cout << "[Key Bindings - HARDWARE]\n"
			<< "   [F3] Toggle FireFX (ON/OFF)\n"
			<< "   [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n\n";

		// Print SOFTWARE
		// Change console text color to purple
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5);
		std::cout << "[Key Bindings - SOFTWARE]\n"
			<< "   [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n"
			<< "   [F6] Toggle NormalMap (ON/OFF)\n"
			<< "   [F7] Toggle DepthBuffer Visualization (ON/OFF)\n"
			<< "   [F8] Toggle BoundingBox Visualization (ON/OFF)\n\n\n";
	}

	HRESULT Renderer::InitializeDirectX()
	{
		return S_FALSE;
	}
}
