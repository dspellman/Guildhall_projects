// Class handling all rendering capability for engine library
#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include <string>
#include <vector>
#include "Engine/Renderer/shader.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"
#include "IndexBuffer.hpp"

#define DX_SAFE_RELEASE(dxObject)			\
{											\
	if (( dxObject) != nullptr)				\
	{										\
		(dxObject)->Release();				\
		(dxObject) = nullptr;				\
	}										\
}

#undef OPAQUE

class Window;
class Texture;
class Image;
class BitmapFont;

struct ID3D11Device;
struct ID3D11DeviceContext;
struct IDXGISwapChain;
struct ID3D11RenderTargetView;
struct ID3D11RasterizerState;
struct ID3D11BlendState;
struct ID3D11SamplerState;

enum class SamplerMode
{
	POINTCLAMP,
	POINTWRAP,
	BILINEARCLAMP,
	BILINEARWRAP,
};

enum class DepthTest
{
	NEVER = 1,
	LESS = 2,
	EQUAL = 3,
	LESS_EQUAL = 4,
	GREATER = 5,
	NOT_EQUAL = 6,
	GREATER_EQUAL = 7,
	ALWAYS = 8
};

enum class CullMode
{
	NONE = D3D11_CULL_NONE,
	FRONT = D3D11_CULL_FRONT,
	BACK = D3D11_CULL_BACK
};

enum class FillMode
{
	SOLID = D3D11_FILL_SOLID,
	WIREFRAME = D3D11_FILL_WIREFRAME
};

enum class WindingOrder
{
	CLOCKWISE,
	COUNTERCLOCKWISE
};

enum class BlendMode
{
	ALPHA,
	ADDITIVE,
	OPAQUE
};

struct RendererConfig
{
	Window* m_window = nullptr;
};

class Renderer
{
public:
	Renderer( RendererConfig config );
	~Renderer();

	//void Startup(HDC displayDeviceContext);
	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void CreateRenderingContext();
	void SetDebugName(ID3D11DeviceChild* object, char const* name);
	Texture* CreateTextureFromImage(const Image& image);

	void ClearScreen(const Rgba8& clearColor);
	void BeginCamera(const Camera& camera);
	void EndCamera(const Camera& camera);
	void DrawVertexArray(int numVertices, const Vertex_PCU* vertices);
	void DrawVertexArray(std::vector<Vertex_PCU> const& vertices);
	void DrawVertexArray(int numVertices, Vertex_PNCU const* vertices);
	void DrawVertexArray(std::vector<Vertex_PNCU> const& vertices);

	void BindTexture(const Texture* texture);
	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);
	BitmapFont* CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension);
	Shader* CreateOrGetShaderFromFile(char const* shaderFilePath);
	void SetBlendMode( BlendMode blendMode );

	void SetModelMatrix(Mat44 modelMatrix);
	void SetModelColor(Rgba8 modelColor);
	void SetRasterizerState( CullMode cullMode, FillMode fillMode, WindingOrder windingOrder );
	void ClearDepth(float value = 1.0f);
	void SetDepthStencilState(DepthTest depthTest, bool writeDepth);
	void SetSamplerMode( SamplerMode samplerMode );

	IndexBuffer* CreateIndexBuffer(unsigned int* data, const size_t size);
	void DrawIndexedVertexBuffer(IndexBuffer* ibo, VertexBuffer* vbo, int indexCount, int indexOffset = 0, int vertexOffset = 0);
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer* vbo);
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer* ibo);
	VertexBuffer* CreateVertexBuffer(const size_t size, unsigned int stride );

	void SetSunDirection(const Vec3& direction);
	void SetSunIntensity(float intensity);
	void SetAmbientIntensity(float intensity);
	void ChangeSunDirection(float param1);
	void ChangeSunIntensity(float intensity);
	void ChangeAmbientIntensity(float intensity);

	Texture* GetTextureForFileName( char const* imageFilePath );
	Texture* CreateTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);

	BitmapFont* GetFontForFileName(std::string const& fontFilePath);
	BitmapFont* CreateFontFromFile(std::string const& fontFilePath);

	Shader* CreateOrGetShader(const char* shaderName);
	void BindShaderByName(const char* shaderName);
	void BindShader(Shader const* shader);
	Shader* GetShaderForName(const char* shaderName);
	Shader* CreateShader(const char* shaderName);
	Shader* CreateShader(char const* shaderName, char const* shaderSource);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);
	
	void BindVertexBuffer(VertexBuffer* vbo);
	void DrawVertexBuffer(VertexBuffer* vbo, int vertexCount, int vertexOffset = 0);
	ConstantBuffer* CreateConstantBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo);
	void BindConstantBuffer(int slot, ConstantBuffer* cbo);

public:
	EulerAngles	m_sunDirection = EulerAngles(0.0f, 135.0f, 0.0f);
	float	m_sunIntensity = 0.5f;
	float	m_ambientIntensity = 0.5f;

private:
	RendererConfig m_config;
	unsigned int m_flags = 0;
	std::vector< Texture* > m_loadedTextures;
	std::vector< BitmapFont* > m_loadedFonts;
	std::vector< Shader* > m_loadedShaders;
	Shader const* m_currentShader = nullptr;
	VertexBuffer* m_immediateVBO_PCU = nullptr;
	VertexBuffer* m_immediateVBO_PNCU = nullptr;

	// TBD
	void* m_dxgiDebugModule = nullptr;
	void* m_dxgiDebug = nullptr;
	Mat44 m_modelMatrix;
	Rgba8 m_modelColor = Rgba8::WHITE;

protected:
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	ID3D11Device* m_device = nullptr;
	ID3D11DeviceContext* m_deviceContext = nullptr;
	IDXGISwapChain* m_swapChain = nullptr;
	ID3D11RenderTargetView* m_renderTargetView = nullptr;
	ID3D11RasterizerState* m_rasterizerState = nullptr;
	Shader* m_defaultShader = nullptr;
	ID3D11BlendState*	m_blendState = nullptr;
	Texture* m_defaultTexture = nullptr;
	ID3D11SamplerState*	 m_samplerState = nullptr;
	ID3D11DepthStencilState* m_depthStencilState = nullptr;
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
};
