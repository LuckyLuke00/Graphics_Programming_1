#include "pch.h"
#include "Effect.h"
#include "Texture.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		: m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
		CaptureTechniques();

		//m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		//if (!m_pMatWorldViewProjVariable->IsValid())
		//{
		//	std::wcout << L"m_pMatWorldViewProjVariable is invalid\n";
		//}

		//m_pMatWorldVariable = m_pEffect->GetVariableByName("gWorld")->AsMatrix();
		//if (!m_pMatWorldVariable->IsValid())
		//{
		//	std::wcout << L"m_pMatWorldVariable is invalid\n";
		//}

		//m_pMatInvViewVariable = m_pEffect->GetVariableByName("gViewInverse")->AsMatrix();
		//if (!m_pMatInvViewVariable->IsValid())
		//{
		//	std::wcout << L"m_pMatInvViewVariable is invalid\n";
		//}

		//m_pDiffuseMapVariable = m_pEffect->GetVariableByName("gDiffuseMap")->AsShaderResource();
		//if (!m_pDiffuseMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pDiffuseMapVariable is invalid\n";
		//}

		//// Save the normal texture variable of the effect as a member variable
		//m_pNormalMapVariable = m_pEffect->GetVariableByName("gNormalMap")->AsShaderResource();
		//if (!m_pNormalMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pNormalMapVariable is invalid\n";
		//}

		//// Save the specular texture variable of the effect as a member variable
		//m_pSpecularMapVariable = m_pEffect->GetVariableByName("gSpecularMap")->AsShaderResource();
		//if (!m_pSpecularMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pSpecularMapVariable is invalid\n";
		//}

		//// Save the glossiness texture variable of the effect as a member variable
		//m_pGlossMapVariable = m_pEffect->GetVariableByName("gGlossinessMap")->AsShaderResource();
		//if (!m_pGlossMapVariable->IsValid())
		//{
		//	std::wcout << L"m_pGlossinessMapVariable is invalid\n";
		//}
	}

	Effect::~Effect()
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}

	ID3DX11Effect* Effect::LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile)
	{
		HRESULT result;
		ID3D10Blob* pErrorBlob{ nullptr };
		ID3DX11Effect* pEffect;

		DWORD shaderFlags{ 0 };
#if defined( DEBUG ) || defined( _DEBUG )
		shaderFlags |= D3D10_SHADER_DEBUG;
		shaderFlags |= D3D10_SHADER_SKIP_OPTIMIZATION;
#endif

		result = D3DX11CompileEffectFromFile
		(
			assetFile.c_str(),
			nullptr,
			nullptr,
			shaderFlags,
			0,
			pDevice,
			&pEffect,
			&pErrorBlob
		);

		if (FAILED(result))
		{
			if (pErrorBlob != nullptr)
			{
				const char* pError{ static_cast<const char*>(pErrorBlob->GetBufferPointer()) };

				std::wstringstream ss;
				for (unsigned int i{ 0 }; i < pErrorBlob->GetBufferSize(); ++i)
					ss << pError[i];

				OutputDebugStringW(ss.str().c_str());
				pErrorBlob->Release();
				pErrorBlob = nullptr;

				std::wcout << ss.str() << '\n';
			}
			else
			{
				std::wstringstream ss;
				ss << "EffectLoader: Failed to CreateEffectFromFile!\nPath: " << assetFile;
				std::wcout << ss.str() << '\n';
				return nullptr;
			}
		}

		return pEffect;
	}

	//void Effect::SetDiffuseMap(const Texture* pDiffuseMap)
	//{
	//	m_pDiffuseMapVariable->SetResource(pDiffuseMap->GetSRV());
	//}

	//void Effect::SetNormalMap(const Texture* pNormalMap)
	//{
	//	m_pNormalMapVariable->SetResource(pNormalMap->GetSRV());
	//}

	//void Effect::SetSpecularMap(const Texture* pSpecularMap)
	//{
	//	m_pSpecularMapVariable->SetResource(pSpecularMap->GetSRV());
	//}

	//void Effect::SetGlossinessMap(const Texture* pGlossinessMap)
	//{
	//	m_pGlossMapVariable->SetResource(pGlossinessMap->GetSRV());
	//}

	void Effect::SetTechnique(ID3DX11EffectTechnique* pTechnique)
	{
		if (!pTechnique->IsValid())
		{
			std::wcout << L"Technique is invalid\n";
			return;
		}

		m_pTechnique = pTechnique;

		PrintTechnique();
	}

	void Effect::CaptureTechniques()
	{
		D3DX11_EFFECT_DESC effectDesc{};
		m_pEffect->GetDesc(&effectDesc);

		const uint32_t nrOfTechniques{ effectDesc.Techniques };

		m_Techniques.reserve(nrOfTechniques);

		for (uint32_t i{ 0 }; i < nrOfTechniques; ++i)
		{
			ID3DX11EffectTechnique* pTechnique{ m_pEffect->GetTechniqueByIndex(i) };

			// Only add the technique to the vector if it is valid
			if (pTechnique->IsValid())
			{
				m_Techniques.emplace_back(pTechnique);
			}
		}

		// Set the default technique to the first one, but only if there is one
		if (m_Techniques.empty())
		{
			std::wcout << L"No valid techniques found!\n";
			m_pTechnique = nullptr;
			return;
		}

		SetTechnique(m_Techniques.front());
	}

	void Effect::PrintTechnique() const
	{
		D3DX11_TECHNIQUE_DESC techniqueDesc{};
		m_pTechnique->GetDesc(&techniqueDesc);
		std::wcout << "Current Technique: " << techniqueDesc.Name << '\n';
	}
}
