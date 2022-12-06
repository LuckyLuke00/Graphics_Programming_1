#pragma once
class Effect
{
public:
	Effect() = default;
	Effect(ID3D11Device* pDevice, const std::wstring& assetFile);

	Effect(const Effect& other) = delete;
	Effect(Effect&& other) noexcept = delete;
	Effect& operator=(const Effect& other) = delete;
	Effect& operator=(Effect&& other) noexcept = delete;

	~Effect();

	// Getter functions
	ID3DX11Effect* GetEffect() const { return m_pEffect; }
	ID3DX11EffectTechnique* GetTechnique() const { return m_pTechnique; }
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);

private:
	ID3DX11Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};
};
