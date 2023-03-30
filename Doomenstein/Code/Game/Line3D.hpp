#pragma once
#include "Game/GameCommon.hpp"
#include <vector>
#include "Engine/Core/Vertex_PCU.hpp"

class Line3D
{
public:
	Vec3 m_position;
	EulerAngles m_orientation;
	EulerAngles m_angularVelocity;
	Rgba8 m_modelColor = Rgba8::WHITE;

public:
	~Line3D() {};
	Line3D(Vec3 const& position, EulerAngles orientation);
	void Update(float deltaSeconds);
	void Render() const;
	void DebugRender() const;
	void Create(float length, Rgba8 color = Rgba8::WHITE, float width = 0.03f, int edges = 4);
	void SetTexture(Texture* texture);
	Mat44 GetModelMatrix() const;

protected:
	std::vector<Vertex_PCU> m_vertices;
	Texture* m_texture = nullptr;
	Rgba8 m_vertexColor = Rgba8::WHITE;
};