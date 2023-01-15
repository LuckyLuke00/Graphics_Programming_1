#pragma once

namespace dae
{
	class Texture
	{
	public:
		virtual ~Texture();

		static Texture* LoadFromFile(ID3D11Device* pDevice, const std::string& path);

		// Getters
		ID3D11ShaderResourceView* GetSRV() const { return m_pShaderResourceView; }

	private:
		Texture() = default;
		static ID3D11Texture2D* m_pResource;
		static ID3D11ShaderResourceView* m_pShaderResourceView;
	};
}
