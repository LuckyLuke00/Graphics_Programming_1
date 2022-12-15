#pragma once

namespace dae
{
	class Effect;
	class Texture;
	struct Vertex_In;

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices);

		Mesh(const Mesh& other) = delete;
		Mesh(Mesh&& other) = delete;
		Mesh& operator=(const Mesh& other) = delete;
		Mesh& operator=(Mesh&& other) = delete;

		~Mesh();

		void Render(ID3D11DeviceContext* pDeviceContext, const dae::Matrix& WorldViewProjection) const;
		void SetTexture(const Texture* texture);
		void RotateY(const float degrees);
		void CycleTechniques();

	private:
		Effect* m_pEffect{};

		ID3D11Buffer* m_pIndexBuffer{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3DX11EffectTechnique* m_pTechnique{};
		uint32_t m_NumIndices{};

		Matrix m_WorldMatrix{};
	};
}
