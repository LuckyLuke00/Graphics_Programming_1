#pragma once
#include "Effect.h"

namespace dae
{
	class EffectPhong final : public Effect
	{
	public:
		explicit EffectPhong(ID3D11Device* pDevice, const std::wstring& assetFile);

		// Getters
		ID3DX11EffectMatrixVariable* GetMatWorldViewProjVariable() const { return m_pMatWorldViewProjVariable; }
		ID3DX11EffectMatrixVariable* GetMatWorldVariable() const { return m_pMatWorldVariable; }
		ID3DX11EffectMatrixVariable* GetMatInvViewVariable() const { return m_pMatInvViewVariable; }

		// Setters
		void SetDiffuseMap(const Texture* pDiffuseMap);
		void SetNormalMap(const Texture* pNormalMap);
		void SetSpecularMap(const Texture* pSpecularMap);
		void SetGlossinessMap(const Texture* pGlossinessMap);
	private:
		ID3DX11EffectTechnique* m_pTechnique{};
		std::vector<ID3DX11EffectTechnique*> m_Techniques;

		// Matrices
		ID3DX11EffectMatrixVariable* m_pMatWorldViewProjVariable{};
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
