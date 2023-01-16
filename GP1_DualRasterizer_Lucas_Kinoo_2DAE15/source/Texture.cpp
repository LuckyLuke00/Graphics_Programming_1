#include "pch.h"
#include "Texture.h"

namespace dae
{
	ID3D11Texture2D* Texture::m_pResource = nullptr;
	ID3D11ShaderResourceView* Texture::m_pShaderResourceView = nullptr;

	Texture::Texture(SDL_Surface* pSurface)
		: m_pSurface{ pSurface },
		m_pSurfacePixels{ reinterpret_cast<uint32_t*>(m_pSurface->pixels) }
	{
	}

	Texture::~Texture()
	{
		if (m_pShaderResourceView)
		{
			m_pShaderResourceView->Release();
			m_pShaderResourceView = nullptr;
		}

		if (m_pResource)
		{
			m_pResource->Release();
			m_pResource = nullptr;
		}
	}

	Texture* Texture::LoadFromFile(ID3D11Device* pDevice, const std::string& path)
	{
		SDL_Surface* pSurface{ IMG_Load(path.c_str()) };
		if (!pSurface)
		{
			std::cout << "Failed to load texture from file: " << path << "\n";
			return nullptr;
		}

		DXGI_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM };
		D3D11_TEXTURE2D_DESC desc{};
		desc.Width = pSurface->w;
		desc.Height = pSurface->h;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = pSurface->pixels;
		initData.SysMemPitch = static_cast<UINT>(pSurface->pitch);
		initData.SysMemSlicePitch = static_cast<UINT>(pSurface->h * pSurface->pitch);

		HRESULT hr{ pDevice->CreateTexture2D(&desc, &initData, &m_pResource) };
		if (FAILED(hr))
		{
			std::cout << "Texture::LoadFromFile() failed: " << std::hex << hr << '\n';
			return nullptr;
		}

		D3D11_SHADER_RESOURCE_VIEW_DESC SRVDesc{};
		SRVDesc.Format = format;
		SRVDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Texture2D.MipLevels = 1;

		hr = pDevice->CreateShaderResourceView(m_pResource, &SRVDesc, &m_pShaderResourceView);
		if (FAILED(hr))
		{
			std::cout << "Texture::LoadFromFile() failed: " << std::hex << hr << '\n';
			return nullptr;
		}

		SDL_FreeSurface(pSurface);

		return new Texture{ IMG_Load(path.c_str()) };
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
