#include "pch.h"
#include "Effect.h"

namespace dae
{
	Effect::Effect(ID3D11Device* pDevice, const std::wstring& assetFile)
		: m_pEffect{ LoadEffect(pDevice, assetFile) }
	{
		InitVariables();
		CaptureTechniques();
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

	void Effect::InitVariables()
	{
		m_pMatWorldViewProjVariable = m_pEffect->GetVariableByName("gWorldViewProj")->AsMatrix();
		if (!m_pMatWorldViewProjVariable->IsValid())
		{
			std::wcout << L"m_pMatWorldViewProjVariable is invalid\n";
		}
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
