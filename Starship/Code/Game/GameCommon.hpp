#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Core/Rgba8.hpp"

constexpr int NUM_STARTING_ASTEROIDS = 3;
constexpr int NUM_STARTING_BEETLES = 1;
constexpr int NUM_STARTING_WASPS = 1;
constexpr int MAX_ASTEROIDS = 30;
constexpr int MAX_BULLETS = 50;
constexpr int MAX_DEBRIS = 600;
constexpr int MAX_BEETLES = 6;
constexpr int MAX_WASPS = 3;
constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;
constexpr float ASTEROID_SPEED = 10.f;
constexpr float ASTEROID_ANGULAR_VELOCITY_DEGREES = 200.0f;
constexpr float ASTEROID_PHYSICS_RADIUS = 1.6f;
constexpr float ASTEROID_COSMETIC_RADIUS = 2.0f;
constexpr float BULLET_LIFETIME_SECONDS = 4.0f;
constexpr float BULLET_SPEED = 50.f;
constexpr float BULLET_PHYSICS_RADIUS = 0.5f;
constexpr float BULLET_COSMETIC_RADIUS = 2.0f;
constexpr float PLAYER_SHIP_ACCELERATION = 30.f;
constexpr float PLAYER_SHIP_TURN_SPEED = 300.f;
constexpr float PLAYER_SHIP_PHYSICS_RADIUS = 1.75f;
constexpr float PLAYER_SHIP_COSMETIC_RADIUS = 2.25f;
constexpr float PLAYER_SHIP_MAX_SPEED = 15.0f;

constexpr float WORMHOLE_PHYSICS_RADIUS = 4.8f;
constexpr float WORMHOLE_COSMETIC_RADIUS = 5.0f;
constexpr float WORMHOLE_WIDTH = 2.0f;
constexpr float WORMHOLE_VELOCITY = 4.0f;
constexpr int MAX_WORMHOLES = 3;
constexpr float WORMHOLE_FREQUENCY = 0.10f;
constexpr float WORMHOLE_DEBRIS_SCALE = 0.5f;
constexpr double WORMHOLE_COOLDOWN = 5.0;
constexpr float SHRINK_FACTOR = 1.0f;

constexpr int SHIP_VERTS = 24;
constexpr int ASTEROID_VERTS = 16;
constexpr int BULLET_VERTS = 6;
constexpr int WASP_VERTS = 15;
constexpr float WASP_PHYSICS_RADIUS = 1.85f;
constexpr float WASP_COSMETIC_RADIUS = 2.25f;
constexpr float WASP_SPEED = 5.0f;
constexpr float WASP_MAX_SPEED = 14.0f;
constexpr float WASP_ACCELERATION = 7.0f;
constexpr int BEETLE_VERTS = 15;
constexpr float BEETLE_PHYSICS_RADIUS = 1.85f;
constexpr float BEETLE_COSMETIC_RADIUS = 2.25f;
constexpr float BEETLE_SPEED = 8.0f;
constexpr int DEBRIS_VERTS = 6;
constexpr float DEBRIS_MIN_RADIUS = 0.25f;
constexpr float DEBRIS_MAX_RADIUS = 1.5f;
constexpr float DEBRIS_LIFETIME = 2.0f;
constexpr float DEBRIS_SPEED = 5.0f;
constexpr float DEBRIS_ANGULAR_VELOCITY_DEGREES = 300.0f;
constexpr float SPEEDFRACTION = 0.2f;
constexpr float BULLETSCALE = 0.3f;
constexpr float ENTITYSCALE = 1.0f;
constexpr float PLAYERSCALE = 1.0f;

constexpr float THRUST_BASE_OFFSET = -2.0f;
constexpr float THRUST_RANGE_MIN = 0.5f;
constexpr float THRUST_RANGE_MAX = 1.5f;
constexpr double THRUST_PLAY_TIME = 1.0;
constexpr double MUSIC_PLAY_TIME = 85.0;

constexpr int ASTEROID_HEALTH = 3;
constexpr int WASP_HEALTH = 3;
constexpr int BEETLE_HEALTH = 3;
constexpr int SHIP_HEALTH = 1;
constexpr int PLAYER_LIVES = 3;
constexpr int MAX_WAVES = 5;
constexpr double GAME_OVER_WAIT = 3.0;

constexpr float WORLD_CAMERA_WIDTH = 200.0f;
constexpr float WORLD_CAMERA_HEIGHT = 100.0f;
constexpr float WORLD_SCREEN_SHAKE_FRACTION = 0.06f; // starting value
constexpr float WORLD_SCREEN_SHAKE_REDUCTION = 0.02f; // per second reduction
constexpr float SCREEN_CAMERA_WIDTH = 1600.0f;
constexpr float SCREEN_CAMERA_HEIGHT = 800.0f;

struct Vec2;
struct Vec3;
struct Rgba8;

extern Rgba8 shipColor;
extern Rgba8 asteroidColor;
extern Rgba8 beetleColor;
extern Rgba8 waspColor;
extern Rgba8 bulletColor;

constexpr int RING_SEGMENTS = 18;
constexpr float segmentAngle = 360.0f / static_cast<float>(RING_SEGMENTS);

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float width);
void DebugDrawRing(Vec2 center, float radius, Rgba8 color, float width);

extern Renderer* g_theRenderer; // created and owned by the App
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;