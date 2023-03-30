#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/PlayerShip.hpp"
#include "Game/Bullet.hpp"
#include "Game/Asteroid.hpp"
#include "Game/Wasp.hpp"
#include "Game/Beetle.hpp"
#include "Game/Debris.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Game/GameCommon.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Game/Wormhole.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Audio/AudioSystem.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Core/EngineCommon.hpp"

extern AudioSystem* g_theAudio;

RandomNumberGenerator random; // singleton for now only used by entities in the Game

bool Command_Test(EventArgs& args)
{
	UNUSED(args);
	return false;
}

Game::Game()
{
	g_theEventSystem->SubscribeEventCallbackFunction("test", Command_Test);
}

void Game::Startup()
{
//	m_gameState = PLAY;
	if (m_gameState == ATTRACT)
	{
		m_isDebug = false; // only meaningful in playing modes
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		m_attractShip = new PlayerShip(this, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
		m_attractBeetle = new Beetle(this, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
	}
	else
	{
		m_gameState = PLAY; // any state but attract comes here and will be play
		m_worldCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		m_wave = 0;
		m_playerShip = new PlayerShip(this, Vec2(WORLD_CENTER_X, WORLD_CENTER_Y));
		for (int count = 0; count < MAX_WORMHOLES; count++)
		{
			m_wormhole[count] = new Wormhole(this, Vec2(0.0f, 0.0f));
		}
		SpawnWave(m_wave); // start at the beginning by default
	}
}

/////////////////////////////////////////////////////////////
// Key press actions:
// Q: quit program
// T: slow motion while key pressed
// P: toggle pause state of game
// O: single step frame and pause
// I: create additional asteroid
// F1 : debug mode
// F3 : easy mode (eliminate almost all enemies)
// F8 : restart game

void Game::Update(float deltaSeconds)
{
	static int frameCount = 0;
	frameCount++;
	if (frameCount % 44 == 0)
		FireEvent("test");

	if (g_theInput->WasKeyJustReleased(KEYCODE_TILDE))
	{
		m_showDevConsole = !m_showDevConsole;
		//g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		g_systemClock.TogglePause();
	}

	if (g_theInput->IsKeyDown('T'))
	{
		m_isSlowMo = true; // holding T key causes slow motion
		g_systemClock.SetTimeDilation(0.1f);
	}

	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_isSlowMo = false;  // slow motion ends when T is released
		g_systemClock.SetTimeDilation(1.0f);
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		g_systemClock.StepFrame(); // just sets the flag to cause stepping
	}

	// state specific updates
	if (m_gameState == ATTRACT)
	{
		PlayMusic();
		if (g_theInput->WasKeyJustReleased('N') || g_theInput->WasKeyJustReleased(KEYCODE_SPACE))
		{
			Shutdown();
			m_gameState = PLAY;
			Startup();
		}

		XboxController controller = g_theInput->GetController(0); // leap of faith
		if (controller.WasButtonJustPressed(XboxButtonID::A) || controller.WasButtonJustPressed(XboxButtonID::START))
		{
			Shutdown();
			m_gameState = PLAY;
			Startup();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			m_isQuitting = true;  // pressing Q tell the program to quit
			Shutdown();
		}

		// update camera last
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
	}
	else // any mode but attract
	{
		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			m_isDebug = !m_isDebug; // toggle debug rendering
		}

		if (g_theInput->WasKeyJustPressed('I'))
		{
			SpawnRandomAsteroid();
		}

		UpdateEntities(static_cast<float>(g_systemClock.GetDeltaTime()));
		CheckCollisions();
		CheckWormholeCollisions();

		if (WaveComplete())
		{
			m_wave++;
			if (m_wave >= MAX_WAVES)
			{
				if (!(m_gameState == VICTORY))
				{
					SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Scifi_Heal_Cloak09.wav");
					g_theAudio->StartSound(soundID);
				}
				m_gameState = VICTORY; // we won even if we died on last collision!
			}
			else
			{
				SpawnWave(m_wave);
			}
		}

		CheckGameOver();

		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			MakeEasyMode();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
		{
			Shutdown();
			Startup();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			Shutdown();
			m_gameState = ATTRACT;
			Startup();
		}

		UpdateCameras(deltaSeconds);
	}
}

void Game::UpdateEntities(float deltaSeconds)
{
	m_playerShip->Update(deltaSeconds);

	for (int i = 0; i < MAX_WORMHOLES; i++)
	{
		if (m_wormhole[i])
		{
			m_wormhole[i]->Update(deltaSeconds);
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i])
		{
			m_bullets[i]->Update(deltaSeconds);
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i])
		{
			m_asteroids[i]->Update(deltaSeconds);
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i])
		{
			m_wasps[i]->Update(deltaSeconds);
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i])
		{
			m_beetles[i]->Update(deltaSeconds);
		}
	}

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i])
		{
			m_debris[i]->Update(deltaSeconds);
		}
	}
}

void Game::UpdateCameras(float deltaSeconds)
{
	m_worldCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
	m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));

	// add random variation if camera is shaking
	float xShake = random.RollRandomFloatInRange(-WORLD_CAMERA_WIDTH * shakeFraction, WORLD_CAMERA_WIDTH * shakeFraction);
	float yShake = random.RollRandomFloatInRange(-WORLD_CAMERA_HEIGHT * shakeFraction, WORLD_CAMERA_HEIGHT * shakeFraction);
	m_worldCamera.Translate2d(Vec2(xShake, yShake));
	if (shakeFraction > 0.0f)
	{
		shakeFraction -= WORLD_SCREEN_SHAKE_REDUCTION * deltaSeconds;
		Clamp(shakeFraction, 0.0f, WORLD_SCREEN_SHAKE_FRACTION);
	}
}

void Game::CheckWormholeCollisions()
{
	for (int w = 0; w < MAX_WORMHOLES; w++)
	{
		Wormhole*& originWormhole = m_wormhole[w];
		if (!m_playerShip->m_shrinking && m_playerShip->IsAlive() && (GetCurrentTimeSeconds() - m_playerShip->m_savedTime > WORMHOLE_COOLDOWN) &&
			DoDiscsOverlap(originWormhole->m_position, originWormhole->m_physicalRadius, m_playerShip->m_position, m_playerShip->m_physicalRadius))
		{
			m_playerShip->SetupTeleportation(originWormhole);
		}

		for (int iA = 0; iA < MAX_BULLETS; iA++)
		{
			if (!m_bullets[iA])
				continue;
			Bullet*& bullet = m_bullets[iA];
			if (!bullet->m_shrinking && (GetCurrentTimeSeconds() - bullet->m_savedTime > WORMHOLE_COOLDOWN) &&
				DoDiscsOverlap(originWormhole->m_position, originWormhole->m_physicalRadius, bullet->m_position, bullet->m_physicalRadius))
			{
				bullet->SetupTeleportation(originWormhole);
			}
		}

		for (int iA = 0; iA < MAX_ASTEROIDS; iA++)
		{
			if (!m_asteroids[iA])
				continue;
			Asteroid*& asteroid = m_asteroids[iA];
			if (!asteroid->m_shrinking && (GetCurrentTimeSeconds() - asteroid->m_savedTime > WORMHOLE_COOLDOWN) &&
				DoDiscsOverlap(originWormhole->m_position, originWormhole->m_physicalRadius, asteroid->m_position, asteroid->m_physicalRadius))
			{
				asteroid->SetupTeleportation(originWormhole);
			}
		}

		for (int iA = 0; iA < MAX_BEETLES; iA++)
		{
			if (!m_beetles[iA])
				continue;
			Beetle*& beetle = m_beetles[iA];
			if (!beetle->m_shrinking && (GetCurrentTimeSeconds() - beetle->m_savedTime > WORMHOLE_COOLDOWN) &&
				DoDiscsOverlap(originWormhole->m_position, originWormhole->m_physicalRadius, beetle->m_position, beetle->m_physicalRadius))
			{
				beetle->SetupTeleportation(originWormhole);
			}
		}

		for (int iA = 0; iA < MAX_WASPS; iA++)
		{
			if (!m_wasps[iA])
				continue;
			Wasp*& wasp = m_wasps[iA];
			if (!wasp->m_shrinking && (GetCurrentTimeSeconds() - wasp->m_savedTime > WORMHOLE_COOLDOWN) &&
				DoDiscsOverlap(originWormhole->m_position, originWormhole->m_physicalRadius, wasp->m_position, wasp->m_physicalRadius))
			{
				wasp->SetupTeleportation(originWormhole);
			}
		}
	}
}

void Game::CheckCollisions()
{
	// Test and handle collisions between entities of interest
	for (int iA = 0; iA < MAX_ASTEROIDS; iA++)
	{
		Asteroid*& asteroid = m_asteroids[iA];
		if (asteroid == nullptr || asteroid->m_isDead)
			continue; // skip non-existent or garbage asteroids

		if (m_playerShip->IsAlive() &&
			DoDiscsOverlap(asteroid->m_position, asteroid->m_physicalRadius, m_playerShip->m_position, m_playerShip->m_physicalRadius))
		{
			m_playerShip->Die(); // ship is destroyed by asteroid collision
			SpawnDebris(m_playerShip->m_position, m_playerShip->m_velocity, random.RollRandomIntInRange(5, 30), PLAYERSCALE, shipColor);
			--asteroid->m_health; // ship hit reduces health by one according to specification
			if (asteroid->m_health <= 0)
			{
				asteroid->Die(); // asteroid is destroyed by collision with player ship
				SpawnDebris(asteroid->m_position, asteroid->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, asteroidColor);
			}
		}

		for (int iB = 0; iB < MAX_BULLETS; iB++)
		{
			Bullet*& bullet = m_bullets[iB];
			if (bullet == nullptr || bullet->m_isDead)
				continue; // skip non-existent or garbage bullets
			if (DoDiscsOverlap(asteroid->m_position, asteroid->m_physicalRadius, bullet->m_position, bullet->m_physicalRadius))
			{
				--asteroid->m_health; // bullet hit reduces health by one
				if (asteroid->m_health <= 0)
				{
					SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Explo_Small_12.wav");
					g_theAudio->StartSound(soundID);
					asteroid->Die(); // Asteroid destroyed when health is gone
					SpawnDebris(asteroid->m_position, asteroid->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, asteroidColor);
				}
				SpawnDebris(bullet->m_position, asteroid->m_velocity - (SPEEDFRACTION * bullet->m_velocity), random.RollRandomIntInRange(1, 3), BULLETSCALE, bulletColor);
				bullet->Die(); // bullet is destroyed on asteroid collision
			}
		}
	}

	for (int iA = 0; iA < MAX_WASPS; iA++)
	{
		Wasp*& wasp = m_wasps[iA];
		if (wasp == nullptr || wasp->m_isDead)
			continue; // skip non-existent or garbage asteroids

		if (m_playerShip->IsAlive() &&
			DoDiscsOverlap(wasp->m_position, wasp->m_physicalRadius, m_playerShip->m_position, m_playerShip->m_physicalRadius))
		{
			m_playerShip->Die(); // ship is destroyed by asteroid collision
			SpawnDebris(m_playerShip->m_position, m_playerShip->m_velocity, random.RollRandomIntInRange(5, 30), PLAYERSCALE, shipColor);
			--wasp->m_health; // ship hit reduces health by one according to specification
			if (wasp->m_health <= 0)
			{
				wasp->Die(); // asteroid is destroyed by collision with player ship
				SpawnDebris(wasp->m_position, wasp->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, waspColor);
			}
		}

		for (int iB = 0; iB < MAX_BULLETS; iB++)
		{
			Bullet*& bullet = m_bullets[iB];
			if (bullet == nullptr || bullet->m_isDead)
				continue; // skip non-existent or garbage bullets
			if (DoDiscsOverlap(wasp->m_position, wasp->m_physicalRadius, bullet->m_position, bullet->m_physicalRadius))
			{
				--wasp->m_health; // bullet hit reduces health by one
				if (wasp->m_health <= 0)
				{
					SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Explo_Small_12.wav");
					g_theAudio->StartSound(soundID);
					wasp->Die(); // asteroid is destroyed by collision with player ship
					SpawnDebris(wasp->m_position, wasp->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, waspColor);
				}
				bullet->Die(); // bullet is destroyed on asteroid collision
				SpawnDebris(bullet->m_position, wasp->m_velocity - (SPEEDFRACTION * bullet->m_velocity), random.RollRandomIntInRange(1, 3), BULLETSCALE, bulletColor);
			}
		}
	}

	for (int iA = 0; iA < MAX_BEETLES; iA++)
	{
		Beetle*& beetle = m_beetles[iA];
		if (beetle == nullptr || beetle->m_isDead)
			continue; // skip non-existent or garbage asteroids

		if (m_playerShip->IsAlive() &&
			DoDiscsOverlap(beetle->m_position, beetle->m_physicalRadius, m_playerShip->m_position, m_playerShip->m_physicalRadius))
		{
			m_playerShip->Die(); // ship is destroyed by asteroid collision
			SpawnDebris(m_playerShip->m_position, m_playerShip->m_velocity, random.RollRandomIntInRange(5, 30), PLAYERSCALE, shipColor);
			--beetle->m_health; // ship hit reduces health by one according to specification
			if (beetle->m_health <= 0)
			{
				beetle->Die(); // asteroid is destroyed by collision with player ship
				SpawnDebris(beetle->m_position, beetle->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, beetleColor);
			}
		}

		for (int iB = 0; iB < MAX_BULLETS; iB++)
		{
			Bullet*& bullet = m_bullets[iB];
			if (bullet == nullptr || bullet->m_isDead)
				continue; // skip non-existent or garbage bullets
			if (DoDiscsOverlap(beetle->m_position, beetle->m_physicalRadius, bullet->m_position, bullet->m_physicalRadius))
			{
				--beetle->m_health; // bullet hit reduces health by one
				if (beetle->m_health <= 0)
				{
					SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Explo_Small_12.wav");
					g_theAudio->StartSound(soundID);
					beetle->Die(); // asteroid is destroyed by collision with player ship
					SpawnDebris(beetle->m_position, beetle->m_velocity, random.RollRandomIntInRange(3, 12), ENTITYSCALE, beetleColor);
				}
				bullet->Die(); // bullet is destroyed on asteroid collision
				SpawnDebris(bullet->m_position, beetle->m_velocity - (SPEEDFRACTION * bullet->m_velocity), random.RollRandomIntInRange(1, 3), BULLETSCALE, bulletColor);
			}
		}
	}

	// delete garbage entities (bullet or asteroid)
	DeleteGarbageEntities();
}

bool Game::WaveComplete()
{
	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i])
		{
			return false; // early exit from function
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i])
		{
			return false; // early exit from function
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i])
		{
			return false; // early exit from function
		}
	}

	return true; // nothing left so wave is complete
}

void Game::DeleteGarbageEntities()
{
	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i] && m_bullets[i]->m_isGarbage)
		{
			delete m_bullets[i];
			m_bullets[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i] && m_asteroids[i]->m_isGarbage)
		{
			delete m_asteroids[i];
			m_asteroids[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i] && m_wasps[i]->m_isGarbage)
		{
			delete m_wasps[i];
			m_wasps[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i] && m_beetles[i]->m_isGarbage)
		{
			delete m_beetles[i];
			m_beetles[i] = nullptr;
		}
	}

	for (int i = 0; i < MAX_DEBRIS; i++)
	{
		if (m_debris[i] && m_debris[i]->m_isGarbage)
		{
			delete m_debris[i];
			m_debris[i] = nullptr;
		}
	}
}

void Game::Render() const
{
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	float cellHeight = 0.0f;
	float textWdith = 0.0f;

	switch (m_gameState)
	{
	case ATTRACT:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		g_theRenderer->BeginCamera(m_attractCamera);
		g_theRenderer->BindTexture(nullptr);
		cellHeight = 10.0f;
		str = "Starship Gold";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2(WORLD_CENTER_X - textWdith * 0.5f, WORLD_CENTER_Y - cellHeight * 0.5f), cellHeight, Rgba8(255, 150, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		cellHeight = 3.0f;
		str = "Press SPACE, 'N' key or joystick 'A' button to start";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2(WORLD_CENTER_X - textWdith * 0.5f, WORLD_CENTER_Y - 10.0f), cellHeight, Rgba8(255, 150, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);

		if (m_attractShip)
			m_attractShip->AttractRender(Vec2(WORLD_CENTER_X, WORLD_CENTER_Y), 0.0f, 4.0f);
		if (m_attractBeetle)
			m_attractBeetle->AttractRender(Vec2(WORLD_CENTER_X, WORLD_CENTER_Y), 0.0f, 4.0f);

		g_theRenderer->EndCamera(m_attractCamera);

		g_theRenderer->BeginCamera(m_screenCamera); 
		// show console
		if (m_showDevConsole)
		{
			g_theConsole->SetMode(DevConsoleMode::FULL);
			g_theConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()));
		}
		else
		{
			g_theConsole->SetMode(DevConsoleMode::HIDDEN);
		}

		g_theRenderer->EndCamera(m_screenCamera);
		break;

	case PLAY:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
		g_theRenderer->BeginCamera(m_worldCamera); 
		g_theRenderer->BindTexture(nullptr);
		RenderEntities();
		if (m_isDebug)
		{
			DebugRender();
		}
		g_theRenderer->EndCamera(m_worldCamera);
		g_theRenderer->BeginCamera(m_screenCamera); 
		RenderLives();

		// show console
		if (m_showDevConsole)
		{
			g_theConsole->SetMode(DevConsoleMode::FULL);
			g_theConsole->Render(AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight()));
		}
		else
		{
			g_theConsole->SetMode(DevConsoleMode::HIDDEN);
		}

		g_theRenderer->EndCamera(m_screenCamera);
		break;

	case VICTORY:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
		g_theRenderer->BeginCamera(m_worldCamera); 
		g_theRenderer->BindTexture(nullptr);
		RenderEntities();
		if (m_isDebug)
		{
			DebugRender();
		}
		g_theRenderer->EndCamera(m_worldCamera);
		g_theRenderer->BeginCamera(m_screenCamera); 
		g_theRenderer->BindTexture(nullptr);
		RenderLives();
		cellHeight = 160.0f;
		str = "Victory!";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2((SCREEN_CAMERA_WIDTH - textWdith) * 0.5f, (SCREEN_CAMERA_HEIGHT - cellHeight) * 0.5f), cellHeight,  Rgba8(0, 255, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		g_theRenderer->EndCamera(m_screenCamera);
		break;

	case DEFEAT:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
		g_theRenderer->BeginCamera(m_worldCamera); 
		g_theRenderer->BindTexture(nullptr);
		RenderEntities();
		if (m_isDebug)
		{
			DebugRender();
		}
		g_theRenderer->EndCamera(m_worldCamera);
		g_theRenderer->BeginCamera(m_screenCamera); 
		g_theRenderer->BindTexture(nullptr);
		RenderLives();
		cellHeight = 160.0f;
		str = "Defeat!";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2((SCREEN_CAMERA_WIDTH - textWdith) * 0.5f, (SCREEN_CAMERA_HEIGHT - cellHeight) * 0.5f), cellHeight, Rgba8(255, 0, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		g_theRenderer->EndCamera(m_screenCamera);
		break;

	default:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
		g_theRenderer->BeginCamera(m_screenCamera); 
		g_theRenderer->BindTexture(nullptr);
		cellHeight = 160.0f;
		str = "Unknown Game State!";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2((SCREEN_CAMERA_WIDTH - textWdith) * 0.5f, (SCREEN_CAMERA_HEIGHT - cellHeight) * 0.5f), cellHeight, Rgba8(255, 0, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		g_theRenderer->EndCamera(m_screenCamera);
		break;
	}
}

void Game::RenderEntities() const
{
	for (int i = 0; i < MAX_WORMHOLES; i++)
	{
		if (m_wormhole[i])
		{
			m_wormhole[i]->Render();
		}
	}

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (m_bullets[i])
			m_bullets[i]->Render();
	}

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i])
			m_asteroids[i]->Render();
	}

	for (int index = 0; index < MAX_WASPS; index++)
	{
		if (m_wasps[index])
		{
			m_wasps[index]->Render();
		}
	}

	for (int index = 0; index < MAX_BEETLES; index++)
	{
		if (m_beetles[index])
		{
			m_beetles[index]->Render();
		}
	}

	for (int index = 0; index < MAX_DEBRIS; index++)
	{
		if (m_debris[index])
		{
			m_debris[index]->Render();
		}
	}

	if (m_playerShip->IsAlive())
	{
		m_playerShip->Render();
	}
}

void Game::DebugRender() const
{
	// Draw lines to entities from player ship
	for (int index = 0; index < MAX_WORMHOLES; index++)
	{
		if (m_wormhole[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_wormhole[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	for (int index = 0; index < MAX_ASTEROIDS; index++)
	{
		if (m_asteroids[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_asteroids[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	for (int index = 0; index < MAX_BULLETS; index++)
	{
		if (m_bullets[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_bullets[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	for (int index = 0; index < MAX_WASPS; index++)
	{
		if (m_wasps[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_wasps[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	for (int index = 0; index < MAX_BEETLES; index++)
	{
		if (m_beetles[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_beetles[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	for (int index = 0; index < MAX_DEBRIS; index++)
	{
		if (m_debris[index])
		{
			DebugDrawLine(m_playerShip->m_position, m_debris[index]->m_position, Rgba8(50, 50, 50, 255), 0.25f);
		}
	}

	// Draw entity debug info
	for (int index = 0; index < MAX_WORMHOLES; index++)
	{
		if (m_wormhole[index])
		{
			m_wormhole[index]->DebugRender();
		}
	}

	for (int index = 0; index < MAX_ASTEROIDS; index++)
	{
		if (m_asteroids[index])
		{
			m_asteroids[index]->DebugRender();
		}
	}

	for (int index = 0; index < MAX_BULLETS; index++)
	{
		if (m_bullets[index])
		{
			m_bullets[index]->DebugRender();
		}
	}

	for (int index = 0; index < MAX_WASPS; index++)
	{
		if (m_wasps[index])
		{
			m_wasps[index]->DebugRender();
		}
	}

	for (int index = 0; index < MAX_BEETLES; index++)
	{
		if (m_beetles[index])
		{
			m_beetles[index]->DebugRender();
		}
	}

	for (int index = 0; index < MAX_DEBRIS; index++)
	{
		if (m_debris[index])
		{
			m_debris[index]->DebugRender();
		}
	}

	m_playerShip->DebugRender();
}

void Game::Shutdown()
{
	if (m_gameState == ATTRACT)
	{
		g_theAudio->StopSound(m_musicSoundPlaybackID);
		delete m_attractShip;
		m_attractShip = nullptr;
		delete m_attractBeetle;
		m_attractBeetle = nullptr;
	}
	else
	{
		delete m_playerShip;
		m_playerShip = nullptr;

		for (int i = 0; i < MAX_WORMHOLES; i++)
		{
			if (m_wormhole[i])
			{
				delete m_wormhole[i];
				m_wormhole[i] = nullptr;
			}
		}

		for (int i = 0; i < MAX_ASTEROIDS; i++)
		{
			if (m_asteroids[i])
			{
				delete m_asteroids[i];
				m_asteroids[i] = nullptr;
			}
		}

		for (int i = 0; i < MAX_BULLETS; i++)
		{
			if (m_bullets[i])
			{
				delete m_bullets[i];
				m_bullets[i] = nullptr;
			}
		}

		for (int i = 0; i < MAX_WASPS; i++)
		{
			if (m_wasps[i])
			{
				delete m_wasps[i];
				m_wasps[i] = nullptr;
			}
		}

		for (int i = 0; i < MAX_BEETLES; i++)
		{
			if (m_beetles[i])
			{
				delete m_beetles[i];
				m_beetles[i] = nullptr;
			}
		}

		for (int i = 0; i < MAX_DEBRIS; i++)
		{
			if (m_debris[i])
			{
				delete m_debris[i];
				m_debris[i] = nullptr;
			}
		}
	}
}

void Game::SpawnRandomAsteroid()
{
	bool failed = true;

	for (int i = 0; i < MAX_ASTEROIDS; i++)
	{
		if (failed && nullptr == m_asteroids[i])
		{
			m_asteroids[i] = new Asteroid(this, Vec2(0.0f, 0.0f));
			failed = false;
			break;
		}
	}

	if (failed)
		ERROR_AND_DIE("That's an astronomical number of asteroids, Cap'n Sir!");
}

void Game::SpawnRandomWasp()
{
	bool failed = true;

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (failed && nullptr == m_wasps[i])
		{
			m_wasps[i] = new Wasp(this, Vec2(0.0f, 0.0f));
			failed = false;
			break;
		}
	}

	if (failed)
		ERROR_AND_DIE("We got stung trying to instantiate!");
}

void Game::SpawnRandomBeetle()
{
	bool failed = true;

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (failed && nullptr == m_beetles[i])
		{
			m_beetles[i] = new Beetle(this, Vec2(0.0f, 0.0f));
			failed = false;
			break;
		}
	}

	if (failed)
		ERROR_AND_DIE("Time to call the exterminator!");
}

void Game::SpawnDebris(Vec2 startPos, Vec2 velocity, int pieces, float scale, Rgba8 color)
{
	float angleDegrees = 360.0f / pieces;
	for (int piece = 0; piece < pieces; piece++)
	{
		bool failed = true;

		for (int i = 0; i < MAX_DEBRIS; i++)
		{
			if (failed && nullptr == m_debris[i])
			{
				m_debris[i] = new Debris(this, startPos, velocity * random.RollRandomFloatZeroToOne(), 
					angleDegrees * (static_cast<float>(piece) + random.RollRandomFloatInRange(-0.45f, 0.45f)), scale, color);
				failed = false;
				break;
			}
		}

		if (failed)
			return; // no room for more debris
	}
}

void Game::FireBullet(Vec2 position, float orientationDegrees)
{
	bool failed = true;

	for (int i = 0; i < MAX_BULLETS; i++)
	{
		if (failed && nullptr == m_bullets[i])
		{
			m_bullets[i] = new Bullet(this, position, orientationDegrees);
			failed = false;
			break;
		}
	}

	if (failed)
		ERROR_AND_DIE("That's some wild shootin, Pardner!");
}

PlayerShip* Game::GetNearestLivingPlayer(Vec2 position)
{
	if (m_playerShip->IsAlive())
	{
		return m_playerShip;
	}
	return nullptr;
}

void Game::RenderLives() const
{
	Vec2 lifePos = Vec2(40.0f, 0.97 * SCREEN_CAMERA_HEIGHT);
	for (int life = 0; life < m_playerShip->m_extraLives; life++)
	{
		m_playerShip->LifeRender(lifePos, 0.0f, 6.0f);
		lifePos.x += 35.0f;
	}
}

void Game::SpawnWave(int wave)
{
	for (int count = 0; count < asteroidsPerWave[wave]; count++)
	{
		SpawnRandomAsteroid();
	}

	for (int count = 0; count < waspsPerWave[wave]; count++)
	{
		SpawnRandomWasp();
	}

	for (int count = 0; count < beetlesPerWave[wave]; count++)
	{
		SpawnRandomBeetle();
	}
	SoundID soundID = g_theAudio->CreateOrGetSound("Data/Audio/Scifi_Heal_Cloak03.wav");
	g_theAudio->StartSound(soundID);
}

void Game::MakeEasyMode()
{
	// kill all but one asteroid in the wave
	for (int i = 1; i < MAX_ASTEROIDS; i++)
	{
		if (m_asteroids[i])
		{
			m_asteroids[i]->Die();
		}
	}

	for (int i = 0; i < MAX_WASPS; i++)
	{
		if (m_wasps[i])
		{
			m_wasps[i]->Die();
		}
	}

	for (int i = 0; i < MAX_BEETLES; i++)
	{
		if (m_beetles[i])
		{
			m_beetles[i]->Die();
		}
	}
}

void Game::PlayMusic()
{
	if (g_theAudio->IsPlaying(m_musicSoundPlaybackID))
		return;
	SoundID musicSoundID = g_theAudio->CreateOrGetSound("Data/Audio/Track01LightYears.mp3");
	m_musicSoundPlaybackID = g_theAudio->StartSound(musicSoundID);
	g_theAudio->SetSoundPlaybackVolume(m_musicSoundPlaybackID, 0.4f);
}

void Game::CheckGameOver()
{
	if (m_gameState == VICTORY || m_gameState == DEFEAT)
	{
		if (!m_waitingToEnd)
		{
			m_waitingToEnd = true;
			m_gameOverWait = GetCurrentTimeSeconds();
		}
		else
		{
			if (GetCurrentTimeSeconds() - m_gameOverWait > GAME_OVER_WAIT)
			{
				Shutdown();
				m_waitingToEnd = false;
				m_gameState = ATTRACT;
				Startup();
			}
		}
	}
}
