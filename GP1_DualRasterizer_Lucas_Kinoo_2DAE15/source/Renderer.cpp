#include "pch.h"
#include "Renderer.h"

namespace dae {
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow(pWindow)
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		const HRESULT result{ InitializeDirectX() };
		if (result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}
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

	HRESULT Renderer::InitializeDirectX()
	{
		return S_FALSE;
	}
}
