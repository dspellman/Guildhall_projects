#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Entity.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Game.hpp"
#include "Game/App.hpp"
#include <math.h>
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Game/Wormhole.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Audio/AudioSystem.hpp"

extern AudioSystem* g_theAudio;
extern RandomNumberGenerator random; // singleton for now only used by entities in the Game

Rgba8 shipColor(0, 255, 255, 255);
Rgba8 flameColor(255, 0, 0, 127);
Rgba8 tailColor(255, 255, 0, 0);

PlayerShip::~PlayerShip()
{
}

PlayerShip::PlayerShip(Game* game, Vec2 startPos) : Entity(game, startPos)
{
	Reset(); // just to be safe

	m_physicalRadius = PLAYER_SHIP_PHYSICS_RADIUS;
	m_cosmeticRadius = PLAYER_SHIP_COSMETIC_RADIUS;

	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(2.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[2] = Vertex_PCU(Vec3(0.0f, 2.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(0.0f, -2.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(2.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	// upper back triangle
	vertex[6] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[7] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[8] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	// lower back triangle
	vertex[9] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[10] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[11] = Vertex_PCU(Vec3(0.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	// middle triangle
	vertex[12] = Vertex_PCU(Vec3(0.0f, 1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[13] = Vertex_PCU(Vec3(0.0f, -1.0f, 0.0f), shipColor, Vec2(0.f, 0.f));
	vertex[14] = Vertex_PCU(Vec3(1.0f, 0.0f, 0.0f), shipColor, Vec2(0.f, 0.f));

	// forward thruster triangle
	vertex[15] = Vertex_PCU(Vec3(-5.0f, 0.0f, 0.0f), tailColor, Vec2(0.f, 0.f));
	vertex[16] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
	vertex[17] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
	// right thruster triangle
	vertex[18] = Vertex_PCU(Vec3(-4.0f, -1.0f, 0.0f), tailColor, Vec2(0.f, 0.f));
	vertex[19] = Vertex_PCU(Vec3(-2.0f, -1.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
	vertex[20] = Vertex_PCU(Vec3(-2.0f, 0.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
	// left thruster triangle
	vertex[21] = Vertex_PCU(Vec3(-4.0f, 1.0f, 0.0f), tailColor, Vec2(0.f, 0.f));
	vertex[22] = Vertex_PCU(Vec3(-2.0f, 0.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
	vertex[23] = Vertex_PCU(Vec3(-2.0f, 1.0f, 0.0f), flameColor, Vec2(0.f, 0.f));
}

void PlayerShip::Update(float deltaSeconds)
{
	// check for NaN problem
	if (isnan(m_position.x) || isnan(m_position.y))
	{
		m_position.x = WORLD_CENTER_X;
		m_position.y = WORLD_CENTER_Y;
	}

	XboxController const& controller = g_theInput->GetController(0); // a leap of faith

	if (m_shrinking && m_wormhole && m_wormhole->m_velocity != m_wormholeVelocity)
	{
		m_velocity -= m_wormholeVelocity; // remove old velocity component and add new velocity component
		m_velocity += m_wormhole->m_velocity;
		m_wormholeVelocity = m_wormhole->m_velocity;
	}
	TeleportThroughWormhole(deltaSeconds);

	m_position += deltaSeconds * m_velocity;

	BounceOffWalls();

	if (m_isDead)
	{
		if (m_extraLives > 0 && (g_theInput->WasKeyJustPressed('N') || controller.WasButtonJustPressed(XboxButtonID::START)))
		{
			Reset();
			m_extraLives -= 1; // subtract a life on re-spawn, test end of game at death
			SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Scifi_Heal_Cloak08.wav");
			g_theAudio->StartSound(soundID);
		}
		else
			return; // No input allowed for dead ship
	}

	if (m_shrinking)
		return; // no ship control allowed if being sucked through a wormhole

	// reset  parameters for thrust animation
	m_leftThrust = 0.0f;
	m_rightThrust = 0.0f;
	m_forwardThrust = 0.0f;

	// holding both S and F will keep the ship from turning, which makes sense with competing thrusters
	if (g_theInput->IsKeyDown('S'))
	{
		m_orientationDegrees += PLAYER_SHIP_TURN_SPEED * deltaSeconds;
		m_rightThrust = 1.0f;
		DoThrustNoise();
	}

	if (g_theInput->IsKeyDown('F'))
	{
		m_orientationDegrees -= PLAYER_SHIP_TURN_SPEED * deltaSeconds;
		m_leftThrust = 1.0f;
		DoThrustNoise();
	}

	AnalogJoystick leftJoystick = controller.GetLeftStick();
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		// set thrust animation parameters if accelerating or turning
		m_forwardThrust += leftJoystick.GetMagnitude();
		if (GetShortestAngularDispDegrees(m_orientationDegrees, leftJoystick.GetOrientationDegrees()) < 0.0f)
		{
			m_leftThrust = 1.0f;
		}
		if (GetShortestAngularDispDegrees(m_orientationDegrees, leftJoystick.GetOrientationDegrees()) > 0.0f)
		{
			m_rightThrust = 1.0f;
		}

		m_orientationDegrees = leftJoystick.GetOrientationDegrees(); // joystick will snap orientation
		Vec2 forwardNormal = GetForwardNormal();
		m_velocity.x += forwardNormal.x * leftJoystick.GetMagnitude() * PLAYER_SHIP_ACCELERATION * deltaSeconds;
		m_velocity.y += forwardNormal.y * leftJoystick.GetMagnitude() * PLAYER_SHIP_ACCELERATION * deltaSeconds;
		m_velocity.ClampLength(PLAYER_SHIP_MAX_SPEED);
		DoThrustNoise();
	}

	if (g_theInput->IsKeyDown('E'))
	{
		if (deltaSeconds != 0.0f)
		{
			Vec2 forwardNormal = GetForwardNormal();
			m_velocity.x += forwardNormal.x * PLAYER_SHIP_ACCELERATION * deltaSeconds;
			m_velocity.y += forwardNormal.y * PLAYER_SHIP_ACCELERATION * deltaSeconds;
			m_velocity.ClampLength(PLAYER_SHIP_MAX_SPEED);
			// add thrust for animation
			m_forwardThrust += 1.0f;
			DoThrustNoise();
		}
	}

	CheckStopThrustNoise(leftJoystick);

	if (IsAlive() && (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || controller.WasButtonJustPressed(XboxButtonID::A)))
	{
		m_game->FireBullet(m_position + GetForwardNormal() * 1.0f, m_orientationDegrees); // this naming convention follows the "action verb" standard
		SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Laser_Impact01.wav");
		g_theAudio->StartSound(soundID);
	}
}

void PlayerShip::DoThrustNoise()
{
	if (g_theAudio->IsPlaying(m_thrustSoundPlaybackID))
		return;
	m_thrustSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Rocket_Missile_Loop04.wav");
	m_thrustSoundPlaybackID = g_theAudio->StartSound(m_thrustSoundID);
	g_theAudio->SetSoundPlaybackVolume(m_thrustSoundPlaybackID, 0.3f);
}

void PlayerShip::CheckStopThrustNoise(AnalogJoystick joystick)
{
	if (joystick.GetMagnitude() == 0.0f && !g_theInput->IsKeyDown('E') && !g_theInput->IsKeyDown('S') && !g_theInput->IsKeyDown('F'))
	{
		g_theAudio->StopSound(m_thrustSoundPlaybackID);
	}
}

void PlayerShip::Render() const
{
	Vertex_PCU tvertex[SHIP_VERTS];
	for (int vertIndex = 0; vertIndex < SHIP_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
	}
	// do thrust animation
	tvertex[15].m_position.x = THRUST_BASE_OFFSET - 3.0f * m_forwardThrust * random.RollRandomFloatInRange(THRUST_RANGE_MIN, THRUST_RANGE_MAX);
	tvertex[18].m_position.x = THRUST_BASE_OFFSET - 2.0f * m_rightThrust * random.RollRandomFloatInRange(THRUST_RANGE_MIN, THRUST_RANGE_MAX);
	tvertex[21].m_position.x = THRUST_BASE_OFFSET - 2.0f * m_leftThrust * random.RollRandomFloatInRange(THRUST_RANGE_MIN, THRUST_RANGE_MAX);

	TransformVertexArrayXY3D( SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position );
	g_theRenderer->DrawVertexArray(SHIP_VERTS, tvertex);
}

void PlayerShip::AttractRender(Vec2 position, float orientationDegrees, float scale) const
{
	UNUSED( orientationDegrees );

	Vertex_PCU tvertex[SHIP_VERTS];
	for (int vertIndex = 0; vertIndex < SHIP_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
		tvertex[vertIndex].m_color.a /= 2; // make transparent
	}
	// use sinusoidal behavior for spice
	double time = GetCurrentTimeSeconds();
	float sine = static_cast<float>(sin(time));
	float cosine = static_cast<float>(cos(time));
	// do static thrust animation
	tvertex[15].m_position.x = THRUST_BASE_OFFSET - 3.0f * random.RollRandomFloatInRange(THRUST_RANGE_MIN, THRUST_RANGE_MAX);
	tvertex[18].m_position.x = THRUST_BASE_OFFSET - 2.0f * fabsf(cosine) * random.RollRandomFloatInRange(THRUST_RANGE_MIN, THRUST_RANGE_MAX);
 	tvertex[21].m_position.x = THRUST_BASE_OFFSET - 2.0f * random.RollRandomFloatInRange(0.0f, 0.5f);
	unsigned char timeA = static_cast<unsigned char>(64.0f + 191.0f * (sine + 1.0f) * 0.5f);
	for (int vert = 0; vert < SHIP_VERTS; vert++)
		tvertex[vert].m_color.a = timeA;
	position.x += 20.0f * cosine * scale;
	position.y += 8.0f * sine * scale;
	float orthoAngleDegrees = Atan2Degrees(sine, cosine) + 90.0f;

	TransformVertexArrayXY3D(SHIP_VERTS, tvertex, scale * ((sine + 1.0f) * 0.25f + 0.75f), orthoAngleDegrees, position);
	g_theRenderer->DrawVertexArray(SHIP_VERTS, tvertex);
}

void PlayerShip::LifeRender(Vec2 position, float orientationDegrees, float scale) const
{
	Vertex_PCU tvertex[SHIP_VERTS];
	for (int vertIndex = 0; vertIndex < SHIP_VERTS; vertIndex++)
	{
		tvertex[vertIndex] = vertex[vertIndex];
		tvertex[vertIndex].m_color.a /= 2; // make transparent
	}

	TransformVertexArrayXY3D(SHIP_VERTS, tvertex, scale, orientationDegrees, position);
	g_theRenderer->DrawVertexArray(SHIP_VERTS - 9, tvertex);
}

void PlayerShip::Die()
{
	m_game->shakeFraction = WORLD_SCREEN_SHAKE_FRACTION;
	SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Explo_Large_11.wav");
	g_theAudio->StartSound(soundID);
	soundID = g_theAudio->CreateOrGetSound("Data/Audio/Explo_Small_12.wav");
	g_theAudio->StartSound(soundID);
	m_isDead = true; // mark dead so that it does not render
	if (m_extraLives == 0)
	{
		SoundID deathSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Laser_Shot14.wav");
		g_theAudio->StartSound(deathSoundID);
		m_game->m_gameState = DEFEAT; // test game over at last death
	}
}

void PlayerShip::Reset()
{
	m_position = Vec2(WORLD_CENTER_X, WORLD_CENTER_Y);
	m_velocity = Vec2(0.0f, 0.0f);
	m_orientationDegrees = 0.0f;
	m_angularVelocity = 0.0f;
	m_health = SHIP_HEALTH;
	m_isDead = false;
	m_isGarbage = false;
}
