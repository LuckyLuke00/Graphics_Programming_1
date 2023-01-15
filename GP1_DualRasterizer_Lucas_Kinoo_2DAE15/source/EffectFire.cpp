#include "pch.h"
#include "EffectFire.h"
#include "Texture.h"

namespace dae
{
	EffectFire::EffectFire(ID3D11Device* pDevice, const std::wstring& assetFile)
		: Effect{ pDevice, assetFile }
	{
		InitVariables();
	}

	void EffectFire::SetMatrices(const Matrix& world, const Matrix& viewProj, const Matrix& invView) const
	{
		Effect::GetMatWorldViewProjVariable()->SetMatrix(reinterpret_cast<const float*>(&viewProj));
	}

	void EffectFire::SetDiffuse(const Texture* diffuse)
	{
		m_pDiffuseMapVariable->SetResource(diffuse->GetSRV());
	}

	void EffectFire::SetNormal(const Texture* normal)
	{
	}

	void EffectFire::SetGloss(const Texture* gloss)
	{
	}

	void EffectFire::SetSpecular(const Texture* specular)
	{
	}

	void EffectFire::InitVariables()
	{
		m_pDiffuseMapVariable = GetEffect()->GetVariableByName("gDiffuseMap")->AsShaderResource();
		if (!m_pDiffuseMapVariable->IsValid())
		{
			std::wcout << L"m_pDiffuseMapVariable is invalid\n";
		}
	}
}
