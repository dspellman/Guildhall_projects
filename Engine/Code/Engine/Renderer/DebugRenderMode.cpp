#include "DebugRenderMode.hpp"
#include "../Core/EngineCommon.hpp"
#include "../Core/DevConsole.hpp"
#include "BitmapFont.hpp"
#include <mutex>

std::recursive_mutex debugMutex;

// declarations /////////////////////////////////////////////////////////////////////////
constexpr float PRINT_POSITION = 50.0f;

class Raycast;
class ScreenText;
class WorldText;
class Cylinder;
class Cone;
class Cube;
class Sphere;

// variables /////////////////////////////////////////////////////////////////////////////
bool m_hidden = false;
bool m_raycastVisible = true;
Renderer* renderer = nullptr;
Camera m_currentCamera;
//std::vector<Vertex_PCU> geometryVerts;
std::vector<ScreenText*> screenText;
std::vector<ScreenText*> messageText;
std::vector<Cylinder*> cylinders;
std::vector<Cone*> cones;
std::vector<Cube*> cubes;
std::vector<Sphere*> spheres;
std::vector<WorldText*> worldText;
Clock* debugClock = nullptr;
std::vector<Raycast*> raycasts;

// Template functions for vector management ///////////////////////////////////////////////
template < typename T > void clean(std::vector<T*>& objVector)
{
	for (int index = 0; index < objVector.size(); index++)
	{
		if (objVector[index] && objVector[index]->m_inactive)
		{
			delete objVector[index];
			objVector[index] = nullptr;
		}
	}
}

template < typename T > void show(std::vector<T*> const& objVector)
{
	for (int index = 0; index < objVector.size(); index++)
	{
		if (objVector[index])
		{
			objVector[index]->Render();
		}
	}
}

template < typename T > void showText(std::vector<T*> const& objVector)
{
	for (int index = 0; index < objVector.size(); index++)
	{
		if (objVector[index])
		{
			objVector[index]->Render(Vec2::ZERO);
		}
	}
}

// ray casting omni class /////////////////////////////////////////
class Raycast 
{
public:
	RaycastResult3D m_rayCast;
	float m_duration = 10.0f;
	float m_length = 10.0f;
	double m_startTime = 0.0f;
	bool m_inactive = false;

	~Raycast()
	{

	}

	Raycast(RaycastResult3D rayCast, float duration, float length)
		: m_rayCast(rayCast), m_duration(duration), m_length(length)
	{
		m_startTime = debugClock->GetTotalTime();
	}

	void Render()
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}

		Vec3 lineEnd = m_rayCast.m_rayStartPos + m_rayCast.m_rayFwdNormal * m_rayCast.m_rayMaxLength;
		DebugAddWorldLine(m_rayCast.m_rayStartPos, lineEnd, 0.01f, 0.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::XRAY);
		if (m_rayCast.m_didImpact)
		{
			DebugAddWorldPoint(m_rayCast.m_impactPos, 0.06f, 0.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);
			Vec3 arrowEnd = m_rayCast.m_impactPos + m_rayCast.m_impactNormal * 0.3f;
			DebugAddWorldArrow(m_rayCast.m_impactPos, arrowEnd, 0.03f, 0.0f, Rgba8::BLUE, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);
		}
	}
};

// ScreenText classes ///////////////////////////////////////////////////////////////////////
class WorldText
{
public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	bool m_inactive = false;
	std::string m_text;
	Mat44 m_transform;
	float m_duration;
	Vec2 m_alignment;
	float m_height;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	float m_fontAspect = 1.0f;
	mutable BitmapFont* m_font;
	double m_startTime = 0.0;
	DebugRenderMode m_mode = DebugRenderMode::USEDEPTH;
	bool m_billboard = false;

public:
	WorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
		: m_text(text), m_transform(transform), m_duration(duration), m_alignment(alignment), m_height(textHeight), m_startColor(startColor), m_endColor(endColor), m_mode(mode)
	{
		m_font = renderer->CreateOrGetBitmapFont("Data/Fonts/MyFixedFont");
		m_startTime = debugClock->GetTotalTime();
	}

	void Render()
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}
		t = Clamp(t, 0.0f, 1.0f);
		if (m_duration == -1.0f)
		{
			t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
		}

		// set rendering parameters
		renderer->SetSamplerMode(SamplerMode::POINTCLAMP);
		renderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		renderer->SetBlendMode(BlendMode::ALPHA);
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));

		std::vector<Vertex_PCU> textVerts;
		AABB2 lineBox;
		float width = m_font->GetTextWidth(m_height, m_text, m_fontAspect);
		lineBox.SetDimensions(Vec2(width, m_height)); // centered text for billboard version
		if (!m_billboard)
		{
			lineBox.SetCenter(Vec2(width * 0.5f, m_height * 0.5f)); // 0,0 based print pivot line
		}
		//		lineBox.Translate(m_position);

//		m_font->AddVertsForTextInBox2D(textVerts, lineBox, m_height, m_text, Rgba8::ColorLerp(m_startColor, m_endColor, t), m_fontAspect, m_alignment, SHRINK);
		m_font->AddVertsForTextInBox2D(textVerts, lineBox, m_height, m_text, Rgba8::WHITE, m_fontAspect, m_alignment, SHRINK);

		renderer->BindTexture(&m_font->GetTexture());
		//	renderer.SetBlendMode(BlendMode::ALPHA);
		if (textVerts.size())
		{
			renderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
		}
	}

	Mat44 GetModelMatrix() const
	{
		if (!m_billboard)
		{
			return m_transform;
		}

		// build matrix to aim text at player (camera)
		Mat44 xform;
		Mat44 FaceXaxis( Vec3( 0.0f, 1.0f, 0.0f ), Vec3( 0.0f, 0.0f, 1.0f ), Vec3( 1.0f, 0.0f, 0.0f ), Vec3::ZERO );
		Mat44 camera = m_currentCamera.GetOrientation().GetAsMatrix_XFwd_YLeft_ZUp();
		Mat44 rotation(-camera.GetIBasis3D(), -camera.GetJBasis3D(), camera.GetKBasis3D(), Vec3::ZERO);
		xform.Append(m_transform);
		xform.Append(rotation);
		xform.Append(FaceXaxis);
		return xform;
	}
};

// Cube class ////////////////////////////////////////////////////////////////////////
class Sphere
{
public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	std::vector<Vertex_PCU> m_vertices;
	Texture* m_texture = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	DebugRenderMode m_mode = DebugRenderMode::USEDEPTH;
	bool m_wireFrame = false;
	float m_duration = -1.0f;
	bool m_inactive = false;
	double m_startTime = 0.0;

	Sphere(Vec3 const& position, EulerAngles orientation)
		: m_position(position), m_orientation(orientation)
	{
		m_startTime = debugClock->GetTotalTime();
	}

	Mat44 GetModelMatrix() const
	{
		Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		orient.Orthonormalize_XFwd_YLeft_ZUp();
		Mat44 trans;
		trans.SetTranslation3D(m_position);
		trans.Append(orient);
		return trans;
	}

	void Render()
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}
		t = Clamp(t, 0.0f, 1.0f);
		if (m_duration == -1.0f)
		{
			t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
		}

		if (m_wireFrame)
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
		}
		else
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		}
		if (m_mode == DebugRenderMode::XRAY)
		{
			RenderXray(t);
			return;
		}
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		if (m_mode == DebugRenderMode::ALWAYS)
		{
			renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		}
		else
		{
			renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		}
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void RenderXray(float t) const
	{
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t).dim(0.5f));
		renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void Create(float radius, float duration, Rgba8 startColor = Rgba8::WHITE, Rgba8 endColor = Rgba8::GRAY, DebugRenderMode mode = DebugRenderMode::USEDEPTH)
	{
		m_duration = duration;
		m_mode = mode;
		m_startColor = startColor; // this will be the model color
		m_endColor = endColor;
		AddVertsForSphere(m_vertices, Vec3::ZERO, EulerAngles::ZERO, radius, 16, 8, startColor);
	}

	void SetTexture(Texture* texture)
	{
		m_texture = texture;
	}
};

// Cube class ////////////////////////////////////////////////////////////////////////
class Cube
{
public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	std::vector<Vertex_PCU> m_vertices;
	Texture* m_texture = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	DebugRenderMode m_mode = DebugRenderMode::USEDEPTH;
	bool m_wireFrame = false;
	float m_duration = -1.0f;
	bool m_inactive = false;
	double m_startTime = 0.0;

	Cube(Vec3 const& position, EulerAngles orientation)
		: m_position(position), m_orientation(orientation)
	{
		m_startTime = debugClock->GetTotalTime();
	}

	Mat44 GetModelMatrix() const
	{
		Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		orient.Orthonormalize_XFwd_YLeft_ZUp();
		Mat44 trans;
		trans.SetTranslation3D(m_position);
		trans.Append(orient);
		return trans;
	}

	void Render()
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}
		t = Clamp(t, 0.0f, 1.0f);
		if (m_duration == -1.0f)
		{
			t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
		}

		if (m_wireFrame)
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
		}
		else
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		}
		if (m_mode == DebugRenderMode::XRAY)
		{
			RenderXray(t);
			return;
		}
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		if (m_mode == DebugRenderMode::ALWAYS)
		{
			renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		}
		else
		{
			renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		}
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void RenderXray(float t) const
	{
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t).dim(0.5f));
		renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void Create(const AABB3& bounds, float duration, Rgba8 startColor = Rgba8::WHITE, Rgba8 endColor = Rgba8::GRAY, DebugRenderMode mode = DebugRenderMode::USEDEPTH)
	{
		m_duration = duration;
		m_mode = mode;
		m_startColor = startColor; // this will be the model color
		m_endColor = endColor;

		AddVertsForAABB3D(m_vertices, bounds, startColor);
	}

	void SetTexture(Texture* texture)
	{
		m_texture = texture;
	}
};

// Cylinder class ////////////////////////////////////////////////////////////////////////
class Cylinder
{
public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	std::vector<Vertex_PCU> m_vertices;
	Texture* m_texture = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	DebugRenderMode m_mode = DebugRenderMode::USEDEPTH;
	bool m_wireFrame = false;
	float m_duration = -1.0f;
	bool m_inactive = false;
	double m_startTime = 0.0;

Cylinder(Vec3 const& position, EulerAngles orientation)
	: m_position(position), m_orientation(orientation)
{
	m_startTime = debugClock->GetTotalTime();
}

Mat44 GetModelMatrix() const
{
	Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(m_position);
	trans.Append(orient);
	return trans;
}

void Render()
{
	if (m_inactive)
	{
		return;
	}
	if (m_duration == 0.0f)
	{
		m_inactive = true;
		m_duration = -1.0f;
	}
	float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
	if (m_duration != -1.0f && t > 1.0f)
	{
		m_inactive = true;
		return; // text is faded
	}
	t = Clamp(t, 0.0f, 1.0f);
	if (m_duration == -1.0f)
	{
		t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
	}

	if (m_wireFrame)
	{
		renderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
	}
	else
	{
		renderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	}
	if (m_mode == DebugRenderMode::XRAY)
	{
		RenderXray(t);
		return;
	}
	renderer->SetModelMatrix(GetModelMatrix());
	renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
	renderer->BindTexture(m_texture);
	renderer->SetBlendMode(BlendMode::OPAQUE);
	if (m_mode == DebugRenderMode::ALWAYS)
	{
		renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	}
	else
	{
		renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
	}
	renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
}

void RenderXray(float t) const
{
	renderer->SetModelMatrix(GetModelMatrix());
	renderer->BindTexture(m_texture);
	renderer->SetBlendMode(BlendMode::OPAQUE);
	renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t).dim(0.5f));
	renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	renderer->SetBlendMode(BlendMode::OPAQUE);
	renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
	renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
	renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
}

void Create(float length, Rgba8 startColor = Rgba8::WHITE, Rgba8 endColor = Rgba8::GRAY, float width = 0.05f, int edges = 16, float duration = -1.0f, DebugRenderMode mode = DebugRenderMode::USEDEPTH)
{
	UNUSED(edges);
	m_duration = duration;
	m_mode = mode;
	m_startColor = startColor; // this will be the model color
	m_endColor = endColor; 
	AddVertsForCylinder3D(m_vertices, Vec3::ZERO, EulerAngles::ZERO, length, width);
}

void SetTexture(Texture* texture)
{
	m_texture = texture;
}
};

// Cone class /////////////////////////////////////////////////////////////////////////////
class Cone
{
public:
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	std::vector<Vertex_PCU> m_vertices;
	Texture* m_texture = nullptr;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;
	DebugRenderMode m_mode = DebugRenderMode::USEDEPTH;
	bool m_wireFrame = false;
	float m_duration = -1.0f;
	bool m_inactive = false;
	double m_startTime = 0.0;

	Cone(Vec3 const& position, EulerAngles orientation)
		: m_position(position), m_orientation(orientation)
	{
		m_startTime = debugClock->GetTotalTime();
	}

	Mat44 GetModelMatrix() const
	{
		Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
		orient.Orthonormalize_XFwd_YLeft_ZUp();
		Mat44 trans;
		trans.SetTranslation3D(m_position);
		trans.Append(orient);
		return trans;
	}

	void Render()
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}
		t = Clamp(t, 0.0f, 1.0f);
		if (m_duration == -1.0f)
		{
			t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
		}

		if (m_wireFrame)
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
		}
		else
		{
			renderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		}
		if (m_mode == DebugRenderMode::XRAY)
		{
			RenderXray(t);
			return;
		}
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		if (m_mode == DebugRenderMode::ALWAYS)
		{
			renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		}
		else
		{
			renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		}
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void RenderXray(float t) const
	{
		renderer->SetModelMatrix(GetModelMatrix());
		renderer->BindTexture(m_texture);
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t).dim(0.5f));
		renderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
		renderer->SetBlendMode(BlendMode::OPAQUE);
		renderer->SetModelColor(Rgba8::ColorLerp(m_startColor, m_endColor, t));
		renderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		renderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
	}

	void Create(float length, Rgba8 startColor = Rgba8::WHITE, Rgba8 endColor = Rgba8::GRAY, float width = 0.05f, int edges = 16, float duration = -1.0f, DebugRenderMode mode = DebugRenderMode::USEDEPTH)
	{
		m_duration = duration;
		m_mode = mode;
		m_startColor = startColor; // this will be the model color
		m_endColor = endColor;

		AddVertsForCone(m_vertices, Vec3::ZERO, EulerAngles::ZERO, length, width, startColor, edges);
	}

	void SetTexture(Texture* texture)
	{
		m_texture = texture;
	}
};

// ScreenText classes ///////////////////////////////////////////////////////////////////////
class ScreenText
{
public:
	bool m_inactive = false;
	std::string m_text;
	Vec2 m_position;
	float m_duration;
	Vec2 m_alignment;
	float m_size;
	Rgba8 m_startColor;
	Rgba8 m_endColor;
	float m_fontAspect = 1.0f;
	mutable BitmapFont* m_font;
	double m_startTime = 0.0;

public:
	ScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
		: m_text(text), m_position(position), m_duration(duration), m_alignment(alignment), m_size(size), m_startColor(startColor), m_endColor(endColor)
	{
		m_font = renderer->CreateOrGetBitmapFont("Data/Fonts/MyFixedFont");
		m_startTime = debugClock->GetTotalTime();
	}

	void Render(Vec2 offset = Vec2::ZERO)
	{
		if (m_inactive)
		{
			return;
		}
		if (m_duration == 0.0f)
		{
			m_inactive = true;
			m_duration = -1.0f;
		}
		float t = static_cast<float>(debugClock->GetTotalTime() - m_startTime) / m_duration;
		if (m_duration != -1.0f && t > 1.0f)
		{
			m_inactive = true;
			return; // text is faded
		}
		t = Clamp(t, 0.0f, 1.0f);
		if (m_duration == -1.0f)
		{
			t = 0.0f; // assumes changeless if infinite, but could be 0.5 too
		}

		std::vector<Vertex_PCU> textVerts;
		AABB2 lineBox;
		float width = m_font->GetTextWidth(m_size, m_text, m_fontAspect);
		lineBox.SetDimensions(Vec2(width, m_size));
		lineBox.SetCenter(Vec2(width * 0.5f, m_size * 0.5f));
		lineBox.Translate(m_position);
		lineBox.Translate(offset);

		m_font->AddVertsForTextInBox2D(textVerts, lineBox, m_size, m_text, Rgba8::ColorLerp(m_startColor, m_endColor, t), m_fontAspect, m_alignment, SHRINK);

		renderer->BindTexture(&m_font->GetTexture());
		//	renderer.SetBlendMode(BlendMode::ALPHA);
		if (textVerts.size())
		{
			renderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
		}
	}
};

//functions
void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	debugMutex.lock();
	renderer = config.renderer;
	m_hidden = config.m_startHidden;
	m_hidden = false;
	debugMutex.unlock();
}

void DebugRenderSystemShutdown()
{
	debugMutex.lock();
	DebugRenderClear();
	renderer = nullptr; // don't delete but do flag non-usable
	m_hidden = true;
	debugMutex.unlock();
}

void DebugRenderSetVisible()
{
	debugMutex.lock();
	m_hidden = false;
	debugMutex.unlock();
}

void DebugRenderSetHidden()
{
	debugMutex.lock();
	m_hidden = true;
	debugMutex.unlock();
}

void DebugRenderToggle()
{
	debugMutex.lock();
	m_hidden = !m_hidden;
	debugMutex.unlock();
}

// add all drawing objects to this list
void DebugRenderClear()
{
	destroy<ScreenText>(screenText);

	destroy<Cylinder>(cylinders);

	destroy<Cone>(cones);

	for (Cube* cone : cubes)
	{
		delete cone;
	}
	cubes.clear();	

	destroy<Sphere>(spheres);

	for (WorldText* text : worldText)
	{
		delete text;
	}
	worldText.clear();

	destroy<Raycast>(raycasts);
}

void DebugRenderSetParentClock(Clock& parent)
{
	debugClock = new Clock(parent);
}

void DebugRenderAdjustClockDialation(double increment)
{
	double dialation = debugClock->GetTimeDilation();
	dialation += increment;
	if (dialation < 0.1)
	{
		dialation = 0.1;
	}
	if (dialation > 10.0)
	{
		dialation = 10.0;
	}
	debugClock->SetTimeDilation(dialation);
}

void DebugRaycastToggle()
{
	m_raycastVisible = !m_raycastVisible; // toggle rendering of raycast hits
}

void DebugRaycast(RaycastResult3D rayCast, float duration, float length)
{
	debugMutex.lock();
	Raycast* ray = new Raycast(rayCast, duration, length);
	push<Raycast>(ray, raycasts);
	debugMutex.unlock();
}

void DebugRenderBeginFrame()
{
	debugMutex.lock();
	if (m_hidden)
	{
		debugMutex.unlock();
		return; // no rendering if flagged hidden
	}

	clean<Raycast>(raycasts);
	clean<Cylinder>(cylinders);
	clean<Sphere>(spheres);
	clean<Cone>(cones);
	clean<ScreenText>(screenText);
	debugMutex.unlock();
}

void DebugRenderWorld(const Camera& camera)
{
	debugMutex.lock();
	if (m_hidden)
	{
		debugMutex.unlock();
		return; // no rendering if flagged hidden
	}
	m_currentCamera = camera;

	show<Cylinder>(cylinders);

	show<Cone>(cones);

	for (Cube* cube : cubes)
	{
		cube->Render();
	}

	show<Sphere>(spheres);

	for (WorldText* text : worldText)
	{
		text->Render();
	}

	if (m_raycastVisible)
	{
		show<Raycast>(raycasts);
	}
	debugMutex.unlock();
}

void DebugRenderScreen(const Camera& camera)
{
	debugMutex.lock();
	UNUSED(camera);
	if (m_hidden)
	{
		debugMutex.unlock();
		return; // no rendering if flagged hidden
	}

// 	for (ScreenText* text : screenText)
// 	{
// 		text->Render(Vec2::ZERO);
// 	}
	show<ScreenText>(screenText);

	float printPos = PRINT_POSITION;
	float vertical = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);

	for (ScreenText* text : messageText)
	{
		if (text->m_inactive)
		{
			continue; // faded out
		}
		printPos += text->m_size;
		text->Render(Vec2(0.0f, vertical - printPos));
	}
	debugMutex.unlock();
}

void DebugRenderEndFrame()
{

}

// specific functions to add content for rendering
void DebugAddWorldPoint(const Vec3& pos, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Sphere* sphere = new Sphere(pos, EulerAngles(0.0f, 0.0f, 0.0f));
	sphere->Create(radius, duration, startColor, endColor, mode);
	//sphere->SetTexture(renderer->CreateOrGetTextureFromFile((char const*)"Data/Images/TestUV.png"));
// 	spheres.push_back(sphere);
	push<Sphere>(sphere, spheres);
	debugMutex.unlock();
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Vec3 EtoS(end - start);
	EulerAngles orientation(EtoS);
	orientation.m_pitchDegrees += 90.0f;
	Cylinder* line = new Cylinder(start + EtoS * 0.5f, orientation);
	line->Create(EtoS.GetLength(), startColor, endColor, radius, 16, duration, mode);
// 	cylinders.push_back(line);
	push<Cylinder>(line, cylinders);
	debugMutex.unlock();
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Vec3 EtoS(top - base);
	EulerAngles orientation(EtoS);
	orientation.m_pitchDegrees += 90.0f; // adjust orientation from x-axis to z-axis parallel
	Cylinder* line = new Cylinder(base + EtoS * 0.5f, orientation);
	line->Create(EtoS.GetLength(), startColor, endColor, radius, 16, duration, mode);
	line->m_wireFrame = true;
// 	cylinders.push_back(line);
	push<Cylinder>(line, cylinders);
	debugMutex.unlock();
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Sphere* sphere = new Sphere(center, EulerAngles(0.0f, 0.0f, 0.0f));
	sphere->Create(radius, duration, startColor, endColor, mode);
	sphere->m_wireFrame = true;
// 	spheres.push_back(sphere);
	push<Sphere>(sphere, spheres);
	debugMutex.unlock();
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float radius, float duration, const Rgba8& baseColor, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
	{
	debugMutex.lock();
	UNUSED(baseColor);
	Vec3 EtoS(end - start);
	float ESlength = EtoS.GetLength();
	float ESinverse = 1.0f / ESlength;
	float shaft;
	float arrow;
	if (ESlength < 1.0f)
	{
		shaft = ESlength * 0.5f; // half length allocated to shaft and arrow head
		arrow = ESlength * 0.5f;
	}
	else
	{
		shaft = ESlength - 0.5f; // shaft + 0.5 unit arrow head
		arrow = 0.5f;
	}
	EulerAngles orientation(EtoS);
	orientation.m_pitchDegrees += 90.0f;
	Cylinder* line = new Cylinder(start + EtoS * (shaft * ESinverse * 0.5f), orientation);
	line->Create(shaft, startColor, endColor, radius, 16, duration, mode);
	push<Cylinder>(line, cylinders);

	Cone* iArrow = new Cone(start + EtoS * ((shaft + arrow * 0.5f) * ESinverse), orientation);
	iArrow->Create(arrow, startColor, endColor, 2 * radius, 16, duration, mode);
	push<Cone>(iArrow, cones);
	debugMutex.unlock();
}

// center is doubled from origin?
void DebugAddWorldBox(const AABB3& bounds, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Cube* cube = new Cube(Vec3::ZERO, EulerAngles::ZERO);
	cube->Create(bounds, duration, startColor, endColor, mode);
	cubes.push_back(cube);
	debugMutex.unlock();
}

void DebugAddWorldBasis(const Mat44& basis, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	UNUSED(startColor);
	UNUSED(endColor);
	Vec3 i = basis.GetIBasis3D();
	Vec3 j = basis.GetJBasis3D();
	Vec3 k = basis.GetKBasis3D();
	Vec3 t = basis.GetTranslation3D();

	DebugAddWorldArrow(t, t + i, 0.1f, duration, Rgba8::RED, Rgba8::RED, Rgba8::RED, mode);
	DebugAddWorldArrow(t, t + j, 0.1f, duration, Rgba8::GREEN, Rgba8::GREEN, Rgba8::GREEN, mode);
	DebugAddWorldArrow(t, t + k, 0.1f, duration, Rgba8::BLUE, Rgba8::BLUE, Rgba8::BLUE, mode);
	debugMutex.unlock();
}

// World text functions and class ////////////////////////////////////////////////////////////////////////////////////

// Global Text Functions /////////////////////////////////////////////////////////////
void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	WorldText* world = new WorldText(text, transform, textHeight, alignment, duration, startColor, endColor, mode);
	worldText.push_back(world);
	debugMutex.unlock();
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	debugMutex.lock();
	Mat44 transform;
	transform.AppendTranslation3D(origin);
	WorldText* world = new WorldText(text, transform, textHeight, alignment, duration, startColor, endColor, mode);
	world->m_billboard = true;
	worldText.push_back(world);
	debugMutex.unlock();
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float duration, const Vec2& alignment, float size, const Rgba8& startColor, const Rgba8& endColor)
{
	debugMutex.lock();
// 	screenText.push_back( new ScreenText(  text, position, duration, alignment, size, startColor, endColor ));
	push<ScreenText>(new ScreenText(  text, position, duration, alignment, size, startColor, endColor ), screenText);
	debugMutex.unlock();
}

void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	debugMutex.lock();
	messageText.push_back( new ScreenText(  text, Vec2::ZERO, duration, Vec2(0.0f, 1.0f), 20.0f, startColor, endColor ));
	debugMutex.unlock();
}
