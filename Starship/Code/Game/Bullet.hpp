#pragma once
#include "Game/Entity.hpp"

class Bullet: public Entity
{
public:
	Vertex_PCU vertex[BULLET_VERTS] = {};

	Bullet(Game* game, Vec2 const& position, float forwardDegrees);
	void Update(float deltaSeconds);
	void Render() const;
	void DisappearIntoTheVoid();
private:
	double m_lifeTime;
};