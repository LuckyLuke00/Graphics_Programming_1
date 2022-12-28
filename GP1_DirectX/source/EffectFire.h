#pragma once
#include "Effect.h"

namespace dae
{
	class EffectFire final : public Effect
	{
	public:
		explicit EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile);

		void SetMatrices(const Matrix& world, const Matrix& viewProj, const Matrix& invView) const override;

		void SetDiffuse(const Texture* diffuse) override;
		void SetNormal(const Texture* normal) override;
		void SetGloss(const Texture* gloss) override;
		void SetSpecular(const Texture* specular) override;

	private:
		// Textures
		ID3DX11EffectShaderResourceVariable* m_pDiffuseMapVariable{};

		void InitVariables();
	};
}
