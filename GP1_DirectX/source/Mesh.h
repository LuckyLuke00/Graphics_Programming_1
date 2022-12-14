#pragma once

class Effect;

// Create the Vertex_PosCol struct
struct Vertex_PosCol
{
	dae::Vector3 pos;
	dae::Vector3 col;
};

class Mesh
{
public:
	Mesh() = default;
	Mesh(ID3D11Device* pDevice, const std::vector<Vertex_PosCol>& vertices, const std::vector<uint32_t>& indices);

	Mesh(const Mesh& other) = delete;
	Mesh(Mesh&& other) = delete;
	Mesh& operator=(const Mesh& other) = delete;
	Mesh& operator=(Mesh&& other) = delete;

	~Mesh();

	void Render(ID3D11DeviceContext* pDeviceContext, const dae::Matrix& WorldViewProjection) const;

private:
	Effect* m_pEffect{};
	ID3DX11EffectTechnique* m_pTechnique{};

	ID3D11Buffer* m_pIndexBuffer{};
	ID3D11Buffer* m_pVertexBuffer{};
	ID3D11InputLayout* m_pInputLayout{};

	uint32_t m_NumIndices{};
};
