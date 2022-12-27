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
		ID3DX11EffectMatrixVariable* GetMatWorldVariable() const { return m_pMatWorldVariable; }
		ID3DX11EffectMatrixVariable* GetMatInvViewVariable() const { return m_pMatInvViewVariable; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }

		// Setter functions
		void SetDiffuseMap(const Texture* pDiffuseMap);
		void SetNormalMap(const Texture* pNormalMap);
		void SetSpecularMap(const Texture* pSpecularMap);
		void SetGlossinessMap(const Texture* pGlossinessMap);

		void SetTechnique(ID3DX11EffectTechnique* pTechnique);

	private:
		ID3DX11Effect* m_pEffect{};
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};
		ID3DX11EffectTechnique* m_pTechnique{};

		// Textures
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable{};

		std::vector<ID3DX11EffectTechnique*> m_Techniques;

		void CaptureTechniques();
		void PrintTechnique() const;
	};
}
