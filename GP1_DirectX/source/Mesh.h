#pragma once

namespace dae
{
	class EffectPhong;
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

		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void RotateY(const float degrees);
		void CycleTechniques();

		// Setters
		void SetDiffuse(const Texture* diffuse);
		void SetNormal(const Texture* normal);
		void SetGloss(const Texture* gloss);
		void SetSpecular(const Texture* specular);

		void SetMatrices(const Matrix& viewProj, const Matrix& invView) const;

	private:
		EffectPhong* m_pEffect{};

		ID3D11Buffer* m_pIndexBuffer{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3DX11EffectTechnique* m_pTechnique{};
		uint32_t m_NumIndices{};

		Matrix m_RotationMatrix{};
	};
}
