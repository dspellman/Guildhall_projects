#pragma once
#include "Game/GameCommon.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Engine/Renderer/Camera.hpp"

class PlayerShip;
class Asteroid;
class Bullet;
class Wasp;
class Beetle;
class Debris;
class Wormhole;

enum GameState
{
	ATTRACT,
	PLAY,
	VICTORY,
	DEFEAT,
	STATES
};

class Game
{
public:
	GameState m_gameState = GameState::ATTRACT; // default start in attract mode
	PlayerShip* m_playerShip = nullptr;		// Just one player ship (for now...)
	Asteroid* 	m_asteroids[MAX_ASTEROIDS] = {};	// Fixed number of asteroid “slots”; nullptr if unused.
	Bullet* 	m_bullets[MAX_BULLETS] = {};	// The “= {};” syntax initializes the array to zeros.
	Wasp* m_wasps[MAX_WASPS] = {};
	Beetle* m_beetles[MAX_BEETLES] = {};
	Debris* m_debris[MAX_DEBRIS] = {};
	Wormhole* m_wormhole[MAX_WORMHOLES] = {};

	float shakeFraction = 0.0f;

	// flags for actions selected by user
	bool m_isDebug = false;
	bool m_isQuitting = false;
	bool m_isSlowMo = false;
	double m_gameOverWait = 0.0; // time the end game wait
	bool m_waitingToEnd = false;
	bool m_showDevConsole = false;

	SoundPlaybackID m_musicSoundPlaybackID = 0;
	Camera m_attractCamera;
	PlayerShip* m_attractShip = nullptr;
	Beetle* m_attractBeetle = nullptr;

	int m_wave = 0;

	Camera m_worldCamera; // needed for API in assignment example code
	Camera m_screenCamera; // needed for API in assignment example code

public:
	Game();
	void Startup();
	void Update(float deltaSeconds);
	void UpdateEntities(float deltaSeconds);
	void UpdateCameras( float deltaSeconds);
	void CheckWormholeCollisions();
	void CheckCollisions();
	bool WaveComplete();
	void DeleteGarbageEntities();
	void Render() const;
	void RenderEntities() const;
	void DebugRender() const;
	void Shutdown();

	void SpawnRandomAsteroid();
	void SpawnRandomWasp();
	void SpawnRandomBeetle();
	void SpawnDebris(Vec2 startPos, Vec2 velocity, int pieces, float scale, Rgba8 color);
	void FireBullet(Vec2 position, float orientationDegrees);
	PlayerShip* GetNearestLivingPlayer(Vec2 position);

private:
	void RenderLives() const;
	void SpawnWave(int wave);

	int asteroidsPerWave[MAX_WAVES] = {3, 5, 7, 9, 11};
	int beetlesPerWave[MAX_WAVES] = {1, 1, 2, 2, 3};
	int waspsPerWave[MAX_WAVES] = {0, 1, 1, 2, 2};
	void MakeEasyMode();
	void PlayMusic();
	void CheckGameOver();
};