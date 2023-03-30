#pragma once
#include "Engine/Math/Vec2.hpp"
#include "Game/GameCommon.hpp"

class Game;
class Wormhole;

class Entity
{
public:
	Game* m_game = nullptr;
	Wormhole* m_wormhole = nullptr;
	Vec2 m_wormholeVelocity = {};
	Vec2 m_savedVelocity = {};
	double m_savedTime = 0.0;
	float m_scalingFactor = 1.0f;
	bool m_shrinking = false;

//protected:
public:
	Vec2 m_position = {};
	Vec2 m_velocity = {};
	float m_orientationDegrees = 0.0f;
	float m_angularVelocity = 0.0f;
	float m_physicalRadius = 0.0f;
	float m_cosmeticRadius = 0.0f;
	int m_health = SHIP_HEALTH;
	bool m_isDead = false;
	bool m_isGarbage = false;

public:
	Entity(Game * game, Vec2 startPos);
	virtual void Update(float deltaSeconds);
	virtual void Render() const;
	virtual void DebugRender() const;
	virtual void Die();
	bool IsAlive();
	bool IsOffscreen();
	virtual void SpawnOffscreen();
	Vec2 const GetForwardNormal() const;
	void BounceOffWalls();
	Vec2 SpawnOnBorder();
	void SetupTeleportation(Wormhole*& worm);
	void TeleportThroughWormhole(float deltaSeconds);
};