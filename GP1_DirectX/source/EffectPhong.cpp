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
		m_pMatWorldViewProjVariable = GetEffect()->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable is invalid\n";
		}

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

	void EffectPhong::SetDiffuseMap(const Texture* pDiffuseMap)
	{
		m_pDiffuseMapVariable->SetResource(pDiffuseMap->GetSRV());
	}

	void EffectPhong::SetNormalMap(const Texture* pNormalMap)
	{
		m_pNormalMapVariable->SetResource(pNormalMap->GetSRV());
	}

	void EffectPhong::SetSpecularMap(const Texture* pSpecularMap)
	{
		m_pSpecularMapVariable->SetResource(pSpecularMap->GetSRV());
	}

	void EffectPhong::SetGlossinessMap(const Texture* pGlossinessMap)
	{
		m_pGlossMapVariable->SetResource(pGlossinessMap->GetSRV());
	}
}
