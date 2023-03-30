#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Entity.hpp"

class Wormhole :  public Entity
{
public:
	~Wormhole() {};
	Wormhole(Game* game, Vec2 startPos);

	void Update(float deltaSeconds);
	void Render();

	float m_radius = 0.0f;
	float m_width = 0.0f;
private:
	void SpawnInfallingDebris();
};

