#include "pch.h"
#include "Mesh.h"

#include "Effect.h"
#include "DataTypes.h"

namespace dae
{
	Mesh::Mesh(ID3D11Device* pDevice, const std::vector<Vertex_In>& vertices, const std::vector<uint32_t>& indices)
		: m_pEffect{ new Effect{ pDevice, std::wstring{ L"Resources/PosCol3D.fx" } } },
		m_pTechnique{ m_pEffect->GetTechnique() }
	{
		//Create vertex layout
		static constexpr uint32_t numElements{ 3 };
		D3D11_INPUT_ELEMENT_DESC vertexDesc[numElements]{};

		vertexDesc[0].SemanticName = "POSITION";
		vertexDesc[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[1].SemanticName = "COLOR";
		vertexDesc[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexDesc[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		vertexDesc[2].SemanticName = "TEXCOORD";
		vertexDesc[2].Format = DXGI_FORMAT_R32G32_FLOAT;
		vertexDesc[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		vertexDesc[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

		//Create vertex buffer
		D3D11_BUFFER_DESC bd{};
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(Vertex_In) * static_cast<uint32_t>(vertices.size());
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA initData{};
		initData.pSysMem = vertices.data();

		HRESULT result{ pDevice->CreateBuffer(&bd, &initData, &m_pVertexBuffer) };
		if (FAILED(result))
			return;

		//Create input layout
		D3DX11_PASS_DESC passDesc{};
		m_pTechnique->GetPassByIndex(0)->GetDesc(&passDesc);

		result = pDevice->CreateInputLayout
		(
			vertexDesc,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pInputLayout
		);

		if (FAILED(result))
			return;

		//Create index buffer
		m_NumIndices = static_cast<uint32_t>(indices.size());
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.ByteWidth = sizeof(uint32_t) * m_NumIndices;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = 0;
		bd.MiscFlags = 0;
		initData.pSysMem = indices.data();
		result = pDevice->CreateBuffer(&bd, &initData, &m_pIndexBuffer);

		if (FAILED(result))
			return;
	}

	Mesh::~Mesh()
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = nullptr;

		m_pInputLayout->Release();
		m_pInputLayout = nullptr;

		m_pVertexBuffer->Release();
		m_pVertexBuffer = nullptr;

		delete m_pEffect;
		m_pEffect = nullptr;
	}

	void Mesh::Render(ID3D11DeviceContext* pDeviceContext, const Matrix& WorldViewProjection) const
	{
		//1. Set primitive topology
		pDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//2. Set input layout
		pDeviceContext->IASetInputLayout(m_pInputLayout);

		//3. Set vertex buffer
		constexpr UINT stride{ sizeof(Vertex_In) };
		constexpr UINT offset{ 0 };

		const Matrix WVP{ m_WorldMatrix * WorldViewProjection };

		pDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		m_pEffect->GetMatWorldViewProjVariable()->SetMatrix(reinterpret_cast<const float*>(&WVP));

		//4. Set index buffer
		pDeviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		//5. Draw
		D3DX11_TECHNIQUE_DESC techDesc{};
		m_pEffect->GetTechnique()->GetDesc(&techDesc);
		for (UINT p{ 0 }; p < techDesc.Passes; ++p)
		{
			m_pEffect->GetTechnique()->GetPassByIndex(p)->Apply(0, pDeviceContext);
			pDeviceContext->DrawIndexed(m_NumIndices, 0, 0);
		}
	}
	void Mesh::SetTexture(const Texture* texture)
	{
		m_pEffect->SetDiffuseMap(texture);
	}

	void Mesh::RotateY(const float degrees)
	{
		m_WorldMatrix = Matrix::CreateRotationY(degrees * TO_RADIANS) * m_WorldMatrix;
	}
}
