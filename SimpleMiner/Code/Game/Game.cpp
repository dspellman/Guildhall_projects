#include "Game/Game.hpp"
#include "Game/App.hpp"
// #include "Game/Player.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "BlockDefinition.hpp"
#include "ThirdParty/Squirrel/SmoothNoise.hpp"
#include "Game/Entity.hpp"
#include "BlockTemplate.hpp"
#include "TestJob.hpp"
#include "BuildingTemplate.hpp"

RandomNumberGenerator random; // singleton for now only used by entities in the Game

bool Command_Test(EventArgs& args)
{
	UNUSED(args);
//	Vec2 mouse = g_theInput->GetMouseClientDelta();
	g_theConsole->AddLine(Rgba8::WHITE, args.GetValue("Mouse", "none"));
	return false;
}

Game::~Game()
{
	if (m_world)
	{
		delete m_world;
		m_world = nullptr;
	}
	BlockTemplate::Destroy();
	BuildingTemplate::Destroy();
	delete BlockDefinition::s_spriteSheet;

	for (auto index : m_testJobs)
	{
		delete index;
	}
}

Game::Game()
{
	m_worldSeed = g_gameConfigBlackboard.GetValue("WORLD_SEED", WORLD_SEED);

	g_theEventSystem->SubscribeEventCallbackFunction( "test", Command_Test );

	// Load the test font for testing
	g_testFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/MyFixedFont" ); // DO NOT SPECIFY FILE EXTENSION!!  (Important later on.)

	// load all assets for the game here
	m_textures.reserve(TEXTURE_COUNT);
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/BasicSprites_64x64.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.png"));

	// create sprite sheet and block definitions for our terrain texture
	m_terrainSpriteSheet = new SpriteSheet(*m_textures[TERRAIN], IntVec2(64, 64));
	BlockDefinition::s_spriteSheet = m_terrainSpriteSheet;
	BlockDefinition::Initialize("Data/Definitions/BlockDefinitions.xml");
	BlockTemplate::Initialize("Data/Definitions/BlockTemplates.xml");
	BuildingTemplate::Initialize("Data/Definitions/TemplateNames.xml");

	// load all sounds for the game
	m_soundID.reserve(SOUND_COUNT);
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/AttractMusic.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/GameplayMusic.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3"));

	// create array of playback IDs to control volume and stop sounds as needed
	for (int index = 0; index < SOUND_COUNT; index++)
	{
		m_soundPlaybackID.push_back(0); // create an array of null values
	}
}

void Game::Startup()
{
	if (m_gameState == ATTRACT)
	{
		m_isDebug = false; // only meaningful in playing modes
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_WIDTH", 200.0f), g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_HEIGHT", 100.0f)));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f), g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f)));
	}
	else
	{
		m_gameState = PLAY; // any state but attract comes here and will be play
		m_worldCamera.SetOrthoView(Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f));
		m_worldCamera.SetRenderTransform(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
		//m_worldCamera.SetRenderTransform(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f), g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f)));
		// static graphics elements
		DebugRenderClear();

		m_gameCBO = g_theRenderer->CreateConstantBuffer(sizeof(GameConstants));
		m_world = new World(m_worldSeed);
	}

	g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
}

/////////////////////////////////////////////////////////////
// Key press actions:
// Q: quit program
// T: slow motion while key pressed
// Y: fast motion while key pressed
// P: start game or toggle pause state of game
// O: single step frame and pause
// C: Toggle DevConsole display in attract mode
// F1 : debug mode
// F2 : invulnerable mode
// F3 : no clip mode
// F5 : sound volume down
// F6 : sound volume up
// F7 : restart game
// F8 : regenerate current world location

void Game::Update(float deltaSeconds)
{
// 	float fontSize = 20.0f;
// 	float vertical = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
// 	Vec3 pos;

// 	static int frameCount = 0;
// 	frameCount++;
// 	if (frameCount % 44 == 0)
// 		FireEvent("test");

	// handle shutdown here later instead of in middle of updates
	if (m_nextState != m_gameState)
	{
		switch (m_nextState)
		{
		case ATTRACT:
			Shutdown();
			m_gameState = ATTRACT;
			Startup();
			g_systemClock.Unpause();
			DebugRenderClear();
			break;
		case RESTART:
			m_nextState = PLAY; // reset to avoid churn and then fall through
		case PLAY:
			Shutdown();
			m_gameState = PLAY;
			Startup();
//			g_theGame->m_soundPlaybackID[WELCOME] = g_theAudio->StartSound(g_theGame->m_soundID[WELCOME]);
			break;
		case TEST:
			m_gameState = TEST;
			if (g_theAudio->IsPlaying(m_soundPlaybackID[GAMEPLAYMUSIC]))
				g_theAudio->StopSound(m_soundPlaybackID[GAMEPLAYMUSIC]);
//			g_theGame->m_soundPlaybackID[VICTORY] = g_theAudio->StartSound(g_theGame->m_soundID[VICTORY]);
			break;
		}
	}

	// global commands independent of state
	// set parameters based on dev console
	if (g_theInput->WasKeyJustReleased(KEYCODE_TILDE))
	{
		m_showDevConsole = !m_showDevConsole;
		g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
	}

	if (g_theWindow->HasFocus() == false)
	{
		g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
	}
	else if (m_showDevConsole == true)
	{
		g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
	}
	else if (m_gameState == ATTRACT)
	{
		g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
	}
	else // any game play mode
	{
		g_theInput->SetMouseMode(true, true, true); // mouse hidden, clipped and relative
	}

	// speed changes
	if (g_theInput->WasKeyJustPressed('T'))
	{
		g_systemClock.SetTimeDilation(0.1f * g_systemClock.GetTimeDilation());
	}

	if (g_theInput->WasKeyJustReleased('T'))
	{
		g_systemClock.SetTimeDilation(10.0f * g_systemClock.GetTimeDilation());
	}

	if (g_theInput->WasKeyJustPressed('Y'))
	{
		g_systemClock.SetTimeDilation(1.4f * g_systemClock.GetTimeDilation());
	}

	if (g_theInput->WasKeyJustReleased('Y'))
	{
		g_systemClock.SetTimeDilation(g_systemClock.GetTimeDilation() / 1.4f);
	}

	if (g_theInput->WasKeyJustPressed('O'))
	{
		g_systemClock.StepFrame();
	}

	// handle sound volume change and playback mode
	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_volume *= 0.5f;
		m_volume = Clamp(m_volume, 0.0f, 1.0f);
		g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_volume *= 2.0f;
		m_volume = Clamp(m_volume, 0.0f, 1.0f);
		g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
	}

	SetSoundPlaybackParameters(m_volume);

	// state specific updates
	XboxController controller = g_theInput->GetController(0); // leap of faith

	switch (m_gameState)
	{
	case ATTRACT:
		PlayAttractMusic();
		if (g_theInput->WasKeyJustReleased('P'))
		{
			m_nextState = PLAY;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::START))
		{
			m_nextState = PLAY;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			m_isQuitting = true;  // pressing Q tell the program to quit
			Shutdown();
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F9) || g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			m_nextState = TEST;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		// update camera last
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_WIDTH", 200.0f), g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_HEIGHT", 100.0f)));
		break;

	case PLAY:
		PlayGameMusic();
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			if (g_systemClock.IsPaused())
			{
				m_nextState = ATTRACT;
			}
		}

		if (g_theInput->WasKeyJustPressed('P') || g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::START))
		{
			g_theGame->m_soundPlaybackID[g_systemClock.IsPaused() ? PAUSE : UNPAUSE] = g_theAudio->StartSound(g_theGame->m_soundID[g_systemClock.IsPaused() ? PAUSE : UNPAUSE]);
			g_systemClock.TogglePause();
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			m_isDebug = !m_isDebug; // toggle debug rendering
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		UpdateEntities(static_cast<float>(g_systemClock.GetDeltaTime()));

		if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
		{
			m_world->m_player->CycleCameraView();
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			m_world->m_player->CyclePhysics();
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
		{
			m_world->m_player->m_UseAxialCameraControls = !m_world->m_player->m_UseAxialCameraControls;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
		{
			m_nextState = RESTART;
			m_worldSeed++;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
		{
			m_world->ClearChunkMap();
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}
		
// 		if (g_theInput->WasKeyJustReleased('9'))
// 		{
// 			DebugRenderAdjustClockDialation(-0.1);
// 		}
// 		if (g_theInput->WasKeyJustReleased('0'))
// 		{
// 			DebugRenderAdjustClockDialation(0.1);
// 		}

		UpdateCameras(deltaSeconds);
		break;

	case TEST:
		Job* job;
		if (m_runTest)
		{
			m_runTest = false;
			for (int index = 0; index < 800; index++)
			{
				job = new TestJob(index);
				m_testJobs.push_back(job);
				g_theJobSystem->QueueJob(job);
			}
		}

		if (g_theInput->WasKeyJustReleased('A'))
		{
			job = g_theJobSystem->RetrieveCompletedJob();
		}

		if (g_theInput->WasKeyJustReleased('S'))
		{
			job = g_theJobSystem->RetrieveCompletedJob();
			while (job)
			{
				job = g_theJobSystem->RetrieveCompletedJob();
			}
		}

		if (g_theInput->WasKeyJustReleased('D'))
		{
			for (int index = 0; index < 800; index++)
			{
				dynamic_cast<TestJob*>(m_testJobs[index])->ForceComplete();
			}
		}

		for (int index = 0; index < 800; index++)
		{
			if (m_testJobs[index]->m_state != JobState::RETIRED)
			{
				return;
			}
		}

		if (g_theInput->WasKeyJustReleased('N') || controller.WasButtonJustReleased(XboxButtonID::START) ||
			g_theInput->WasKeyJustReleased(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK) ||
			g_theInput->WasKeyJustReleased('P'))
		{
			m_nextState = ATTRACT;
		}

		UpdateCameras(deltaSeconds);
		break;
	}
}

void Game::UpdateEntities(float deltaSeconds)
{
	if (m_world)
	{
		m_world->Update(deltaSeconds);
		double integral;
		float timeOfDay = (float)modf(m_world->m_timeOfDay, &integral);
		Rgba8 skyColor;
		Rgba8 outdoorLightColor;

		// sky color
		if (timeOfDay < 0.25f || timeOfDay > 0.75f)
		{
			skyColor = Rgba8(20, 20, 40);
		}
		else if (timeOfDay < 0.5f)
		{
			skyColor = Rgba8::ColorLerp(Rgba8(20, 20, 40), Rgba8(200, 230, 255), RangeMap(timeOfDay, 0.25f, 0.5f, 0.0f, 1.0f));
		}
		else
		{
			skyColor = Rgba8::ColorLerp(Rgba8(200, 230, 255), Rgba8(20, 20, 40), RangeMap(timeOfDay, 0.5f, 0.75f, 0.0f, 1.0f));
		}

		// outdoor light color
		if (timeOfDay < 0.5f)
		{
			outdoorLightColor = Rgba8::ColorLerp(Rgba8(20, 20, 40), Rgba8::WHITE, RangeMap(timeOfDay, 0.0f, 0.5f, 0.0f, 1.0f));
		}
		else
		{
			outdoorLightColor = Rgba8::ColorLerp(Rgba8::WHITE, Rgba8(20, 20, 40), RangeMap(timeOfDay, 0.5f, 1.0f, 0.0f, 1.0f));
		}

		// lightning color
		float lightningStrength = Clamp(RangeMap( Compute1dPerlinNoise( m_world->m_timeOfDay, LIGHTNING_SCALE, LIGHTNING_OCTAVES, 0.5f, 2.0f, true, 20 ), 0.6f, 0.9f, 0.0f, 1.0f), 0.0f, 1.0f);
		m_skyColor = Rgba8::ColorLerp(skyColor, Rgba8::WHITE, lightningStrength);
		m_outdoorLightColor = Rgba8::ColorLerp(outdoorLightColor, Rgba8::WHITE, lightningStrength);

		// glowstone flicker
		float glowStrength = Clamp(RangeMap(Compute1dPerlinNoise(m_world->m_timeOfDay, LIGHTNING_SCALE, NOISE_OCTAVES, 0.5f, 2.0f, true, 200), -1.0f, 1.0f, 0.8f, 1.0f), 0.0f, 1.0f);
		m_indoorLightColor = Rgba8(255, 230, 204).dim(glowStrength);
	}

	static float deltaAverage = 0.01f;
	deltaAverage = 0.97f * deltaAverage + 0.03f * deltaSeconds;
	deltaAverage = deltaAverage < 0.0003f ? 0.0003f : deltaAverage;
	float fontSize = 20.0f;
	float vertical = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);

	char pbuffer[80];
	Vec3 position = m_world->m_player->m_position;
	sprintf_s(pbuffer, "WASD horizontal, QE vertical, F2 Camera, F3 Physics, ESC exit, Block (1-9): %i", m_world->m_player->m_blockSelection);
	DebugAddScreenText(pbuffer, Vec2(0.0f, vertical - 1.1f * fontSize), 0.0f, Vec2(0.0f, 1.0f), fontSize, Rgba8::GOLD, Rgba8::GOLD);
	if (m_isDebug)
	{
		sprintf_s(pbuffer, "World Location:  %.1f  %.1f  %.1f Frame rate: %.0f (%f ms)", position.x, position.y, position.z, 1.0f / deltaAverage, deltaAverage * 1000.0f);
		DebugAddScreenText(pbuffer, Vec2(0.0f, vertical - 2.2f * fontSize), 0.0f, Vec2(0.0f, 1.0f), fontSize, Rgba8::BLUE, Rgba8::BLUE);
	}
// 	sprintf_s(pbuffer, "indoor lighting:  %i %i %i strength: %.2f", m_indoorLightColor.r, m_indoorLightColor.g, m_indoorLightColor.b, 0.0f);
// 	DebugAddScreenText(pbuffer, Vec2(0.0f, vertical - 2.2f * fontSize), 0.0f, Vec2(0.0f, 1.0f), fontSize, Rgba8::BLUE, Rgba8::BLUE);
}


void Game::UpdateCameras(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_worldCamera.SetPerspectiveView(2.0f, 60.0f, 0.1f, 400.0f);
}

void Game::Render() const
{
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
// 	float cellHeight = 0.0f;
// 	float textWdith = 0.0f;
	std::vector<Vertex_PCU> textVerts;

	switch (m_gameState)
	{
	case ATTRACT:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		g_theRenderer->BeginCamera(m_attractCamera);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		SplashScreen();

		//DebugRenderWorld(m_attractCamera);

		g_theRenderer->EndCamera(m_attractCamera);

		g_theRenderer->BeginCamera(m_screenCamera);

		//DebugRenderScreen(m_screenCamera);

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
//		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		g_theRenderer->ClearScreen(m_skyColor);

		GameConstants gameConsts;
		gameConsts.camWorldPos[0] = m_worldCamera.GetPosition().x;			// Used for fog thickness calculations, specular lighting, etc.
		gameConsts.camWorldPos[1] = m_worldCamera.GetPosition().y;
		gameConsts.camWorldPos[2] = m_worldCamera.GetPosition().z;
		gameConsts.camWorldPos[3] = 0.0f;
		m_skyColor.GetAsFloats(gameConsts.skyColor);						// Also used as fog color
		m_outdoorLightColor.GetAsFloats(gameConsts.outdoorLightColor);		// Used for outdoor lighting exposure
		m_indoorLightColor.GetAsFloats(gameConsts.indoorLightColor);		// Used for outdoor lighting exposure
		gameConsts.fogStartDist = m_fogStartDist;							// Fog has zero opacity at or before this distance
		gameConsts.fogEndDist = m_fogEndDist;								// Fog has maximum opacity at or beyond this distance
		gameConsts.fogMaxAlpha = m_foxMaxAlpha;								// At and beyond fogEndDist, fog gets this much opacity
		gameConsts.time = m_world->m_timeOfDay;								// CBO structs and members must be 16B-aligned and 16B-sized!!

		g_theRenderer->CopyCPUToGPU(&gameConsts, sizeof(GameConstants), m_gameCBO);
		g_theRenderer->BindConstantBuffer(CAMERA_GAME_BUFFER_SLOT, m_gameCBO);

		g_theRenderer->BeginCamera(m_worldCamera); 
//		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
//		g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);

		RenderEntities();

		if (m_isDebug)
		{
			DebugRender();
		}
		if (g_systemClock.IsPaused())
		{
			std::vector<Vertex_PCU> pauseVerts;
			g_theRenderer->BindTexture(nullptr); // disable texturing while drawing overlay
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);

			AABB2 bounds = AABB2(m_worldCamera.GetOrthoBottomLeft(), m_worldCamera.GetOrthoTopRight());
			AddVertsForAABB2D(pauseVerts, bounds, Rgba8(0, 0, 0, 96));

			g_theRenderer->DrawVertexArray(static_cast<int>(pauseVerts.size()), pauseVerts.data());
		}

		DebugRenderWorld(m_worldCamera);

		g_theRenderer->EndCamera(m_worldCamera);

		g_theRenderer->BeginCamera(m_screenCamera);
		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		//RenderLives();

		// render screen icon bar
		if (true)
		{
			AABB2 bounds;
			bounds = AABB2(Vec2(10.0f, 10.0f), Vec2(60.0f, 60.0f));
			AABB2 uv;
			// selected block to place
			uv = BlockDefinition::s_definitions[m_world->m_player->m_blockSelection].m_side_uvs;
			ShowIcon(bounds, "Data/Images/BasicSprites_64x64.png", uv);
			// current physics mode
			bounds = AABB2(Vec2(70.0f, 10.0f), Vec2(120.0f, 60.0f));
			uv = AABB2::ZERO_TO_ONE;
			switch (m_world->m_player->m_physics)
			{
			case NOCLIP:
				ShowIcon(bounds, "Data/Images/noclip.png", uv);
				break;
			case WALKING:
				ShowIcon(bounds, "Data/Images/walking.png", uv);
				break;
			case FLYING:
				ShowIcon(bounds, "Data/Images/flying.png", uv);
				break;
			}
			// camera view
			bounds = AABB2(Vec2(130.0f, 10.0f), Vec2(180.0f, 60.0f));
			uv = AABB2(Vec2(0.125f * (float)m_world->m_player->m_cameraView, 0.875f), Vec2(0.125f * (float)(m_world->m_player->m_cameraView + 1), 1.0f));
			ShowIcon(bounds, "Data/Images/TestUV.png", uv);
			bounds = AABB2(Vec2(145.0f, 10.0f), Vec2(180.0f, 45.0f));
			uv = AABB2::ZERO_TO_ONE;
			ShowIcon(bounds, "Data/Images/camera.png", uv);
		}

		DebugRenderScreen(m_screenCamera);

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

	case TEST:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
		g_theRenderer->BindTexture(nullptr); // disable texturing while drawing map tiles
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
//		g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);

		g_theRenderer->BeginCamera(m_screenCamera);

		DrawTestGame();
		g_theRenderer->EndCamera(m_screenCamera);
		break;

	default:
// 		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
// 		g_theRenderer->BeginCamera(m_worldCamera);
// 		cellHeight = 160.0f;
// 		str = "Unknown Game State!";
// 		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
// 		AddVertsForTextTriangles2D(vertexArray, str, Vec2((g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f) - textWdith) * 0.5f, (g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f) - cellHeight) * 0.5f), cellHeight, Rgba8(255, 0, 0, 255));
// 		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
// 		g_theRenderer->EndCamera(m_screenCamera);
		break;
	}
}

void Game::RenderEntities() const
{
	if (m_world)
	{
		m_world->Render();
	}
}

void Game::DebugRender() const
{
	// Draw entity debug info
}

void Game::Shutdown()
{
	if (m_world)
	{
		delete m_world;
		m_world = nullptr;
	}

	if (m_gameCBO)
	{
		delete m_gameCBO;
		m_gameCBO = nullptr;
	}
}

void Game::PlayAttractMusic()
{
	if (g_theAudio->IsPlaying(m_soundPlaybackID[GAMEPLAYMUSIC]))
		g_theAudio->StopSound(m_soundPlaybackID[GAMEPLAYMUSIC]);
	if (g_theAudio->IsPlaying(m_soundPlaybackID[ATTRACTMUSIC]))
		return;
	m_soundPlaybackID[ATTRACTMUSIC] = g_theAudio->StartSound(m_soundID[ATTRACTMUSIC]);
}

void Game::PlayGameMusic()
{
	if (g_theAudio->IsPlaying(m_soundPlaybackID[ATTRACTMUSIC]))
		g_theAudio->StopSound(m_soundPlaybackID[ATTRACTMUSIC]);
	if (g_theAudio->IsPlaying(m_soundPlaybackID[GAMEPLAYMUSIC]))
		return;
	m_soundPlaybackID[GAMEPLAYMUSIC] = g_theAudio->StartSound(m_soundID[GAMEPLAYMUSIC]);
}

void Game::SetSoundPlaybackParameters(float volume)
{
	float playbackSpeed = 1.0f;
	if (g_systemClock.IsPaused())
	{
		playbackSpeed = 0.0f;
	}
	else
		if (g_systemClock.GetTimeDilation() > 1.1f)
		{
			playbackSpeed = 1.5f;
		}
		else
			if (g_systemClock.GetTimeDilation() < 0.9f)
			{
				playbackSpeed = 0.7f;
			}
	for (int index = 0; index < SOUND_COUNT; index++)
	{
		if (m_soundPlaybackID[index] == MISSING_SOUND_ID)
		{
			continue;
		}
		g_theAudio->SetSoundPlaybackSpeed(m_soundPlaybackID[index], playbackSpeed);
		g_theAudio->SetSoundPlaybackVolume(m_soundPlaybackID[index], volume);
	}
}

void Game::SplashScreen() const
{
	Rgba8 tintColor(255, 255, 255, 255);
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoBottomLeft().x, m_attractCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoTopRight().x, m_attractCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoBottomLeft().x, m_attractCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoBottomLeft().x, m_attractCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoTopRight().x, m_attractCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(m_attractCamera.GetOrthoTopRight().x, m_attractCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw test texture
// 	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/AttractScreen.png");
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/AttractScreen.png"));
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	//g_theRenderer->BindTexture(nullptr);
}

void Game::ShowIcon(AABB2 bounds, char const* iconFilePath, AABB2 uv) const
{
//	Rgba8 tintColor(255, 255, 255, static_cast<uint8_t>(127.0 + 127.0 * sin(GetCurrentTimeSeconds() + 1.0)));
	Rgba8 tintColor(255, 255, 255, 255);
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), tintColor, uv.m_mins);
	vertex[1] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), tintColor, uv.m_maxs);
	vertex[2] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.0f), tintColor, Vec2(uv.m_mins.x, uv.m_maxs.y));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), tintColor, uv.m_mins);
	vertex[4] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.0f), tintColor, Vec2(uv.m_maxs.x, uv.m_mins.y));
	vertex[5] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), tintColor, uv.m_maxs);

	// Draw test texture
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile(iconFilePath);
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(testTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(nullptr);
}

void Game::TestSprites() const
{
	// Initialization
	int spriteIndex = 5;
	Texture* testTexture_8x2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/TestUV.png");
	SpriteSheet* testSpriteSheet = new SpriteSheet(*testTexture_8x2, IntVec2(8, 8));
	const SpriteDefinition& testSpriteDef = testSpriteSheet->GetSpriteDefinition(spriteIndex);

	// Drawing
	Rgba8 spriteTint(255, 255, 255, 255);
	AABB2 spriteBounds = AABB2(170.0f, 10.0f, 190.0f, 50.0f);
	Vec2 uvAtMins, uvAtMaxs;
	testSpriteDef.GetUVs(uvAtMins, uvAtMaxs);
	std::vector<Vertex_PCU> testVerts;
	AddVertsForAABB2D(testVerts, spriteBounds, spriteTint, uvAtMins, uvAtMaxs);
	//TransformVertexArray();
	g_theRenderer->BindTexture(&testSpriteDef.GetTexture());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(static_cast<int>(testVerts.size()), testVerts.data());
	g_theRenderer->BindTexture(nullptr); // disables texturing in OpenGL (for now) bad hack to disable except here
}

void Game::DrawSquare(AABB2 bounds, Rgba8 color) const
{
	AABB2 uv = AABB2::ZERO_TO_ONE;
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), color, uv.m_mins);
	vertex[1] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), color, uv.m_maxs);
	vertex[2] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_maxs.y, 0.0f), color, Vec2(uv.m_mins.x, uv.m_maxs.y));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(bounds.m_mins.x, bounds.m_mins.y, 0.0f), color, uv.m_mins);
	vertex[4] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_mins.y, 0.0f), color, Vec2(uv.m_maxs.x, uv.m_mins.y));
	vertex[5] = Vertex_PCU(Vec3(bounds.m_maxs.x, bounds.m_maxs.y, 0.0f), color, uv.m_maxs);

	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(nullptr);
}

void Game::DrawTestGame() const
{
	constexpr float ybase = 50.0f;
	constexpr float xbase = 200.0f;
	constexpr float square = 30.0f;

	float vertical = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
	char pbuffer[80];
	sprintf_s(pbuffer, "A: retrieve one job, S: retrieve all jobs, D: Complete all jobs, N: exit");
	DebugAddScreenText(pbuffer, Vec2(0.0f, vertical - 1.1f * 20.0f), 0.0f, Vec2(0.0f, 1.0f), 20.0f, Rgba8::GOLD, Rgba8::GOLD);
	DebugRenderScreen(m_screenCamera);

	for (int row = 0; row < 19; row++)
	{
		for (int col = 0; col < 39; col++)
		{
			int index = row * 40 + col;
			Rgba8 color = Rgba8::GRAY;
			float fraction = 1.0f;
			switch (m_testJobs[index]->m_state)
			{
			case JobState::QUEUED:
				color = Rgba8::RED;
				break;
			case JobState::PROCESSING:
				color = Rgba8::YELLOW;
				fraction = dynamic_cast<TestJob*>(m_testJobs[index])->GetPercent();
				break;
			case JobState::COMPLETE:
				color = Rgba8::GREEN;
				break;
			case JobState::RETIRED:
				color = Rgba8::BLUE;
				break;
			}
			Vec2 ll(xbase + (float)col * square, ybase + (float)(20 - row) * square);
			Vec2 ur(xbase + (float)(col + 1) * square, ybase + (float)(20 - row + fraction) * square);
			DrawSquare(AABB2(ll, ur), color);
		}
	}
}