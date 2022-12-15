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

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		// Getter functions
		const std::vector<ID3DX11EffectTechnique*>& GetTechniques() const { return m_Techniques; }
		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const { return m_pMatWorldViewProjVariable; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }

		// Setter functions
		void SetDiffuseMap(const Texture* pDiffuseTexture);
		void SetTechnique(ID3DX11EffectTechnique* pTechnique);

	private:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectTechnique* m_pTechnique{};

		std::vector<ID3DX11EffectTechnique*> m_Techniques;

		void CaptureTechniques();
		void PrintTechnique() const;
	};
}
