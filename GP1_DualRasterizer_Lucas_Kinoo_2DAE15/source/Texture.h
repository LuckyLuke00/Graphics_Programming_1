#pragma once

namespace dae
{
	class Texture final
	{
	public:
		~Texture();

		static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);
		ColorRGB Sample(const Vector2& uv) const;

		// Getters
		ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; }

	private:
		explicit Texture(ID3D11Device* pDevice, SDL_Surface* pSurface);
		ID3D11Texture2D* m_pResource;
		ID3D11ShaderResourceView* m_pShaderResourceView;

		SDL_Surface* m_pSurface{ nullptr };
		uint32_t* m_pSurfacePixels{ nullptr };
	};
}
