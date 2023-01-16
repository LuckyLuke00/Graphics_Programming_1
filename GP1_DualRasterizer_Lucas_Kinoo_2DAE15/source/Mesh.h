#pragma once

#include "DataTypes.h"

namespace dae
{
	class Effect;
	struct Vertex_In;
	struct Vertex_Out;
	struct Material;

	class Texture;

	class Mesh
	{
	public:
		explicit Mesh(ID3D11Device* pDevice, Effect* pEffect, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices);

		Mesh(const Mesh&) = delete;
		Mesh(Mesh&&) noexcept = delete;
		Mesh& operator=(const Mesh&) = delete;
		Mesh& operator=(Mesh&&) noexcept = delete;

		~Mesh();

		void Render(ID3D11DeviceContext* pDeviceContext) const;
		void RotateY(const float degrees);
		const char* CycleTechniques();

		bool ToggleVisibility() { m_Visible = !m_Visible; return m_Visible; }

		// Getters
		std::vector<Vertex_Out>& GetVerticesOut() { return m_VerticesOut; }
		const std::vector <Vertex_Out>& GetVerticesOut() const { return m_VerticesOut; }
		const std::vector<uint32_t>& GetIndices() const { return m_Indices; }
		const std::vector<Vertex_In>& GetVertices() const { return m_Vertices; }
		const Matrix& GetWorldMatrix() const { return m_WorldMatrix; }
		const Matrix& GetViewProjMatrix() const { return m_ViewProjMatrix; }
		PrimitiveTopology GetPrimitiveTopology() const { return m_PrimitiveTopology; }

		// Texture Getters
		const Texture* GetDiffuse() const { return m_pDiffuse; }
		const Texture* GetSpecular() const { return m_pSpecular; }
		const Texture* GetNormal() const { return m_pNormal; }
		const Texture* GetGloss() const { return m_pGloss; }

		// Setters
		void SetMatrices(const Matrix& viewProj, const Matrix& invView);
		void SetPosition(const Vector3& position);
		void SetVertices(const std::vector<Vertex_In>& vertices) { m_Vertices = vertices; }
		void SetIndices(const std::vector<uint32_t>& indices) { m_Indices = indices; }

		void SetDiffuse(const Texture* diffuse);
		void SetNormal(const Texture* normal);
		void SetGloss(const Texture* gloss);
		void SetSpecular(const Texture* specular);

	private:
		Effect* m_pEffect{};

		ID3D11Buffer* m_pIndexBuffer{};
		ID3D11Buffer* m_pVertexBuffer{};
		ID3D11InputLayout* m_pInputLayout{};
		ID3DX11EffectTechnique* m_pTechnique{};
		uint32_t m_NumIndices{};

		Matrix m_WorldMatrix{};
		Matrix m_ViewProjMatrix{};

		int m_TechniqueIndex{ 0 };
		bool m_Visible{ true };

		const Texture* m_pDiffuse{};
		const Texture* m_pNormal{};
		const Texture* m_pGloss{};
		const Texture* m_pSpecular{};

		// Software
		std::vector<Vertex_In> m_Vertices{};
		std::vector<Vertex_Out> m_VerticesOut{};
		std::vector<uint32_t> m_Indices{};
		PrimitiveTopology m_PrimitiveTopology{ PrimitiveTopology::TriangleList };
	};
}
