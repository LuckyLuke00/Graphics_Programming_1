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
		ID3DX11Effect* GetEffect() const { return m_pEffect; }
		ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
		const std::vector<ID3DX11EffectTechnique*>& GetTechniques() const { return m_Techniques; }

		// Setter functions
		void SetTechnique(ID3DX11EffectTechnique* pTechnique);	
		
	private:
		ID3DX11Effect* m_pEffect{};
		
		ID3DX11EffectTechnique* m_pTechnique{};
		std::vector<ID3DX11EffectTechnique*> m_Techniques;

		void CaptureTechniques();
		void PrintTechnique() const;
	};
}
