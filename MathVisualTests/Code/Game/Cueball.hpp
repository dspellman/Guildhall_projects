#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Math/OBB2.hpp"
#include "Engine/Math/Capsule2.hpp"

class Cueball
{
public:
	Vec2 m_position = Vec2::ZERO;
	Vec2 m_velocity = Vec2::ZERO;
	float m_radius = 0.0f;
	float m_elasticity = 0.9f;
	Rgba8 m_color = Rgba8::WHITE;
	bool m_gravity = false;

	static bool s_bottomless;
	static Vec2 s_gravity;

	~Cueball() {}
	Cueball(Vec2 position, Vec2 velocity, float radius, bool gravity, float elasticity = 0.9f, Rgba8 color = Rgba8::WHITE);
	void UpdateBall(float deltaSeconds);
	void AddVerts(std::vector<Vertex_PCU>& drawVerts) const;
	void BounceAgainstWalls();
	void BounceOffWalls();
	void BounceOffBumperDisc(Vec2 const position, float radius, float elasticity);
	void BounceOffBumperOBB(OBB2 const& obb, float elasticity);
	void BounceOffBumperCapsule(Capsule2 const& capsule, float elasticity);
	void BounceOffBall(Cueball& B);
};