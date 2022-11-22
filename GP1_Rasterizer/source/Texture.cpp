#include "Texture.h"
#include "Vector2.h"
#include <SDL_image.h>
#include <iostream>
#include <cassert>

namespace dae
{
	Texture::Texture(SDL_Surface* pSurface) :
		m_pSurface{ pSurface },
		m_pSurfacePixels{ (uint32_t*)pSurface->pixels }
	{
	}

	Texture::~Texture()
	{
		if (m_pSurface)
		{
			SDL_FreeSurface(m_pSurface);
			m_pSurface = nullptr;
		}

		// Delete color
		delete m_pR;
		m_pR = nullptr;

		delete m_pG;
		m_pG = nullptr;

		delete m_pB;
		m_pB = nullptr;
	}

	Texture* Texture::LoadFromFile(const std::string& path)
	{
		//Load SDL_Surface using IMG_LOAD
		//Create & Return a new Texture Object (using SDL_Surface)
		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		if (!pSurface)
		{
			std::cout << "Texture::LoadFromFile() failed: " << IMG_GetError() << '\n';
			return nullptr;
		}
		return new Texture(pSurface);
	}

	ColorRGB Texture::Sample(const Vector2& uv) const
	{
		const int x{ static_cast<int>(uv.x * m_pSurface->w) };
		const int y{ static_cast<int>(uv.y * m_pSurface->h) };
		const uint32_t& color{ m_pSurfacePixels[y * m_pSurface->w + x] };

		SDL_GetRGB(color, m_pSurface->format, m_pR, m_pG, m_pB);

		return { *m_pR / 255.f, *m_pG / 255.f, *m_pB / 255.f };
	}
}