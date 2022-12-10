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

		// Use bitwise operations to extract the individual color channels
		const uint32_t color{ m_pSurfacePixels[y * m_pSurface->w + x] };
		const uint8_t red{ color & 0xFF };
		const uint8_t green{ (color >> 8) & 0xFF };
		const uint8_t blue{ (color >> 16) & 0xFF };

		// Use a precomputed value for 1/255
		constexpr float inv255{ 1.f / 255.f };
		return ColorRGB{ red * inv255, green * inv255, blue * inv255 };
	}
}
