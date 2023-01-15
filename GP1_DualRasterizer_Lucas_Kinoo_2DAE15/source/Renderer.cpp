#include "pch.h"
#include "Renderer.h"

#include "Camera.h"
#include "DataTypes.h"
#include "EffectFire.h"
#include "EffectPhong.h"
#include "HardwareRasterizer.h"
#include "Mesh.h"
#include "Texture.h"
#include "Utils.h"

namespace dae {
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow{ pWindow },
		m_pCamera{ new Camera{} },
		m_pHardwareRasterizer{ new HardwareRasterizer{ pWindow } }
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		InitCamera();
		InitVehicle();

		PrintKeybinds();
	}

	Renderer::~Renderer()
	{
		// Destroy the hardware rasterizer
		delete m_pHardwareRasterizer;
		m_pHardwareRasterizer = nullptr;

		// Clean up meshes
		for (Mesh* pMesh : m_pMeshes)
		{
			delete pMesh;
			pMesh = nullptr;
		}

		// Destroy the camera
		delete m_pCamera;
		m_pCamera = nullptr;
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		for (Mesh* pMesh : m_pMeshes)
		{
			if (m_RotateMesh) pMesh->RotateY(m_RotationSpeed * pTimer->GetElapsed());
			pMesh->SetMatrices(m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix(), m_pCamera->GetInvViewMatrix());
		}
	}

	void Renderer::Render() const
	{
		switch (m_RasterizerMode)
		{
		case RasterizerMode::Hardware:
			m_pHardwareRasterizer->Render(m_pMeshes, m_UniformClearColor ? m_UniformColor : m_HardwareColor);
			break;
		}
	}

	bool Renderer::IsHardwareMode() const
	{
		return m_RasterizerMode == RasterizerMode::Hardware;
	}

	bool Renderer::ToggleFireFxMesh()
	{
		if (m_RasterizerMode != RasterizerMode::Hardware) return false;

		return m_pMeshes.back()->ToggleVisibility();
	}

	void Renderer::CycleCullMode()
	{
		static constexpr int enumSize{ sizeof(CullMode) - 1 };
		m_CullMode = static_cast<CullMode>((static_cast<int>(m_CullMode) + 1) % enumSize);

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
		std::cout << "**(SHARED) CullMode = ";
		switch (m_CullMode)
		{
		case CullMode::Back:
			std::cout << "BACK\n";
			m_pHardwareRasterizer->SetCullMode(D3D11_CULL_BACK);
			break;
		case CullMode::Front:
			std::cout << "FRONT\n";
			m_pHardwareRasterizer->SetCullMode(D3D11_CULL_FRONT);
			break;
		case CullMode::None:
			std::cout << "NONE\n";
			m_pHardwareRasterizer->SetCullMode(D3D11_CULL_NONE);
			break;
		}
	}

	void Renderer::CycleTechniques() const
	{
		std::string techniqueName{};
		for (const auto& pMesh : m_pMeshes)
		{
			techniqueName = pMesh->CycleTechniques();
		}

		// Make the string uppercase
		std::ranges::transform(techniqueName.begin(), techniqueName.end(), techniqueName.begin(), ::toupper);

		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
		std::cout << "**(HARDWARE) Sampler Filter = " << techniqueName << '\n';
	}

	void Renderer::InitCamera()
	{
		m_pCamera->Initialize(static_cast<float>(m_Width) / static_cast<float>(m_Height), 45.f, { .0f, .0f, -50.f });
	}

	void Renderer::InitVehicle()
	{
		// Initialize vehicle
		std::vector<Vertex_In> vertices;
		std::vector<uint32_t> indices;
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		ID3D11DeviceContext* pDeviceContext{ m_pHardwareRasterizer->GetDeviceContext() };
		ID3D11Device* pDevice{ m_pHardwareRasterizer->GetDevice() };

		EffectPhong* pVehicleEffect{ new EffectPhong{ pDevice, L"Resources/PosCol3D.fx" } };
		m_pMeshes.emplace_back(new Mesh{ pDevice, pVehicleEffect, vertices, indices });

		// Set vehicle diffuse
		const Texture* pTexture{ Texture::LoadFromFile(pDevice, "Resources/vehicle_diffuse.png") };
		pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetDiffuse(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle normal
		pTexture = Texture::LoadFromFile(pDevice, "Resources/vehicle_normal.png");
		pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetNormal(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle gloss
		pTexture = Texture::LoadFromFile(pDevice, "Resources/vehicle_gloss.png");
		pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetGloss(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle specular
		pTexture = Texture::LoadFromFile(pDevice, "Resources/vehicle_specular.png");
		pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetSpecular(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Initialize fire effect
		vertices.clear();
		indices.clear();
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		EffectFire* pFireEffect{ new EffectFire{ pDevice, L"Resources/FireEffect3D.fx" } };
		m_pMeshes.emplace_back(new Mesh{ pDevice, pFireEffect, vertices, indices });

		// Set FireFX diffuse
		pTexture = Texture::LoadFromFile(pDevice, "Resources/fireFX_diffuse.png");
		pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.back()->SetDiffuse(pTexture);
		delete pTexture;
		pTexture = nullptr;
	}

	void Renderer::PrintKeybinds() const
	{
		// Print SHARED keybinds
		// Change console text color to yellow
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
		std::cout << "[Key Bindings - SHARED]\n"
			<< "   [F1]  Toggle Rasterizer Mode (HARDWARE/SOFTWARE)\n"
			<< "   [F2]  Toggle Vehicle Rotation (ON/OFF)\n"
			<< "   [F9]  Cycle CullMode (BACK/FRONT/NONE)\n"
			<< "   [F10] Toggle Uniform ClearColor (ON/OFF)\n"
			<< "   [F11] Toggle Print FPS (ON/OFF)\n\n";

		// Print HARDWARE keybinds
		// Change console text color to green
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 2);
		std::cout << "[Key Bindings - HARDWARE]\n"
			<< "   [F3] Toggle FireFX (ON/OFF)\n"
			<< "   [F4] Cycle Sampler State (POINT/LINEAR/ANISOTROPIC)\n\n";

		// Print SOFTWARE
		// Change console text color to purple
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 5);
		std::cout << "[Key Bindings - SOFTWARE]\n"
			<< "   [F5] Cycle Shading Mode (COMBINED/OBSERVED_AREA/DIFFUSE/SPECULAR)\n"
			<< "   [F6] Toggle NormalMap (ON/OFF)\n"
			<< "   [F7] Toggle DepthBuffer Visualization (ON/OFF)\n"
			<< "   [F8] Toggle BoundingBox Visualization (ON/OFF)\n\n\n";
	}
}
