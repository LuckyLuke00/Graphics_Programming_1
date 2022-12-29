#include "pch.h"
#include "Renderer.h"

#include "DataTypes.h"
#include "Mesh.h"
#include "Utils.h"
#include "EffectPhong.h"
#include "EffectFire.h"

namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow{ pWindow },
		m_ClearColor{ m_HardwareColor }
	{
		//Initialize
		SDL_GetWindowSize(pWindow, &m_Width, &m_Height);

		//Initialize DirectX pipeline
		if (const HRESULT result{ InitializeDirectX() }; result == S_OK)
		{
			m_IsInitialized = true;
			std::cout << "DirectX is initialized and ready!\n";
		}
		else
		{
			std::cout << "DirectX initialization failed!\n";
		}

		InitCamera();
		InitVehicle();
	}

	Renderer::~Renderer()
	{
		if (m_pDeviceContext)
		{
			m_pDepthStencilBuffer->Release();
			m_pDepthStencilBuffer = nullptr;

			m_pDepthStencilView->Release();
			m_pDepthStencilView = nullptr;

			m_pRenderTargetBuffer->Release();
			m_pRenderTargetBuffer = nullptr;

			m_pRenderTargetView->Release();
			m_pRenderTargetView = nullptr;

			m_pSwapChain->Release();
			m_pSwapChain = nullptr;

			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
			m_pDeviceContext->Release();
			m_pDeviceContext = nullptr;

			m_pDevice->Release();
			m_pDevice = nullptr;
		}

		// Clean up meshes
		for (Mesh* pMesh : m_pMeshes)
		{
			delete pMesh;
			pMesh = nullptr;
		}

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
		if (!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		// Only clear when nothing has been drawn yet
		//constexpr ColorRGB clearColor{ .39f, .59f, .93f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &m_ClearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		for (const Mesh* pMesh : m_pMeshes)
		{
			pMesh->Render(m_pDeviceContext);
		}

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::ToggleClearColor()
	{
		m_EnableUniformColor = !m_EnableUniformColor;
		m_ClearColor = m_EnableUniformColor ? m_UniformColor : m_HardwareColor;

		// Set console color to dark yellow
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 6);
		std::cout << "**(SHARED) Uniform ClearColor " << (m_EnableUniformColor ? "ON" : "OFF") << '\n';

		// Reset console color dark gray
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 8);
	}

	void Renderer::ToggleFireFXMesh()
	{
		m_pMeshes.back()->ToggleVisibility();
	}

	void Renderer::InitCamera()
	{
		const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

		m_pCamera = new Camera{};
		m_pCamera->Initialize(aspectRatio, 45.f, { .0f, .0f, -50.f });
	}

	void Renderer::InitVehicle()
	{
		// Initialize vehicle
		std::vector<Vertex_In> vertices;
		std::vector<uint32_t> indices;
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		EffectPhong* pVehicleEffect{ new EffectPhong{ m_pDevice, L"Resources/PosCol3D.fx" } };
		m_pMeshes.emplace_back(new Mesh{ m_pDevice, pVehicleEffect, vertices, indices });

		// Set vehicle diffuse
		const Texture* pTexture{ Texture::LoadFromFile(m_pDevice, "Resources/vehicle_diffuse.png") };
		m_pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetDiffuse(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle normal
		pTexture = Texture::LoadFromFile(m_pDevice, "Resources/vehicle_normal.png");
		m_pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetNormal(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle gloss
		pTexture = Texture::LoadFromFile(m_pDevice, "Resources/vehicle_gloss.png");
		m_pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetGloss(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Set vehicle specular
		pTexture = Texture::LoadFromFile(m_pDevice, "Resources/vehicle_specular.png");
		m_pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.front()->SetSpecular(pTexture);
		delete pTexture;
		pTexture = nullptr;

		// Initialize fire effect
		vertices.clear();
		indices.clear();
		Utils::ParseOBJ("Resources/fireFX.obj", vertices, indices);

		EffectFire* pFireEffect{ new EffectFire{ m_pDevice, L"Resources/FireEffect3D.fx" } };
		m_pMeshes.emplace_back(new Mesh{ m_pDevice, pFireEffect, vertices, indices });

		// Set FireFX diffuse
		pTexture = Texture::LoadFromFile(m_pDevice, "Resources/fireFX_diffuse.png");
		m_pDeviceContext->GenerateMips(pTexture->GetSRV());
		m_pMeshes.back()->SetDiffuse(pTexture);
		delete pTexture;
		pTexture = nullptr;
	}

	HRESULT Renderer::InitializeDirectX()
	{
		//1. Create Device & DeviceContext
		//=====
		D3D_FEATURE_LEVEL featureLevel{ D3D_FEATURE_LEVEL_11_1 };
		uint32_t createDeviceFlags{ 0 };
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

		HRESULT result
		{
			D3D11CreateDevice
			(
				nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
				createDeviceFlags, &featureLevel, 1,
				D3D11_SDK_VERSION, &m_pDevice, nullptr,
				&m_pDeviceContext
			)
		};

		if (FAILED(result))
			return result;

		//Create DXGI Factory
		IDXGIFactory* pDxgiFactory{};
		result = CreateDXGIFactory(__uuidof(IDXGIFactory), reinterpret_cast<void**>(&pDxgiFactory));
		if (FAILED(result))
			return result;

		//2. Create Swapchain
		//=====
		DXGI_SWAP_CHAIN_DESC swapChainDesc{};
		swapChainDesc.BufferDesc.Width = m_Width;
		swapChainDesc.BufferDesc.Height = m_Height;
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 1;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 60;
		swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		swapChainDesc.SampleDesc.Count = 1;
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = 1;
		swapChainDesc.Windowed = true;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
		swapChainDesc.Flags = 0;

		//Get the handle (HWND) from the SDL Backbuffer
		SDL_SysWMinfo sysWMInfo{};
		SDL_VERSION(&sysWMInfo.version);
		SDL_GetWindowWMInfo(m_pWindow, &sysWMInfo);
		swapChainDesc.OutputWindow = sysWMInfo.info.win.window;

		//Create SwapChain
		result = pDxgiFactory->CreateSwapChain(m_pDevice, &swapChainDesc, &m_pSwapChain);
		if (FAILED(result))
			return result;

		// Release the factory
		pDxgiFactory->Release();
		pDxgiFactory = nullptr;

		//3. Create DepthStencil (DS) & DepthStencilView (DSV)
		//Resource
		D3D11_TEXTURE2D_DESC depthStencilDesc{};
		depthStencilDesc.Width = m_Width;
		depthStencilDesc.Height = m_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		//View
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = depthStencilDesc.Format;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;

		result = m_pDevice->CreateTexture2D(&depthStencilDesc, nullptr, &m_pDepthStencilBuffer);
		if (FAILED(result))
			return result;

		result = m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView);
		if (FAILED(result))
			return result;

		//4. Create RenderTarget (RT) & RenderTargetView (RTV)
		//=====

		//Resource
		result = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&m_pRenderTargetBuffer));
		if (FAILED(result))
			return result;

		//View
		result = m_pDevice->CreateRenderTargetView(m_pRenderTargetBuffer, nullptr, &m_pRenderTargetView);
		if (FAILED(result))
			return result;

		//5. Bind RTV & DSV to Output Merger Stage
		//=====
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		//6. Set to Viewport
		//=====
		D3D11_VIEWPORT viewport{};
		viewport.Width = static_cast<float>(m_Width);
		viewport.Height = static_cast<float>(m_Height);
		viewport.TopLeftX = .0f;
		viewport.TopLeftY = .0f;
		viewport.MinDepth = .0f;
		viewport.MaxDepth = 1.f;
		m_pDeviceContext->RSSetViewports(1, &viewport);

		return result;
	}
}
