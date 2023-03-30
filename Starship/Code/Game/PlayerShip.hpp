#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Game/Entity.hpp"
#include "Engine/Audio/AudioSystem.hpp"

class AnalogJoystick;

class PlayerShip: public Entity
{
public:
	int m_extraLives = PLAYER_LIVES;
	float m_leftThrust = 0.0f;
	float m_rightThrust = 0.0f;
	float m_forwardThrust = 0.0f;
	SoundID m_thrustSoundID = 0;
	SoundPlaybackID m_thrustSoundPlaybackID = 0;
	bool thrustNoisePlaying = false;

	Vertex_PCU vertex[SHIP_VERTS] = {};

	~PlayerShip();
	PlayerShip(Game* game, Vec2 startPos);
	void Update(float deltaSeconds);
	void DoThrustNoise();
	void CheckStopThrustNoise(AnalogJoystick joystick);
	void Render() const;
	void AttractRender(Vec2 position, float orientationDegrees, float scale) const;
	void LifeRender(Vec2 position, float orientationDegrees, float scale) const;
	void Die();

	void Reset();
private:
	double thrustStartTime = 0.0;
};
