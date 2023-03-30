#pragma once
#include "Game/Entity.hpp"

class Wasp :
    public Entity
{
public:
	Vertex_PCU vertex[WASP_VERTS] = {};

	Wasp(Game* game, Vec2 startPos);
	void Update(float deltaSeconds);
	void Render() const;

};

