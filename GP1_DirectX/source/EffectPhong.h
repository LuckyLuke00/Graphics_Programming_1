#pragma once
#include "Effect.h"

namespace dae
{
	class Texture;

	class EffectPhong final : public Effect
	{
	public:
		explicit EffectPhong(ID3D11Device* pDevice, const std::wstring& assetFile);

		// Getters
		ID3DX11EffectMatrixVariable* GetMatWorldVariable() const { return m_pMatWorldVariable; }
		ID3DX11EffectMatrixVariable* GetMatInvViewVariable() const { return m_pMatInvViewVariable; }

		// Setters
		void SetMatrices(const Matrix& world, const Matrix& viewProj, const Matrix& invView) const override;

		void SetDiffuse(const Texture* diffuse) override;
		void SetNormal(const Texture* normal) override;
		void SetGloss(const Texture* gloss) override;
		void SetSpecular(const Texture* specular) override;
	private:
		// Matrices
		ID3DX11EffectMatrixVariable* m_pMatWorldVariable{};
		ID3DX11EffectMatrixVariable* m_pMatInvViewVariable{};

		// Textures
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pNormalMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pSpecularMapVariable{};
		ID3DX11EffectShaderResourceVariable* m_pGlossMapVariable{};

		void InitVariables();
	};
}
