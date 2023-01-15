#pragma once

namespace dae
{
	class Texture;

	class Effect
	{
	public:
		explicit Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

		Effect(const Effect& other) = delete;
		Effect(Effect&& other) noexcept = delete;
		Effect& operator=(const Effect& other) = delete;
		Effect& operator=(Effect&& other) noexcept = delete;

		virtual ~Effect();

		static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

		// Getter functions
		const std::vector<ID3DX11EffectTechnique*>& GetTechniques() const { return m_Techniques; }
		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const { return m_pMatWorldViewProjVariable; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }

		// Setter functions
		void SetTechnique(ID3DX11EffectTechnique* pTechnique);
		virtual void SetMatrices(const Matrix& world, const Matrix& viewProj, const Matrix& invView) const = 0;

		virtual void SetDiffuse(const Texture* diffuse) = 0;
		virtual void SetNormal(const Texture* normal) = 0;
		virtual void SetGloss(const Texture* gloss) = 0;
		virtual void SetSpecular(const Texture* specular) = 0;

	private:
		ID3DX11Effect* m_pEffect{};

		ID3DX11EffectTechnique* m_pTechnique{};
		std::vector<ID3DX11EffectTechnique*> m_Techniques;

		// Matrices
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};

		void InitVariables();
		void CaptureTechniques();
	};
}
