#pragma once
#include <vector>
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "World.hpp"

enum GameState
{
	ATTRACT,
	PLAY,
	TEST,
	RESTART,
	STATES
};

enum Textures
{
	UNDEFINED = -1,
	TERRAIN,
	ATTRACT_SCREEN,
	TEXTURE_COUNT
};

enum Sounds
{
	ATTRACTMUSIC,
	CLICK,
	GAMEPLAYMUSIC,
	PAUSE,
	UNPAUSE,
	SOUND_COUNT
};

struct GameConstants
{
	float camWorldPos[4];		// Used for fog thickness calculations, specular lighting, etc.
	float skyColor[4];			// Also used as fog color
	float outdoorLightColor[4];	// Used for outdoor lighting exposure
	float indoorLightColor[4];	// Used for outdoor lighting exposure
	float fogStartDist;			// Fog has zero opacity at or before this distance
	float fogEndDist;			// Fog has maximum opacity at or beyond this distance
	float fogMaxAlpha;			// At and beyond fogEndDist, fog gets this much opacity
	float time;					// CBO structs and members must be 16B-aligned and 16B-sized!!
};

constexpr int CAMERA_GAME_BUFFER_SLOT = 8;

class Game
{
public:
	GameState m_gameState = GameState::ATTRACT; // default start in attract mode
	GameState m_nextState = GameState::ATTRACT; // GameState::ATTRACT;

	float shakeFraction = 0.0f;
	bool m_fullMap = false;
	float m_numTilesInViewVertically = 4.0; // x dimension is twice this number
	// flags for actions selected by user
	bool m_isDebug = false;
	bool m_isQuitting = false;
	double m_gameOverWait = 0.0; // time the end game wait
	bool m_waitingToEnd = false;
	double m_deathStartTime = 0.0;
	bool m_restartMessage = false;

	std::vector<Texture*> m_textures;
	SpriteSheet* m_terrainSpriteSheet = nullptr;
	std::vector<SoundID> m_soundID;
	std::vector<SoundPlaybackID> m_soundPlaybackID;
	float m_volume = 1.0f;

	Camera m_attractCamera;
	Camera m_worldCamera; 
	Camera m_screenCamera; 

	// game constants buffer data
	ConstantBuffer* m_gameCBO = nullptr;
	Rgba8 m_skyColor = Rgba8::BLACK;
	Rgba8 m_outdoorLightColor = Rgba8::WHITE;
	Rgba8 m_indoorLightColor = Rgba8::BLACK; // Rgba8(255, 230, 204);
	float m_fogStartDist = CHUNK_ACTIVATION_RANGE * 0.5f;
	float m_fogEndDist = CHUNK_ACTIVATION_RANGE;
	float m_foxMaxAlpha = 1.0f;
	float m_time = 0.0f;

	World* m_world = nullptr;

public:
	~Game();
	Game();
	void Startup();
	void Update(float deltaSeconds);
	void UpdateEntities(float deltaSeconds);
	void UpdateCameras( float deltaSeconds);
	void Render() const;
	void RenderEntities() const;
	void DebugRender() const;
	void Shutdown();

private:
	void PlayAttractMusic();
	void PlayGameMusic();
	void SetSoundPlaybackParameters(float volume);
	void SplashScreen() const;
	void ShowIcon(AABB2 bounds, char const* iconFilePath, AABB2 uv) const;
	void TestSprites() const;
	void DrawSquare(AABB2 bounds, Rgba8 color) const;
	void DrawTestGame() const;

	double m_lastDiscoveryTime = 0.0;
	bool m_showDevConsole = false;
	bool m_runTest = true;
	std::vector<Job*> m_testJobs = {};
	int m_worldSeed = 0;
};

extern Game* g_theGame;