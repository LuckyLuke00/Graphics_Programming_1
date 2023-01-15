#include "pch.h"
#include "EffectPhong.h"
#include "Texture.h"

namespace dae
{
	EffectPhong::EffectPhong(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect{ pDevice, assetFile }
	{
		InitVariables();
	}

	void EffectPhong::InitVariables()
	{
		m_pMatWorldVariable = GetEffect()->GetVariableByName("gWorld")->AsMatrix();
		if (!m_pMatWorldVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldVariable is invalid\n";
		}

		m_pMatInvViewVariable = GetEffect()->GetVariableByName("gViewInverse")->AsMatrix();
		if (!m_pMatInvViewVariable->IsValid())
		{
			std::wcout << L"m_pMatInvViewVariable is invalid\n";
		}

		m_pDiffuseMapVariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable is invalid\n";
		}

		// Save the normal texture variable of the effect as a member variable
		m_pNormalMapVariable = GetEffect()->GetVariableByName("gNormalMap")->AsShaderResource();
		if (!m_pNormalMapVariable->IsValid())
		{
			std::wcout << L"m_pNormalMapVariable is invalid\n";
		}

		// Save the specular texture variable of the effect as a member variable
		m_pSpecularMapVariable = GetEffect()->GetVariableByName("gSpecularMap")->AsShaderResource();
		if (!m_pSpecularMapVariable->IsValid())
		{
			std::wcout << L"m_pSpecularMapVariable is invalid\n";
		}

		// Save the glossiness texture variable of the effect as a member variable
		m_pGlossMapVariable = GetEffect()->GetVariableByName("gGlossinessMap")->AsShaderResource();
		if (!m_pGlossMapVariable->IsValid())
		{
			std::wcout << L"m_pGlossinessMapVariable is invalid\n";
		}
	}

	void EffectPhong::SetMatrices(const Matrix& world, const Matrix& viewProj, const Matrix& invView) const
	{
		Effect::GetMatWorldViewProjVariable()->SetMatrix(reinterpret_cast<const float*>(&viewProj));
		m_pMatWorldVariable->SetMatrix(reinterpret_cast<const float*>(&world));
		m_pMatInvViewVariable->SetMatrix(reinterpret_cast<const float*>(&invView));
	}

	void EffectPhong::SetDiffuse(const Texture* diffuse)
	{
		m_pDiffuseMapVariable->SetResource(diffuse->GetSRV());
	}
	void EffectPhong::SetNormal(const Texture* normal)
	{
		m_pNormalMapVariable->SetResource(normal->GetSRV());
	}
	void EffectPhong::SetGloss(const Texture* gloss)
	{
		m_pGlossMapVariable->SetResource(gloss->GetSRV());
	}
	void EffectPhong::SetSpecular(const Texture* specular)
	{
		m_pSpecularMapVariable->SetResource(specular->GetSRV());
	}
}
