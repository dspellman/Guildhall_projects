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
#include "Engine/Core/JobSystem.hpp"

constexpr int	WORLD_MAPS = 3;

constexpr int	MAX_MAPS = 3;

struct Vec2;
struct Vec3;
struct IntVec3;
struct Rgba8;
class BitmapFont;
class JobSystem;

constexpr int RING_SEGMENTS = 18;
constexpr float segmentAngle = 360.0f / static_cast<float>(RING_SEGMENTS);

void DebugDrawLine(Vec2 start, Vec2 end, Rgba8 color, float width);
void DebugDrawRing(Vec2 center, float radius, Rgba8 color, float width);

// Anyone interested in these systems has scope to use global renderer
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern JobSystem* g_theJobSystem;
extern Renderer* g_theRenderer;
extern Window* g_theWindow;
extern BitmapFont* g_testFont;

// direction constants
enum direction
{
	NORTH = 0,
	EAST,
	SOUTH,
	WEST
};

enum BlockFlags
{
	BLOCK_BIT_IS_SKY = 0,
	BLOCK_BIT_IS_LIGHT_DIRTY,
	BLOCK_BIT_IS_FULL_OPAQUE,
	BLOCK_BIT_IS_SOLID,
	BLOCK_BIT_IS_VISIBLE,
	BLOCK_BIT_IS_X,
	BLOCK_BIT_IS_Y,
};

// world parameters
constexpr float CHUNK_ACTIVATION_RANGE = 250.0f;
constexpr float NOISE_SCALE = 200.0f;
constexpr float LIGHTNING_SCALE = 0.01f;
constexpr int NOISE_OCTAVES = 5;
constexpr int LIGHTNING_OCTAVES = 9;
constexpr float HEIGHT_OFFSET_RANGE = 30.0f;
constexpr float MAX_RAYCAST_DISTANCE = 8.0f;

// chunk parameters
constexpr int BITS_X = 4;
constexpr int BITS_Y = 4;
constexpr int BITS_Z = 7;

constexpr int SIZE_X = 1 << BITS_X;
constexpr int SIZE_Y = 1 << BITS_Y;
constexpr int SIZE_Z = 1 << BITS_Z;

constexpr int MASK_X = SIZE_X - 1;
constexpr int MASK_Y = SIZE_Y - 1;
constexpr int MASK_Z = SIZE_Z - 1;

constexpr int BLOCKSPERLAYER = SIZE_X * SIZE_Y;
constexpr int BLOCKSPERCHUNK = BLOCKSPERLAYER * SIZE_Z;

constexpr int WORLD_SEED = 20;
constexpr int MAX_LIGHT = 15;
constexpr int TREE_DIAMETER = 7 - 1; // 7 x 7 tree
constexpr int VILLAGE_DIAMETER = 5 - 1; // 5 x 5 chunk villages
constexpr int VILLAGE_RANGE = 23 - 1; // 23 x 23 chunk regions for possible village
constexpr int NOISE_DIM = 16 + 2 * TREE_DIAMETER;
constexpr int NOISE_ARRAY = NOISE_DIM * NOISE_DIM;

constexpr float OCEAN_SCALE = 500.0f;
constexpr float HILL_SCALE = 700.0f;
constexpr float HUMIDITY_SCALE = 300.0f;
constexpr float TEMP_SCALE = 400.0f;

constexpr float OCEAN_FLOOR = 0.5f;
constexpr float OCEAN_RANGE = 2.0f; // inverse of floor fraction to get range of 0 to 1 for LERP
constexpr float OCEAN_LERPED = 0.0f;
constexpr int MAX_OCEAN_DEPTH = 40;
constexpr float RIVER_FLOOR = 60.0f;
constexpr float HUMIDITY_LINE = 0.35f;
constexpr float BEACH_LINE = 0.6f;

constexpr float DRAG = 9.0f;
constexpr float GRAVITY = 40.0f;
constexpr float FIXED_DISTANCE = 10.0f;
constexpr float TOLERANCE = 0.1f;
constexpr float BUFFER = 0.49f;
constexpr float JUMP_IMPULSE = 35.0f;
