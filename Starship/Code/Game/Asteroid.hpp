#pragma once
#include "Game/Entity.hpp"

class Asteroid: public Entity
{
public:
	Vertex_PCU vertex[ASTEROID_VERTS] = {};

	Asteroid(Game* game, Vec2 startPos);
	void Update(float deltaSeconds);
	void Render() const;
	void DisappearIntoTheVoid();
};