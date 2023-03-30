#pragma once
#include "Game/Entity.hpp"

class Beetle :
    public Entity
{
public:
	Vertex_PCU vertex[BEETLE_VERTS] = {};

	Beetle(Game* game, Vec2 startPos);
	void Update(float deltaSeconds);
	void Render() const;

	void AttractRender(Vec2 position, float orientationDegrees, float scale) const;
};

