#pragma once
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"
#include "VertexBuffer.hpp"
#include "IndexBuffer.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Math/IntVec3.hpp"
#include "Texture.hpp"
#include <mutex>
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Engine/Core/EventSystem.hpp"

struct MeshConfig
{
	Renderer* theRenderer = nullptr;
};

struct MeshImportOptions
{
	Mat44 modelMatrix;
	float scale = 1.0f;
	bool reverse = false;
	bool invert = false;
	std::string textureImagePath;
	bool test = false;
};

class Mesh
{
public:
	~Mesh();
	Mesh(MeshConfig& meshConfig);
	bool ImportFromOBJFile( char const* filename, MeshImportOptions const& options);
	Vertex_PNCU MakePNCUVertex(IntVec3 indexes);
	void CreateBuffers();
	bool SaveCustom(char const* filename);
	bool LoadCustom( char const* filename, char const* texturename);
	void ApplyTransform( Mat44 const& transform);
	void ReverseWindingOrder();
	bool ConvertModelToRender();
	void Render() const;
	Mat44 GetModelMatrix() const;

	void Startup();
	void Shutdown();
	void BeginFrame();
	void EndFrame();
public:
	// model
	std::vector<Vec3> m_positions;
	std::vector<Vec3> m_normals;
	std::vector<Vec3> m_textures;
	std::vector<Vertex_PNCU> m_vertices;
	std::vector<uint32_t> m_indices;
	int m_indexCount = 0;
	int m_vertexCount = 0;
	Mat44 m_modelMatrix;
	bool m_reverse = false;
	bool m_invert = false;
	Rgba8 m_modelColor = Rgba8::WHITE;
	Texture* m_texture = nullptr;
	bool m_valid = false; // is the loaded model usable?

	// render
	VertexBuffer* m_vertexBuffer = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
//	INputLayout const* m_layout;
	bool m_indexed = true;
//	int m_elementCount;

	// processing
	double m_loadTime = 0.0;
};

static bool Event_Import(EventArgs& args);
static bool Event_Load(EventArgs& args);
static bool Event_Save(EventArgs& args);