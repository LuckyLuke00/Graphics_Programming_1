#include "pch.h"
#include "Renderer.h"

#include "DataTypes.h"
#include "Mesh.h"
#include "Utils.h"

namespace dae
{
	Renderer::Renderer(SDL_Window* pWindow) :
		m_pWindow{ pWindow }
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

		//InitQuad();
		InitVehicle(true);
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

			delete m_pMesh;
			m_pMesh = nullptr;
		}

		delete m_pCamera;
		m_pCamera = nullptr;

		if (m_pTexture)
		{
			delete m_pTexture;
			m_pTexture = nullptr;
		}
	}

	void Renderer::Update(const Timer* pTimer)
	{
		m_pCamera->Update(pTimer);

		if (m_RotateMesh)
		{
			m_pMesh->RotateY(m_RotationSpeed * .5f * pTimer->GetElapsed());
		}
	}

	void Renderer::Render() const
	{
		if (!m_IsInitialized)
			return;

		//1. CLEAR RTV & DSV
		// Only clear when nothing has been drawn yet
		constexpr ColorRGB clearColor{ .0f, .0f, .3f };
		m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, &clearColor.r);
		m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);

		//2. SET PIPELINE + INVOKE DRAW CALLS (= RENDER)
		m_pMesh->Render(m_pDeviceContext, m_pCamera->GetViewMatrix() * m_pCamera->GetProjectionMatrix());

		//3. PRESENT BACKBUFFER (SWAP)
		m_pSwapChain->Present(0, 0);
	}

	void Renderer::InitQuad(const bool rotate)
	{
		m_RotateMesh = rotate;

		const std::vector<Vertex_In> vertices
		{
			{ { -3.f,  3.f, -2.f }, colors::White, { .0f, .0f } },
			{ {  .0f,  3.f, -2.f }, colors::White, { .5f, .0f } },
			{ {  3.f,  3.f, -2.f }, colors::White, { 1.f, .0f } },
			{ { -3.f,  .0f, -2.f }, colors::White, { .0f, .5f } },
			{ {  .0f,  .0f, -2.f }, colors::White, { .5f, .5f } },
			{ {  3.f,  .0f, -2.f }, colors::White, { 1.f, .5f } },
			{ { -3.f, -3.f, -2.f }, colors::White, { .0f, 1.f } },
			{ {  .0f, -3.f, -2.f }, colors::White, { .5f, 1.f } },
			{ {  3.f, -3.f, -2.f }, colors::White, { 1.f, 1.f } },
		};

		const std::vector<uint32_t> indices
		{
			3, 0, 1,	1, 4, 3,	4, 1, 2,
			2, 5, 4,	6, 3, 4,	4, 7, 6,
			7, 4, 5,	5, 8, 7,
		};

		const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

		m_pCamera = new Camera{};
		m_pCamera->Initialize(aspectRatio, 45.f, { .0f, .0f, -14.f });

		// Initialize the texture
		m_pTexture = Texture::LoadFromFile(m_pDevice, "Resources/uv_grid_2.png");
		m_pDeviceContext->GenerateMips(m_pTexture->GetSRV());

		// Initialize the mesh
		m_pMesh = new Mesh{ m_pDevice, vertices, indices };
		m_pMesh->SetTexture(m_pTexture);
	}

	void Renderer::InitVehicle(const bool rotate)
	{
		m_RotateMesh = rotate;

		std::vector<Vertex_In> vertices;
		std::vector<uint32_t> indices;
		Utils::ParseOBJ("Resources/vehicle.obj", vertices, indices);

		// Initialize the camera
		const float aspectRatio{ static_cast<float>(m_Width) / static_cast<float>(m_Height) };

		m_pCamera = new Camera{};
		m_pCamera->Initialize(aspectRatio, 45.f, { .0f, .0f, -50.f });

		// Initialize the texture
		m_pTexture = Texture::LoadFromFile(m_pDevice, "Resources/vehicle_diffuse.png");
		m_pDeviceContext->GenerateMips(m_pTexture->GetSRV());

		// Initialize the mesh
		m_pMesh = new Mesh{ m_pDevice, vertices, indices };
		m_pMesh->SetTexture(m_pTexture);
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
