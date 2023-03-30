#pragma once
#include "Game/Entity.hpp"

struct Vec2;
struct Rgba8;
struct Vertex_PCU;
class Wormhole;

class Debris :
    public Entity
{
public:
	Vertex_PCU vertex[DEBRIS_VERTS] = {};

	Debris(Game* game, Vec2 startPos, Vec2 velocity, float orientationDegrees, float scale, Rgba8 color);
	Debris(Game* game, Vec2 startPos, Vec2 velocity, float orientationDegrees, float scale, Wormhole* wormhole);
	void Update(float deltaSeconds);
	void Render() const;
private:
	double m_startTime = 0.0;
	void DisappearIntoTheVoid();
	Rgba8 m_color = {};
};

