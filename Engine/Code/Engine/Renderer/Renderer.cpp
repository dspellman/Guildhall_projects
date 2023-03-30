#define WIN32_LEAN_AND_MEAN		// Always #define this before #including <windows.h>
#include <windows.h>
#include "Renderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/DefaultShader.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Game/EngineBuildPreferences.hpp"

#if defined ENGINE_DEBUG_RENDER
#include <dxgidebug.h>
#pragma comment( lib, "dxguid.lib" )
#endif

#include "d3d11.h"
#include "dxgi.h"
#include "d3dcompiler.h"

#pragma comment( lib, "d3d11.lib" )	// Link in the OpenGL32.lib static library
#pragma comment( lib, "dxgi.lib" )	// Link in the OpenGL32.lib static library
#pragma comment( lib, "d3dcompiler.lib" )	// Link in the OpenGL32.lib static library

// REMEMBER: CBO structs must have size multiple of 16B, and <16B members cannot straddle 16B boundaries!!

struct CameraConstants
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};

struct ModelConstants
{
	Mat44 ModelMatrix;
	float ModelColor[4];
};

struct LightingConstants
{
	Vec3 sunDirection;
	float sunIntensity;
	float ambientIntensity;
	float padding[3];
};

constexpr int CAMERA_LIGHTING_BUFFER_SLOT = 1;
constexpr int CAMERA_CONSTANT_BUFFER_SLOT = 2;
constexpr int CAMERA_MODEL_BUFFER_SLOT = 3;

Renderer::Renderer(RendererConfig config)
	: m_config(config)
{

}

Renderer::~Renderer()
{

}

void Renderer::Startup()
{		
	HRESULT hr;
#if defined ENGINE_DEBUG_RENDER
	m_dxgiDebugModule = (void*) ::LoadLibraryA("dxgidebug.dll");
	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	hr = ((GetDebugModuleCB) ::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))(__uuidof(IDXGIDebug), &m_dxgiDebug);
	if (FAILED(hr))
	{
		ERROR_AND_DIE("Debug library call failed");
	}
#endif
	// create the DirextX device
	DXGI_SWAP_CHAIN_DESC description = { 0 };
	description.BufferDesc.Width = m_config.m_window->GetClientDimensions().x;
	description.BufferDesc.Height = m_config.m_window->GetClientDimensions().y;
	description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	description.SampleDesc.Count = 1;
	description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	description.BufferCount = 2;
	description.OutputWindow = HWND(m_config.m_window->GetOSWindowHandle());
	description.Windowed = true;
	description.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	m_flags = 0;
#if defined( ENGINE_DEBUG_RENDER )
	m_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
	D3D_FEATURE_LEVEL featureLevel;

	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, m_flags, nullptr,
		0, D3D11_SDK_VERSION, &description, &m_swapChain, &m_device, &featureLevel, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("D3D11CreateDeviceAndSwapChain() returned error #%x", hr));
	}

	ID3D11Texture2D* texture = nullptr;

	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)(&texture));
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("GetBuffer() returned error #%x", hr));
	}

	hr = m_device->CreateRenderTargetView(texture, nullptr, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateRenderTargetView() returned error #%x", hr));
	}
	DX_SAFE_RELEASE(texture);

	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(m_config.m_window->GetClientDimensions().x);
	viewport.Height = static_cast<float>(m_config.m_window->GetClientDimensions().y);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	m_deviceContext->RSSetViewports(1, &viewport);

	SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);

	D3D11_TEXTURE2D_DESC textureDesc = { 0 };
	textureDesc.Width = m_config.m_window->GetClientDimensions().x;
	textureDesc.Height = m_config.m_window->GetClientDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;

//	ID3D11Texture2D* pTexture2D = nullptr;
	hr = m_device->CreateTexture2D(&textureDesc, NULL, &m_depthStencilTexture );
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTexture2D() returned error #%x", hr));
	}

// 	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = { };
// 	depthStencilDesc.Flags = 0;
// 	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
// 	depthStencilDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, NULL, &m_depthStencilView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateDepthStencilView() returned error #%x", hr));
	}
	SetDepthStencilState(DepthTest::ALWAYS, false);

	m_defaultShader = CreateShader("Default", defaultShaderCode);
	BindShaderByName("Default");
//	BindShaderByName("Data/Shaders/Default");

	// check size and reallocate buffer if too small
	m_immediateVBO_PCU = CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
//	BindVertexBuffer(m_immediateVBO_PCU); // TEST DEBUG removed to avoid problems with lack of shader
	m_immediateVBO_PNCU = CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));
//	BindVertexBuffer(m_immediateVBO_PNCU); // TEST DEBUG removed to avoid problems with lack of shader

	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightCBO = CreateConstantBuffer(sizeof(LightingConstants));

	SetBlendMode(BlendMode::ALPHA);

	m_defaultTexture = CreateTextureFromImage(Image(IntVec2(2, 2), Rgba8::WHITE, "white22"));
// 	m_defaultTexture = CreateTextureFromImage(Image(IntVec2(1, 1), Rgba8::WHITE, "white11"));
	BindTexture(nullptr); // sets default texture

	// Create sampler state
	SetSamplerMode(SamplerMode::POINTCLAMP);

	DebugRenderConfig config;
	config.renderer = this;
	config.m_startHidden = false;
	DebugRenderSetParentClock(g_systemClock);
	DebugRenderSystemStartup(config);
};

void Renderer::BeginFrame()
{
	DebugRenderBeginFrame();
};

void Renderer::EndFrame()
{
	DebugRenderEndFrame();
//	HWND windowHandle = HWND(m_config.m_window->GetOSWindowHandle());
//	HDC displayDeviceContext = ::GetDC(windowHandle);
	Sleep(0);
	m_swapChain->Present(0, 0); // first parameter sets vsync
};

void Renderer::BeginCamera(const Camera& camera)
{
	Vec2 bottomLeft = camera.GetOrthoBottomLeft();
	Vec2 topRight = camera.GetOrthoTopRight();
	// start with known values to clear camera state
	SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	BindShader(nullptr);
	SetModelColor(Rgba8::WHITE);
	SetSamplerMode(SamplerMode::POINTCLAMP);
	SetModelMatrix(Mat44());
	SetDepthStencilState(DepthTest::ALWAYS, false);
	SetBlendMode(BlendMode::ALPHA);
	
	// Camera constants
	CameraConstants camConsts;
	camConsts.ProjectionMatrix = camera.GetOrthoOrProjectionMatrix();
	camConsts.ViewMatrix = camera.GetViewMatrix();

	CopyCPUToGPU(&camConsts, sizeof(CameraConstants), m_cameraCBO);
	BindConstantBuffer(CAMERA_CONSTANT_BUFFER_SLOT, m_cameraCBO);

	// Model Constants
	ModelConstants modelConsts;
	Mat44 identity;
	modelConsts.ModelMatrix = identity;
	m_modelColor = Rgba8::WHITE;
	m_modelColor.GetAsFloats(modelConsts.ModelColor);

	CopyCPUToGPU(&modelConsts, sizeof(ModelConstants), m_modelCBO);
	BindConstantBuffer(CAMERA_MODEL_BUFFER_SLOT, m_modelCBO);

	// Lighting Constants
	LightingConstants ligthingConsts;
	ligthingConsts.sunDirection = m_sunDirection.GetForwardNormal();
	ligthingConsts.sunIntensity = m_sunIntensity;
	ligthingConsts.ambientIntensity = m_ambientIntensity;

	CopyCPUToGPU(&ligthingConsts, sizeof(LightingConstants), m_lightCBO);
	BindConstantBuffer(CAMERA_LIGHTING_BUFFER_SLOT, m_lightCBO);

	float scrHeight = static_cast<float>(m_config.m_window->GetClientDimensions().y);
	float scrWidth = static_cast<float>(m_config.m_window->GetClientDimensions().x);
	AABB2 vwport = camera.GetViewport();
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = vwport.m_mins.x * scrWidth;
	viewport.TopLeftY = (1.0f - vwport.m_maxs.y) * scrHeight;
	viewport.Width = (vwport.m_maxs.x - vwport.m_mins.x) * scrWidth;
	viewport.Height = (vwport.m_maxs.y - vwport.m_mins.y) * scrHeight;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	m_deviceContext->RSSetViewports(1, &viewport);

	m_deviceContext->OMSetRenderTargets(1, &m_renderTargetView, m_depthStencilView);
};

void Renderer::EndCamera(const Camera& camera)
{
	UNUSED( camera );
}

void Renderer::Shutdown()
{
	DebugRenderSystemShutdown();

	for (Texture* pTexture : m_loadedTextures)
	{
		delete pTexture;
	}

	for (Shader* pShader : m_loadedShaders)
	{
		delete pShader;
	}

	DX_SAFE_RELEASE(m_device);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_rasterizerState);
	DX_SAFE_RELEASE(m_blendState);
	DX_SAFE_RELEASE(m_samplerState);
	DX_SAFE_RELEASE(m_depthStencilTexture);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_depthStencilState);

	if (m_immediateVBO_PCU)
	{
		delete m_immediateVBO_PCU;
		m_immediateVBO_PCU = nullptr;
	}
	if (m_immediateVBO_PNCU)
	{
		delete m_immediateVBO_PNCU;
		m_immediateVBO_PNCU = nullptr;
	}
	if (m_cameraCBO)
	{
		delete m_cameraCBO;
		m_cameraCBO = nullptr;
	}
	if (m_modelCBO)
	{
		delete m_modelCBO;
		m_modelCBO = nullptr;
	}
	if (m_lightCBO)
	{
		delete m_lightCBO;
		m_lightCBO = nullptr;
	}

#if defined ENGINE_DEBUG_RENDER
	HRESULT hr = ((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));
	if (FAILED(hr))
	{
		ERROR_AND_DIE("ReportLiveObjects failed");
	}
	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;
	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
};

void Renderer::CreateRenderingContext()
{
	// Creates an OpenGL rendering context (RC) and binds it to the current window's device context (DC)
	PIXELFORMATDESCRIPTOR pixelFormatDescriptor;
	memset(&pixelFormatDescriptor, 0, sizeof(pixelFormatDescriptor));
	pixelFormatDescriptor.nSize = sizeof(pixelFormatDescriptor);
	pixelFormatDescriptor.nVersion = 1;
	pixelFormatDescriptor.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pixelFormatDescriptor.iPixelType = PFD_TYPE_RGBA;
	pixelFormatDescriptor.cColorBits = 24;
	pixelFormatDescriptor.cDepthBits = 24;
	pixelFormatDescriptor.cAccumBits = 0;
	pixelFormatDescriptor.cStencilBits = 8;

	HWND windowHandle = HWND(m_config.m_window->GetOSWindowHandle());
	HDC displayDeviceContext = ::GetDC(windowHandle);
	int pixelFormatCode = ChoosePixelFormat(displayDeviceContext, &pixelFormatDescriptor);
	SetPixelFormat(displayDeviceContext, pixelFormatCode, &pixelFormatDescriptor);
	//wglMakeCurrent(displayDeviceContext, wglCreateContext(displayDeviceContext));
}

void Renderer::SetDebugName(ID3D11DeviceChild* object, char const* name)
{
#if defined ENGINE_DEBUG_RENDER
	object->SetPrivateData(WKPDID_D3DDebugObjectName, (UINT)strlen(name), name);
#else
	UNUSED(object);
	UNUSED(name);
#endif
}

Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	Texture* texture = new Texture(image.GetDimensions(), image.GetImageFilePath());
	
	D3D11_TEXTURE2D_DESC textureDesc = {0};
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subresourceData = {0};
	subresourceData.pSysMem = image.GetRawData();
	subresourceData.SysMemPitch = image.GetDimensions().x * sizeof(Rgba8);

	ID3D11Texture2D* tempTexture = nullptr;
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &subresourceData, &tempTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTexture2D() returned error #%x", hr));
	}
	texture->m_texture = tempTexture;
	SetDebugName(tempTexture, image.GetImageFilePath().c_str());

	hr = m_device->CreateShaderResourceView(texture->m_texture, NULL, &texture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView() returned error #%x", hr));
	}

	m_loadedTextures.push_back(texture);
	return texture;
}

void Renderer::ClearScreen(const Rgba8& clearColor)
{
	float colorAsFloats[4];
	clearColor.GetAsFloats(colorAsFloats);
	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);
	ClearDepth();
};

void Renderer::DrawVertexArray(int numVertices, const Vertex_PCU* vertices)
{
	CopyCPUToGPU(vertices, static_cast<size_t>(numVertices) * m_immediateVBO_PCU->GetStride(), m_immediateVBO_PCU);
	DrawVertexBuffer(m_immediateVBO_PCU, numVertices, 0);
};

void Renderer::DrawVertexArray(std::vector<Vertex_PCU> const& vertices)
{
	DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
}

void Renderer::DrawVertexArray(int numVertices, Vertex_PNCU const* vertices)
{
	CopyCPUToGPU(vertices, static_cast<size_t>(numVertices) * m_immediateVBO_PNCU->GetStride(), m_immediateVBO_PNCU);
	DrawVertexBuffer(m_immediateVBO_PNCU, numVertices, 0);
}

void Renderer::DrawVertexArray(std::vector<Vertex_PNCU> const& vertices)
{
	DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
}

Texture* Renderer::GetTextureForFileName(char const* imageFilePath)
{
	for (Texture* pTexture : m_loadedTextures)
	{
		if (!pTexture->m_name.compare(imageFilePath))
		{
			return pTexture;
		}
	}
	return nullptr;
}

//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureForFileName(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	Image image(imageFilePath);
	Texture* newTexture = CreateTextureFromImage(image);

	return newTexture;
}


//------------------------------------------------------------------------------------------------
Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* texture = new Texture(dimensions, name);

	D3D11_TEXTURE2D_DESC textureDesc = { 0 };
	textureDesc.Width = dimensions.x;
	textureDesc.Height = dimensions.y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = texelData;
	subresourceData.SysMemPitch = dimensions.x * sizeof(Rgba8);

	ID3D11Texture2D* tempTexture = nullptr;
	HRESULT hr = m_device->CreateTexture2D(&textureDesc, &subresourceData, &tempTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTexture2D() returned error #%x", hr));
	}
	texture->m_texture = tempTexture;
	SetDebugName(tempTexture, name);

	hr = m_device->CreateShaderResourceView(texture->m_texture, NULL, &texture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView() returned error #%x", hr));
	}

	m_loadedTextures.push_back(texture);
	return texture;
}

//-----------------------------------------------------------------------------------------------
BitmapFont* Renderer::GetFontForFileName(std::string const& fontFilePath)
{
	for (BitmapFont* pBitmapFont : m_loadedFonts)
	{
		if (!pBitmapFont->m_fontFilePathNameWithNoExtension.compare(fontFilePath))
		{
			return pBitmapFont;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------------------
BitmapFont* Renderer::CreateFontFromFile(std::string const& fontFilePath)
{
	std::string fullFileName = fontFilePath; 
	fullFileName += ".png";
	Texture* fontTexture = CreateOrGetTextureFromFile(fullFileName.c_str());
	BitmapFont* newFont = new BitmapFont(fontFilePath, *fontTexture);
	m_loadedFonts.push_back(newFont);
	return newFont;
}

///
// Shader functions
//-----------------------------------------------------------------------------------------------
Shader* Renderer::CreateOrGetShader(const char* shaderName)
{
	// See if we already have this texture previously loaded
	Shader* existingShader = GetShaderForName(shaderName);
	if (existingShader)
	{
		return existingShader;
	}

	// Never seen this shader before!  Let's create it.
	Shader* newShader = CreateShader(shaderName);
	return newShader;
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindShaderByName(const char* shaderName)
{
	Shader* shader = CreateOrGetShader(shaderName);
	BindShader(shader);
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindShader(Shader const* shader)
{
	if (shader)
	{
		m_currentShader = shader;
	}
	else
	{
		m_currentShader = m_defaultShader; // bind the default shader if the shader is nullptr
	}
	m_deviceContext->VSSetShader(m_currentShader->m_vertexShader, 0, 0);
	m_deviceContext->PSSetShader(m_currentShader->m_pixelShader, 0, 0);
}

//-----------------------------------------------------------------------------------------------
Shader* Renderer::GetShaderForName(const char* shaderName)
{
	for (Shader* pShader : m_loadedShaders)
	{
		if (!pShader->GetName().compare(shaderName))
		{
			return pShader;
		}
	}
	return nullptr;
}

//-----------------------------------------------------------------------------------------------
Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource)
{
	ID3D11VertexShader* vertexShader = nullptr;
	ID3D11PixelShader* pixelShader = nullptr;
	ID3D11InputLayout* inputLayout = nullptr;

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	D3D11_INPUT_ELEMENT_DESC inputElementLitDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	std::vector<unsigned char> vertexCode;
	std::vector<unsigned char> pixelCode;

	// vertex shader
	CompileShaderToByteCode(vertexCode, shaderName, shaderSource, "VertexMain", "vs_5_0");
	HRESULT hr = m_device->CreateVertexShader(vertexCode.data(), vertexCode.size(), nullptr, &vertexShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Create VertexShader returned error #%x", hr));
	}

	// Hack to distinguish which input layout to use
	if (strstr(shaderName, "lit") || strstr(shaderName, "Lit"))
	{
		hr = m_device->CreateInputLayout(inputElementLitDesc, 4, vertexCode.data(), vertexCode.size(), &inputLayout);
	}
	else
	{
		hr = m_device->CreateInputLayout(inputElementDesc, 3, vertexCode.data(), vertexCode.size(), &inputLayout);
	}
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateInputLayout() returned error #%x", hr));
	}

	// pixel shader
	CompileShaderToByteCode(pixelCode, shaderName, shaderSource, "PixelMain", "ps_5_0");
	hr = m_device->CreatePixelShader(pixelCode.data(), pixelCode.size(), nullptr, &pixelShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Create PixelShader returned error #%x", hr));
	}

	ShaderConfig config;
	config.m_name = shaderName;
	Shader* shader = new Shader(config);
	shader->m_vertexShader = vertexShader;
	shader->m_pixelShader = pixelShader;
	shader->m_inputLayout = inputLayout;
	
	// save created shader and return pointer to it
	m_loadedShaders.push_back(shader);
	return shader;
}

Shader* Renderer::CreateShader(const char* shaderName)
{
	std::string filename = shaderName;
	filename += ".hlsl";
	std::string outString;
	FileReadToString(outString, filename);
	return CreateShader(shaderName, outString.c_str());
}

//-----------------------------------------------------------------------------------------------
bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	UINT shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined ENGINE_DEBUG_RENDER
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	// assumes name is the correct parameter
	HRESULT hr = D3DCompile(source, strlen(source), name, nullptr, nullptr, entryPoint, target, shaderFlags, 0, &shaderBlob, &errorBlob);
	if (!SUCCEEDED(hr))
	{
		// we would want to release error blob, but error and die prevents it
		if (errorBlob)
		{
			ERROR_AND_DIE(Stringf("D3DCompile() returned error %s", static_cast<const char*>(errorBlob->GetBufferPointer())));
		}
		else
		{
			ERROR_AND_DIE(Stringf("D3DCompile() returned error"));
		}
	}
	else
	{
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(outByteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
		shaderBlob->Release();
		return true;
	}
}

IndexBuffer* Renderer::CreateIndexBuffer(unsigned int *data, const size_t size)
{
	UNUSED(data);
	ID3D11Buffer* buffer = nullptr;

	D3D11_BUFFER_DESC bufferDescription = { 0 };
	bufferDescription.Usage = D3D11_USAGE_DYNAMIC; // D3D11_USAGE_DYNAMIC; D3D11_USAGE_DEFAULT
	bufferDescription.ByteWidth = static_cast<UINT>(size);
	bufferDescription.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	// Do this somewhere????
	D3D11_SUBRESOURCE_DATA subresourceData = { 0 };
	subresourceData.pSysMem = 0; // &data;
	subresourceData.SysMemPitch = 0; // sizeof(int); //??????
	subresourceData.SysMemSlicePitch = 0;

	HRESULT hr = m_device->CreateBuffer(&bufferDescription, NULL, &buffer);
	if (!SUCCEEDED(hr))
	{
// 		ErrorIn
// 		GetErrorInfo(0, )
		ERROR_AND_DIE(Stringf("CreateBuffer() returned error #%x", hr));
	}

	IndexBuffer* iBuffer = new IndexBuffer(size);
	iBuffer->m_buffer = buffer;
	return iBuffer;
}

//-----------------------------------------------------------------------------------------------
// create vertex buffer
VertexBuffer* Renderer::CreateVertexBuffer(const size_t size, unsigned int stride )
{
	ID3D11Buffer* buffer = nullptr;
	D3D11_BUFFER_DESC bufferDescription = { 0 };
	bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescription.ByteWidth = static_cast<UINT>(size); // sizeof(vertices);
	bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer(&bufferDescription, nullptr, &buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateBuffer() returned error #%x", hr));
	}

	VertexBuffer* vBuffer = new VertexBuffer(size, stride);
	vBuffer->m_buffer = buffer;
	return vBuffer;
}

void Renderer::SetSunDirection(const Vec3& direction)
{
	m_sunDirection = direction.GetEulerAngles();
}

void Renderer::SetSunIntensity(float intensity)
{
	m_sunIntensity = intensity;
}

void Renderer::SetAmbientIntensity(float intensity)
{
	m_ambientIntensity = intensity;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo)
{
	if (ibo->m_size < size)
	{
		// reallocate buffer
		DX_SAFE_RELEASE(ibo->m_buffer);
		D3D11_BUFFER_DESC bufferDescription = { 0 };
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.ByteWidth = static_cast<UINT>(size); // sizeof(vertices);
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = m_device->CreateBuffer(&bufferDescription, nullptr, &(ibo->m_buffer));
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("reallocating buffer returned error #%x", hr));
		}
	}
	D3D11_MAPPED_SUBRESOURCE subresource = { 0 };
	HRESULT hr = m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Indexed Map() returned error #%x", hr));
	}

	memcpy(subresource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

//-----------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo)
{
	if (vbo->m_size < size)
	{
		// reallocate buffer
		DX_SAFE_RELEASE(vbo->m_buffer);
		D3D11_BUFFER_DESC bufferDescription = { 0 };
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.ByteWidth = static_cast<UINT>(size); // sizeof(vertices);
		bufferDescription.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = m_device->CreateBuffer(&bufferDescription, nullptr, &(vbo->m_buffer));
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("reallocating buffer returned error #%x", hr));
		}
	}
	D3D11_MAPPED_SUBRESOURCE subresource = { 0 };
	HRESULT hr = m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Map() returned error #%x", hr));
	}

	memcpy(subresource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindVertexBuffer(VertexBuffer* vbo)
{
	UINT strides = vbo->GetStride();
	UINT offsets = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &strides, &offsets);
 	m_deviceContext->IASetInputLayout(m_currentShader->m_inputLayout);
//	m_deviceContext->IASetInputLayout(vbo->m_inputLayout);
	m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::DrawIndexedVertexBuffer(IndexBuffer* ibo, VertexBuffer* vbo, int indexCount, int indexOffset, int vertexOffset)
{
	// Model Constants
	ModelConstants modelConsts;
	modelConsts.ModelMatrix = m_modelMatrix;
	m_modelColor.GetAsFloats(modelConsts.ModelColor);

	CopyCPUToGPU(&modelConsts, sizeof(ModelConstants), m_modelCBO);
	BindConstantBuffer(CAMERA_MODEL_BUFFER_SLOT, m_modelCBO);

	// Set the buffers
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, 0);
	BindVertexBuffer(vbo);
	m_deviceContext->DrawIndexed(indexCount, indexOffset, vertexOffset);
}

//-----------------------------------------------------------------------------------------------
void Renderer::DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset /*= 0 */)
{
	// Model Constants
	ModelConstants modelConsts;
	modelConsts.ModelMatrix = m_modelMatrix;
	m_modelColor.GetAsFloats(modelConsts.ModelColor);

	CopyCPUToGPU(&modelConsts, sizeof(ModelConstants), m_modelCBO);
	BindConstantBuffer(CAMERA_MODEL_BUFFER_SLOT, m_modelCBO);

	BindVertexBuffer(vbo);
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

//-----------------------------------------------------------------------------------------------
// create constant buffer
ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	ID3D11Buffer* buffer = nullptr;
	D3D11_BUFFER_DESC bufferDescription = { 0 };
	bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
	bufferDescription.ByteWidth = static_cast<UINT>(size); // sizeof(vertices);
	bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	HRESULT hr = m_device->CreateBuffer(&bufferDescription, nullptr, &buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateBuffer() returned error #%x", hr));
	}

	ConstantBuffer* cBuffer = new ConstantBuffer(size);
	cBuffer->m_buffer = buffer;
	return cBuffer;
}

//-----------------------------------------------------------------------------------------------
void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo)
{
	if (cbo->m_size < size)
	{
		// reallocate buffer
		DX_SAFE_RELEASE(cbo->m_buffer);
		D3D11_BUFFER_DESC bufferDescription = { 0 };
		bufferDescription.Usage = D3D11_USAGE_DYNAMIC;
		bufferDescription.ByteWidth = static_cast<UINT>(size); // sizeof(vertices);
		bufferDescription.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDescription.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		HRESULT hr = m_device->CreateBuffer(&bufferDescription, nullptr, &cbo->m_buffer);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE(Stringf("reallocating returned error #%x", hr));
		}
	}
	D3D11_MAPPED_SUBRESOURCE subresource = { 0 };
	HRESULT hr = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Map() returned error #%x", hr));
	}

	memcpy(subresource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::ChangeSunDirection(float deltaPitch)
{
	m_sunDirection.m_pitchDegrees = Clamp(m_sunDirection.m_pitchDegrees + deltaPitch, 0.0f, 180.0f);
}

void Renderer::ChangeSunIntensity(float intensity)
{
	m_sunIntensity = ClampZeroToOne(m_sunIntensity + intensity);
}

void Renderer::ChangeAmbientIntensity(float intensity)
{
	m_ambientIntensity = ClampZeroToOne(m_ambientIntensity + intensity);
}

void Renderer::SetModelMatrix(Mat44 modelMatrix)
{
	m_modelMatrix = modelMatrix;
}

void Renderer::SetModelColor(Rgba8 modelColor)
{
	m_modelColor = modelColor;
}

void Renderer::SetRasterizerState(CullMode cullMode, FillMode fillMode, WindingOrder windingOrder)
{
	D3D11_RASTERIZER_DESC rasterizerDescription = {};
	rasterizerDescription.FillMode = static_cast<D3D11_FILL_MODE>(fillMode);
	rasterizerDescription.CullMode = static_cast<D3D11_CULL_MODE>(cullMode);
	rasterizerDescription.DepthClipEnable = true;
	rasterizerDescription.AntialiasedLineEnable = true;
	rasterizerDescription.FrontCounterClockwise = (windingOrder == WindingOrder::COUNTERCLOCKWISE);

	DX_SAFE_RELEASE(m_rasterizerState);
	HRESULT hr = m_device->CreateRasterizerState(&rasterizerDescription, &m_rasterizerState);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Map() returned error #%x", hr));
	}

	m_deviceContext->RSSetState(m_rasterizerState);
}

void Renderer::ClearDepth(float value /*= 1.0f*/)
{
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, value, 0);
}

void Renderer::SetDepthStencilState(DepthTest depthTest, bool writeDepth)
{
	D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
	switch (depthTest)
	{
	case DepthTest::NEVER:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NEVER;
		break;
	case DepthTest::LESS:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS;
		break;
	case DepthTest::EQUAL:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_EQUAL;
		break;
	case DepthTest::LESS_EQUAL:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_LESS_EQUAL;
		break;
	case DepthTest::GREATER:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER;
		break;
	case DepthTest::NOT_EQUAL:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_NOT_EQUAL;
		break;
	case DepthTest::GREATER_EQUAL:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_GREATER_EQUAL;
		break;
	case DepthTest::ALWAYS:
		depthFunc = D3D11_COMPARISON_FUNC::D3D11_COMPARISON_ALWAYS;
		break;
	}
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc = { };
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = (writeDepth ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO);
	depthStencilDesc.DepthFunc = depthFunc;

	DX_SAFE_RELEASE(m_depthStencilState);
	m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStencilState);

	m_deviceContext->OMSetDepthStencilState(m_depthStencilState, 0);
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	D3D11_FILTER filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
	D3D11_TEXTURE_ADDRESS_MODE mode = D3D11_TEXTURE_ADDRESS_CLAMP;
	switch (samplerMode)
	{
	case SamplerMode::POINTCLAMP:
		filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		mode = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case SamplerMode::POINTWRAP:
		filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
		mode = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	case SamplerMode::BILINEARCLAMP:
		filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		mode = D3D11_TEXTURE_ADDRESS_CLAMP;
		break;
	case SamplerMode::BILINEARWRAP:
		filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		mode = D3D11_TEXTURE_ADDRESS_WRAP;
		break;
	}

	DX_SAFE_RELEASE(m_samplerState);
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = filter;
	samplerDesc.AddressU = mode;
	samplerDesc.AddressV = mode;
	samplerDesc.AddressW = mode;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerState);
}

//-----------------------------------------------------------------------------------------------
void Renderer::BindTexture(const Texture* texture)
{
	if (texture)
	{
		m_deviceContext->PSSetShaderResources(0, 1, &texture->m_shaderResourceView);
	}
	else
	{
		m_deviceContext->PSSetShaderResources(0, 1, &m_defaultTexture->m_shaderResourceView);
	}
}

BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	std::string baseFileName = bitmapFontFilePathWithNoExtension;

	// See if we already have this texture previously loaded
	BitmapFont* existingFont = GetFontForFileName(baseFileName);
	if (existingFont)
	{
		return existingFont;
	}

	// Never seen this texture before!  Let's load it.
	BitmapFont* newFont = CreateFontFromFile(baseFileName);
	return newFont;
}

Shader* Renderer::CreateOrGetShaderFromFile(char const* shaderFilePath)
{
	// call the private version of this function
	return CreateOrGetShader(shaderFilePath);
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	DX_SAFE_RELEASE(m_blendState);
	D3D11_BLEND_DESC blendDescription = { 0 };

	blendDescription.RenderTarget[0] = {0};
	blendDescription.RenderTarget[0].BlendEnable = true;
	blendDescription.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
	blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

	if (blendMode == BlendMode::ALPHA)
	{
		blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	}
	else if (blendMode == BlendMode::ADDITIVE)
	{
		blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
		blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	}
	else if (blendMode == BlendMode::OPAQUE)
	{
		blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
		blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
	}
	else
	{
		ERROR_AND_DIE(Stringf("Unknown / unsupported blend mode #%i", blendMode));
	}
	
	m_device->CreateBlendState(&blendDescription, &m_blendState);

	float blendFactor[4] = {0};
	UINT sampleMask = 0xffffffff;
	m_deviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
}

