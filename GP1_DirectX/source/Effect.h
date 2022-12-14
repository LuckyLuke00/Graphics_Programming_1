#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:
		Effect() = default;
		explicit Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

		Effect(const Effect& other) = delete;
		Effect(Effect&& other) noexcept = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect& operator=(Effect&& other) noexcept = delete;

		~Effect();

		// Getter functions
		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const { return m_pMatWorldViewProjVariable; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
		void SetDiffuseMap(const Texture* pDiffuseTexture);

	private:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectTechnique* m_pTechnique{};
	};
}
