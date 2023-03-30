#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Renderer/Window.hpp"
#include "Game/Game.hpp"

constexpr float WORLD_SIZE_X = 200.f;
constexpr float WORLD_SIZE_Y = 100.f;
constexpr float WORLD_CENTER_X = WORLD_SIZE_X / 2.f;
constexpr float WORLD_CENTER_Y = WORLD_SIZE_Y / 2.f;

constexpr float WORLD_CAMERA_WIDTH = 200.0f;
constexpr float WORLD_CAMERA_HEIGHT = 100.0f;
constexpr float WORLD_SCREEN_SHAKE_FRACTION = 0.06f; // starting value
constexpr float WORLD_SCREEN_SHAKE_REDUCTION = 0.02f; // per second reduction
constexpr float SCREEN_CAMERA_WIDTH = 1600.0f;
constexpr float SCREEN_CAMERA_HEIGHT = 800.0f;

constexpr float WALL_ELASTICITY = 0.9f;

struct Vec2;
struct Vec3;
struct Rgba8;

constexpr int RING_SEGMENTS = 18;
constexpr float segmentAngle = 360.0f / static_cast<float>(RING_SEGMENTS);

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float width);
void DebugDrawRing(Vec2 center, float radius, Rgba8 color, float width);

// Anyone interested in these systems has scope to use global renderer
extern InputSystem* g_theInput;
extern Renderer* g_theRenderer;
extern Window* g_theWindow;
extern BitmapFont* g_testFont;
//extern Game* g_theGame;