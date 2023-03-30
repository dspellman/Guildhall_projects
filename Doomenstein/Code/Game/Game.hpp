#pragma once
#include <vector>
#include "Game/GameCommon.hpp"
#include "Player.hpp"
#include "Line3D.hpp"
#include "Map.hpp"
#include "Engine/Core/Clock.hpp"

enum GameState
{
	ATTRACT,
	LOBBY,
	PLAYING,
	RESTART,
	STATES
};

enum class ControlType
{
	MOUSE_KEYBOARD = -1,
	CONTROLLER = 0
};

enum Textures
{
	TEXTURE_UNDEFINED = -1,
	TEXTURE_TERRAIN,
	TEXTURE_ATTRACT_SCREEN,
	TEXTURE_ANIMATION_MARINE,
	TEXTURE_ANIMATION_DEMON,
	TEXTURE_PISTOL_HIT,
	TEXTURE_BLOOD_SPATTER,
	TEXTURE_PLASMA_BULLET,
	TEXTURE_WEAPON_PISTOL,
	TEXTURE_WEAPON_PLASMA,
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

class Game
{
public:
	GameState m_gameState = GameState::ATTRACT; // default start in attract mode
	GameState m_nextState = GameState::ATTRACT;

	Player* m_player[2];

	float shakeFraction = 0.0f;
	bool m_fullMap = false;
	float m_numTilesInViewVertically = 4.0; // x dimension is twice this number
	// flags for actions selected by user
	bool m_isDebug = false;
// 	bool m_noClip = false;
 	bool m_isQuitting = false;

	std::vector<Texture*> m_textures;
	SpriteSheet* m_explosionSpriteSheet = nullptr;
	SpriteSheet* m_terrainSpriteSheet = nullptr;

	std::vector<SoundID> m_soundID;
	std::vector<SoundPlaybackID> m_soundPlaybackID;
	float m_volume = 0.1f;
//	float m_volume = 0.0125f;

	Camera m_attractCamera;
	Camera m_screenCamera;

	mutable Player* m_renderingPlayer = nullptr;
	Player* m_players[MAX_PLAYERS];
	int m_numPlayers = 0;

	Line3D* m_x[101] = {};
	Line3D* m_y[101] = {};

	// map variables ///////////////
	Image* m_gameMap = nullptr;
	Map* m_map = nullptr;
	float m_averageDeltaTime = 1.0f / 60.0f;

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

	void JoinPlayer(int controllerIndex);
	Player* GetJoinedPlayer(int controllerIndex);
	void AddPlayer(int playerIndex, int controllerIndex);
	void RemovePlayer(int playerIndex);
	void SetPlayerViewports();
	Player* GetPlayer(int playerIndex) const;
	Player* GetRenderingPlayer() const;
	int GetNumPlayers() const;
	void InitializePlayers();
	void SetCameras(int playerNum);

private:
	void PlayAttractMusic();
	void PlayGameMusic();
	void SetSoundPlaybackParameters(float volume);
	void SplashScreen() const;
	void DeathScreen() const;
	void TestTexture() const;
	void AdvisoryTexture() const;
	void TestSprites() const;
	void CreateMap();

	void KeyboardPlayerInfo(int playerNum, float printOffset) const;
	void ControllerPlayerInfo(int playerNum, float printOffset) const;
	double m_lastDiscoveryTime = 0.0;
	bool m_showDevConsole = false;
};

extern Game* g_theGame;
