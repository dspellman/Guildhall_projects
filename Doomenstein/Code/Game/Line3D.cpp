#include "Line3D.hpp"
#include "Engine/Math/AABB3.hpp"

Line3D::Line3D(Vec3 const& position, EulerAngles orientation)
	: m_position(position), m_orientation(orientation)
{
// 	m_scalingFactor = 0.3f;
// 	m_physicalRadius = 0.2f;
// 	m_cosmeticRadius = 0.25f;
//	Test();
}

void Line3D::Update(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	m_orientation += m_angularVelocity * deltaSeconds;

// 	double time = GetCurrentTimeSeconds();
// 	float sine = static_cast<float>(sin(time / Pi()));
// 	unsigned char tint = static_cast<unsigned char>(127.9f * sine + 128.0f);
// 	m_modelColor = Rgba8(tint, tint, 255, 255);
}

void Line3D::Render() const
{
	g_theRenderer->SetModelMatrix(GetModelMatrix());
	g_theRenderer->SetModelColor(m_modelColor);
	g_theRenderer->BindTexture(m_texture);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
	g_theRenderer->DrawVertexArray(static_cast<int>(m_vertices.size()), m_vertices.data());
}

void Line3D::DebugRender() const
{

}

Mat44 Line3D::GetModelMatrix() const
{
	Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(m_position);
	trans.Append(orient);
	return trans;
}

void Line3D::Create(float length, Rgba8 color, float width, int edges)
{
// 	Rgba8 color = Rgba8::WHITE;
	Vec2 uvs = Vec2(0.0f, 0.0f);
// 	float width = 0.05f;
// 	float length = 50.0f;
// 	int edges = 8;
	float incDeg = 360.0f / static_cast<float>(edges);

	float cAngDeg = 0.0f;
	float nAngDeg = incDeg;
	float c0 = CosDegrees(cAngDeg);
	float s0 = SinDegrees(cAngDeg);
	float c1 = CosDegrees(nAngDeg);
	float s1 = SinDegrees(nAngDeg);

	for (int index = 0; index < edges; index++)
	{
		m_vertices.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, -length), Rgba8::WHITE, Vec2(0.0f, 0.0f)));
		m_vertices.push_back(Vertex_PCU(Vec3(c0 * width, s0 * width, length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c1 * width, s1 * width, length), color, uvs));

		m_vertices.push_back(Vertex_PCU(Vec3(0.0f, 0.0f, -length), Rgba8::WHITE, Vec2(0.0f, 0.0f)));
		m_vertices.push_back(Vertex_PCU(Vec3(c1 * width, s1 * width, -length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c0 * width, s0 * width, -length), color, uvs));

		m_vertices.push_back(Vertex_PCU(Vec3(c0 * width, s0 * width, length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c0 * width, s0 * width, -length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c1 * width, s1 * width, -length), color, uvs));

		m_vertices.push_back(Vertex_PCU(Vec3(c0 * width, s0 * width, length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c1 * width, s1 * width, -length), color, uvs));
		m_vertices.push_back(Vertex_PCU(Vec3(c1 * width, s1 * width, length), color, uvs));

		cAngDeg = nAngDeg; // unused
		c0 = c1;
		s0 = s1;
		nAngDeg += incDeg;
		c1 = CosDegrees(nAngDeg);
		s1 = SinDegrees(nAngDeg);
	}
}

void Line3D::SetTexture(Texture* texture)
{
	m_texture = texture;
}
