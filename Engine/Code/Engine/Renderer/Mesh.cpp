#include "Engine/Renderer/Mesh.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"

// We can compile the Mesh out of the engine
#include "Game/EngineBuildPreferences.hpp"
#if !defined( ENGINE_DISABLE_MESH )

Mesh::~Mesh()
{
	if (m_vertexBuffer)
	{
		delete m_vertexBuffer;
		m_vertexBuffer = nullptr;
	}
	if (m_indexBuffer)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}
}

Mesh::Mesh(MeshConfig& meshConfig)
{
	UNUSED(meshConfig);
}

bool Mesh::ImportFromOBJFile(char const* filename, MeshImportOptions const& options)
{
	m_loadTime = GetCurrentTimeSeconds();
	Mat44 modelMatrix = options.modelMatrix;
	m_reverse = options.reverse;
	m_invert = options.invert;
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(options.textureImagePath.c_str());

	std::string outBuffer;
	std::string name(filename);
	int result = FileReadToString(outBuffer, name);
	if (result)
	{
		m_valid = false;
		ERROR_RECOVERABLE("File read error");
	}
	// file read, clear old data and parse new data
	m_indices.clear();
	m_vertices.clear();

	Strings lines = SplitLinesOnCRLFCombo(outBuffer); // split into lines of text based on newline character
	Strings fields;
	int index = 0;
	Vec3 vert;
	for (index = 0; index < lines.size(); index++)
	{
		char value = lines[index].c_str()[0];
		if (value == '#' || value == 13)
		{
			continue; // skip any comment or empty line
		}

		if (value == 'v')
		{
			switch (lines[index].c_str()[1])
			{
			case ' ':
				fields = SplitStringOnDelimiter(lines[index].c_str(), ' ');
				if (fields.size() < 4)
				{
					m_valid = false;
					ERROR_RECOVERABLE("size?");
				}
				// ignoring optional w value since we wouldn't use it anyway
				sscanf_s(lines[index].c_str(), "v %f %f %f", &vert.x, &vert.y, &vert.z);
				m_positions.push_back(vert); // store consecutive points in array
				break;
			case 'n':
				fields = SplitStringOnDelimiter(lines[index].c_str(), ' ');
				if (fields.size() < 4)
				{
					m_valid = false;
					ERROR_RECOVERABLE("size?");
				}
				sscanf_s(lines[index].c_str(), "vn %f %f %f", &vert.x, &vert.y, &vert.z);
				m_normals.push_back(vert); // store consecutive normals in array
				break;
			case 't':
				fields = SplitStringOnDelimiter(lines[index].c_str(), ' ');
				if (fields.size() == 3)
				{
					sscanf_s(lines[index].c_str(), "vt %f %f", &vert.x, &vert.y);
					vert.z = 0.0f;
				}
				if (fields.size() == 4)
				{
					sscanf_s(lines[index].c_str(), "vt %f %f %f", &vert.x, &vert.y, &vert.z);
				}
				// invert v here if requested
				if (m_invert)
				{
					vert.y *= -1.0f;
				}
				m_textures.push_back(vert); // store consecutive normals in array
				break;
			default:
				m_valid = false;
				ERROR_RECOVERABLE("new case to handle");
				break;
			}
		}

		if (value == 'f')
		{
			// extraneous space on the end of these lines causes empty field
			fields = SplitStringOnDelimiter(lines[index].c_str(), ' ');
			int count = 0;
			Strings indexes;
			IntVec3 triad[4];
			while (count < (fields.size() - 1) && fields[count + 1].size())
			{
				indexes = SplitStringOnDelimiter(fields[count + 1], '/');
				triad[count] = IntVec3::ZERO; // set default sentinel in case no value read
				switch (indexes.size())
				{
				case 3:
					sscanf_s(indexes[2].c_str(), "%i", &triad[count].z);
				case 2:
					sscanf_s(indexes[1].c_str(), "%i", &triad[count].y);
				case 1:
					sscanf_s(indexes[0].c_str(), "%i", &triad[count].x);
				}
				count++; // increment count after using it for index to get number
			} 
			int vertexBase = int(m_vertices.size());
			if (count == 3)
			{
				// save the triangle vertices
				for (int three = 0; three < 3; three++)
				{
					m_vertices.push_back(MakePNCUVertex(triad[three]));
				}
				// set indexes by winding order
				if (m_reverse)
				{
					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 2);
					m_indices.push_back(vertexBase + 1);
				}
				else
				{
					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 1);
					m_indices.push_back(vertexBase + 2);
				}
			}
			else if (count == 4)
			{
				// save the quad vertices
				for (int four = 0; four < 4; four++)
				{
					m_vertices.push_back(MakePNCUVertex(triad[four]));
				}
				// split quad into two triangles by indexes in winding order
				if (m_reverse)
				{
					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 2);
					m_indices.push_back(vertexBase + 1);

					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 3);
					m_indices.push_back(vertexBase + 2);
				}
				else
				{
					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 1);
					m_indices.push_back(vertexBase + 2);

					m_indices.push_back(vertexBase + 0);
					m_indices.push_back(vertexBase + 2);
					m_indices.push_back(vertexBase + 3);
				}
			}
			else
			{
				m_valid = false;
				ERROR_RECOVERABLE("Bad face count");
			}
		}
	}
	for (int vertex = 0; vertex < m_vertices.size(); vertex++)
	{
		m_vertices[vertex].m_position = modelMatrix.TransformVectorQuantity3D(m_vertices[vertex].m_position);
	}
	m_indexCount = int(m_indices.size());
	m_vertexCount = int(m_vertices.size());
	CreateBuffers(); // do this now since we have the complete data
	m_loadTime = GetCurrentTimeSeconds() - m_loadTime;
	char msg[80];
	sprintf_s(msg, "Verts %i indexes %i time %f", m_vertexCount, m_indexCount, m_loadTime);
	std::string m_importText(msg);
	if (!options.test)
	{
		g_theConsole->AddLine(Rgba8::PURPLE, m_importText);
	}
	m_valid = true;
	return true;
}

Vertex_PNCU Mesh::MakePNCUVertex(IntVec3 indexes)
{
	// negative indices would be subtracted from the array size (end offset)
	Vertex_PNCU pncu(Vec3::ZERO, Vec3::ZERO, Rgba8::WHITE, Vec2::ZERO);
	if (indexes.x)
	{
		pncu.m_position = m_positions[indexes.x - 1];
	}
	else
	{
		ERROR_RECOVERABLE("Zero vertex?");
	}
	if (indexes.y)
	{
		pncu.m_uvTexCoords.x = m_textures[indexes.y - 1].x;
		pncu.m_uvTexCoords.y = m_textures[indexes.y - 1].y;
	}
	if (indexes.z)
	{
		pncu.m_normal = m_normals[indexes.z - 1];
	}
	return pncu;
}

void Mesh::CreateBuffers()
{
	if (m_indexBuffer)
	{
		delete m_indexBuffer;
	}
	if (m_vertexBuffer)
	{
		delete m_vertexBuffer;
	}
	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indices.data(), m_indexCount * sizeof(unsigned int)); // multiply by  to get total count?
	m_vertexBuffer = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));

	g_theRenderer->CopyCPUToGPU(m_vertices.data(), static_cast<size_t>(m_vertices.size()) * m_vertexBuffer->GetStride(), m_vertexBuffer);
	g_theRenderer->CopyCPUToGPU(m_indices.data(), static_cast<size_t>(m_indices.size()) * m_indexBuffer->GetStride(), m_indexBuffer);
}

bool Mesh::SaveCustom(char const* filename)
{
	int size = sizeof(int) * (m_indexCount + 2) + sizeof(Vertex_PNCU) * m_vertexCount;
	std::vector<uint8_t> outBuffer;
	outBuffer.resize(size);
	std::memcpy(outBuffer.data(), &m_indexCount, sizeof(m_indexCount));
	std::memcpy(outBuffer.data() + 4, &m_vertexCount, sizeof(m_vertexCount));
	std::memcpy(outBuffer.data() + 8, m_indices.data(), m_indexCount * sizeof(int));
	std::memcpy(outBuffer.data() + sizeof(int) * (m_indexCount + 2), m_vertices.data(), m_vertexCount * sizeof(Vertex_PNCU));
	if (FileWriteBinaryBuffer(outBuffer, filename))
	{
		return false; // write error
	}
	return true;
}

bool Mesh::LoadCustom(char const* filename, char const* texturename)
{
	MeshImportOptions options;
	options.textureImagePath = "Data/Images/TestUV.png";
	options.test = true;
	ImportFromOBJFile("Data/Meshes/test.obj", options);

	m_loadTime = GetCurrentTimeSeconds();
	std::vector<uint8_t> outBuffer;
	if (FileReadToBuffer(outBuffer, filename))
	{
		return false; // read error
	}
	m_indices.clear();
	m_vertices.clear();
	std::memcpy(&m_indexCount, outBuffer.data(), sizeof(m_indexCount));
	std::memcpy(&m_vertexCount, outBuffer.data() + 4, sizeof(m_vertexCount));
	for (int index = 0; index < m_indexCount; index++)
	{
		m_indices.push_back(*((int*)(outBuffer.data() + (index + 2) * sizeof(int))));
	}
	for (int index = 0; index < m_vertexCount; index++)
	{
		m_vertices.push_back(*((Vertex_PNCU*)(outBuffer.data() + (m_indexCount + 2) * sizeof(int) + (index) * sizeof(Vertex_PNCU))));
	}
	CreateBuffers(); // do this now since we have the complete data
	m_loadTime = GetCurrentTimeSeconds() - m_loadTime;
	char msg[80];
	sprintf_s(msg, "Verts %i indexes %i time %f", m_vertexCount, m_indexCount, m_loadTime);
	std::string m_importText(msg);
	g_theConsole->AddLine(Rgba8::PURPLE, m_importText);
	m_texture = g_theRenderer->CreateOrGetTextureFromFile(texturename);
	return true;
}

void Mesh::ApplyTransform(Mat44 const& transform)
{
	UNUSED(transform);
}

void Mesh::ReverseWindingOrder()
{
	m_reverse = true;
}

bool Mesh::ConvertModelToRender()
{
	return false;
}

void Mesh::Render() const
{
	if (!m_valid)
	{
		return; // no valid model to render
	}
	g_theRenderer->BindShaderByName("Data/Shaders/SpriteLit");
	g_theRenderer->SetModelMatrix(GetModelMatrix());
	g_theRenderer->SetModelColor(m_modelColor);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);

//	g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
//	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	g_theRenderer->DrawIndexedVertexBuffer(m_indexBuffer, m_vertexBuffer, m_indexCount);
//	g_theRenderer->DrawVertexArray((int)m_vertices.size(), m_vertices.data());
	g_theRenderer->BindShader(nullptr);
}

Mat44 Mesh::GetModelMatrix() const
{
	return m_modelMatrix;
}

void Mesh::Startup()
{
//	m_accessMutex.lock();
	// set up events
	g_theEventSystem->SubscribeEventCallbackFunction("import", Event_Import);
	g_theEventSystem->SubscribeEventCallbackFunction("load", Event_Load);
	g_theEventSystem->SubscribeEventCallbackFunction("save", Event_Save);
	//	m_accessMutex.unlock();

	//	m_modelMatrix = EulerAngles(-90.0f, 0.0f, -90.0f).GetAsMatrix_XFwd_YLeft_ZUp();
	//	m_texture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Meshes/SciFi_Fighter-MK6-diffuse.jpg");
//	m_modelMatrix = EulerAngles(90.0f, 0.0f, 90.0f).GetAsMatrix_XFwd_YLeft_ZUp();
//	m_texture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Meshes/miku_base.png");
}

void Mesh::Shutdown()
{
//	m_accessMutex.lock();
//	m_accessMutex.unlock();
}

void Mesh::BeginFrame()
{

}

void Mesh::EndFrame()
{

}

//======================================================================================================
// static functions
bool Event_Import(EventArgs& args)
{
	delete g_theMesh;
	MeshConfig config;
	g_theMesh = new Mesh(config);
	// decipher arguments
	std::string path = args.GetValue("p", "");
	if (path == "")
	{
		return false;
	}
	float scale = args.GetValue("s", 1.0f);
	std::string xform = args.GetValue("x", "ijk");

	Vec3 basis[3];
	int offset = 0;
	float sign = 1.0f;
	for (int axis = 0; axis < 3; axis++)
	{
		if (xform[offset] == '-')
		{
			sign = -1.0f;
			offset++;
		}
		else
		{
			sign = 1.0f;
		}
		switch (xform[offset])
		{
		case 'i':
			basis[axis] = Vec3(sign * scale, 0.0f, 0.0f);
			break;
		case 'j':
			basis[axis] = Vec3(0.0f, sign * scale, 0.0f);
			break;
		case 'k':
			basis[axis] = Vec3(0.0f, 0.0f, sign * scale);
			break;
		default:
			ERROR_RECOVERABLE("bad xfrom value");
			break;
		}
		offset++;
	}

	MeshImportOptions options;
	options.modelMatrix = Mat44(basis[0], basis[1], basis[2], Vec3::ZERO);
	options.reverse = args.GetValue("r", false);
	options.invert = args.GetValue("i", false);
	options.textureImagePath = args.GetValue("t", "Data/Images/TestUV.png");
	g_theMesh->ImportFromOBJFile(path.c_str(), options);
	return true;
}

bool Event_Load(EventArgs& args)
{
	std::string path = args.GetValue("p", "");
	if (path == "")
	{
		return false;
	}
	std::string texture = args.GetValue("t", "");
	if (texture == "")
	{
		texture = "Data/Images/TestUV.png";
	}
	return g_theMesh->LoadCustom(path.c_str(), texture.c_str());
}

bool Event_Save(EventArgs& args)
{
	std::string path = args.GetValue("p", "");
	if (path == "")
	{
		return false;
	}
	return g_theMesh->SaveCustom(path.c_str());
}

#endif // !defined( ENGINE_DISABLE_MESH )