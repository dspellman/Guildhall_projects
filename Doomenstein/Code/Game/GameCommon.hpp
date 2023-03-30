#pragma once
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/Window.hpp"

struct Vec2;
struct Vec3;
struct Rgba8;
class BitmapFont;

constexpr float GTIME = 15.0f;
constexpr float HEAT_MAX = 9999.0f;
constexpr float BOSS_MAX = 40.0f;
constexpr float GRAVITY = 2.0f;
constexpr float FRACTION = 0.15f;
constexpr int MAX_PLAYERS = 2;
constexpr int RING_SEGMENTS = 18;
constexpr float segmentAngle = 360.0f / static_cast<float>(RING_SEGMENTS);

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float width);
void DebugDrawRing(Vec2 center, float radius, Rgba8 color, float width);

// Anyone interested in these systems has scope to use global renderer
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Renderer* g_theRenderer;
extern Window* g_theWindow;
extern BitmapFont* g_testFont;

extern int g_maxPlayers;
