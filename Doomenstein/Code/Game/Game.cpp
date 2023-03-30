#include "Game/Game.hpp"
#include "Game/App.hpp"
#include "Game/Player.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Renderer/SpriteAnimationDefinition.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Game/MapDefinition.hpp"
#include "Game/WeaponDefinition.hpp"

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
	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (m_player[index])
		{
			delete m_player[index];
			m_player[index] = nullptr;
		}
	}
}

Game::Game()
{
	g_theEventSystem->SubscribeEventCallbackFunction( "test", Command_Test );

	// Load the test font for testing
	g_testFont = g_theRenderer->CreateOrGetBitmapFont( "Data/Fonts/MyFixedFont" ); // DO NOT SPECIFY FILE EXTENSION!!  (Important later on.)

	// load all assets for the game here
	m_textures.reserve(TEXTURE_COUNT);
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Terrain_8x8.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/AttractScreen.jpg"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Actor_Marine_7x9.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Actor_Pinky_8x9.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Projectile_PistolHit.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Projectile_BloodSplatter.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Plasma.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Weapon_Pistol.png"));
	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Weapon_Plasma.png"));
//	m_textures.push_back(g_theRenderer->CreateOrGetTextureFromFile("Data/Images/"));

	for (int index = 0; index < TEXTURE_COUNT; index++)
	{
	}

//	m_explosionSpriteSheet = new SpriteSheet(*m_textures[TEXTURE_EXPLOSION], IntVec2(5, 5));
	m_terrainSpriteSheet = new SpriteSheet(*m_textures[TEXTURE_TERRAIN], IntVec2(8, 8));

	m_soundID.reserve(SOUND_COUNT);
	m_soundID.push_back(g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("mainMenuMusic", "Data/Audio/Music/MainMenu_InTheDark.mp2")));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Click.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound(g_gameConfigBlackboard.GetValue("gameMusic", "Data/Audio/Music/E1M1_AtDoomsGate.mp2")));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Pause.mp3"));
	m_soundID.push_back(g_theAudio->CreateOrGetSound("Data/Audio/Unpause.mp3"));

	for (int index = 0; index < SOUND_COUNT; index++)
	{
		m_soundPlaybackID.push_back(0); // create an array of null values
	}

	m_volume = g_gameConfigBlackboard.GetValue("musicVolume", 0.0125f);
}

void Game::Startup()
{
	if (m_gameState == ATTRACT)
	{
		m_numPlayers = 0;
		for (int index = 0; index < g_maxPlayers; index++)
		{
			if (m_player[index])
			{
				delete m_player[index];
				m_player[index] = nullptr;
			}
		}
		m_isDebug = false; // only meaningful in playing modes
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_WIDTH", 200.0f), g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_HEIGHT", 100.0f)));
	}
	else
	{
		m_gameState = PLAYING; // any state but attract comes here and will be play
// 		m_worldCamera[0].SetOrthoView(Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f));
// 		m_worldCamera[0].SetRenderTransform(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
// 		m_worldCamera[1].SetOrthoView(Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f));
// 		m_worldCamera[1].SetRenderTransform(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
		//m_worldCamera.SetRenderTransform(Vec3(1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f), Vec3(0.0f, 0.0f, 1.0f));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f), g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f)));
		
// 		// static graphics elements
// 		DebugRenderClear();
// 		DebugAddWorldBasis(Mat44(), -1.0f, Rgba8::WHITE, Rgba8::WHITE, DebugRenderMode::USEDEPTH);
// 
// 		Mat44 orientation = Mat44();
// 		orientation.AppendXRotation(90.0);
// 		orientation.AppendTranslation3D(Vec3(0.25f, 0.25f, 0.0f));
// 		DebugAddWorldText("x - forward", orientation, 0.3f, Vec2(0.5f, 0.5f), -1.0f, Rgba8::RED, Rgba8::RED, DebugRenderMode::USEDEPTH);
// 
// 		orientation = Mat44();
// 		orientation.AppendZRotation(90.0);
// 		orientation.AppendTranslation3D(Vec3(0.25f, 0.25f, 0.0f));
// 		DebugAddWorldText("y - left", orientation, 0.3f, Vec2(0.5f, 0.5f), -1.0f, Rgba8::GREEN, Rgba8::GREEN, DebugRenderMode::USEDEPTH);
// 
// 		orientation = Mat44();
// 		orientation.AppendYRotation(-90.0);
// 		orientation.AppendTranslation3D(Vec3(0.25f, 0.25f, 0.0f));
// 		DebugAddWorldText("z - up", orientation, 0.3f, Vec2(0.5f, 0.5f), -1.0f, Rgba8::BLUE, Rgba8::BLUE, DebugRenderMode::USEDEPTH);

		CreateMap();
	}

//	m_player = new Player(Vec3(1.5f, 1.5f, 0.5f), EulerAngles(0.0f, 0.0f, 0.0f));

	// draw grid lines
// 	float width = 0.01f;
// 	for (int index = 0; index < 101; index++)
// 	{
// 		float offset = 50.0f - static_cast<float>(index);
// 		m_x[index] = new Line3D(Vec3(0.0f, offset, 0.0f), EulerAngles(0.0f, 90.0f, 0.0f));
// 		m_x[index]->Create(50.0f, (index == 50) ? Rgba8::WHITE : (index % 5) ? Rgba8::GRAY : Rgba8::RED, (index % 5) ? width : 3 * width, 16);
// 	}
// 	for (int index = 0; index < 101; index++)
// 	{
// 		float offset = 50.0f - static_cast<float>(index);
// 		m_y[index] = new Line3D(Vec3(offset, 0.0f, 0.0f), EulerAngles(0.0f, 0.0f, 90.0f));
// 		m_y[index]->Create(50.0f, (index == 50) ? Rgba8::WHITE : (index % 5) ? Rgba8::GRAY : Rgba8::GREEN, (index % 5) ? width : 3 * width, 16);
// 	}

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
// F9 : sound volume down
// F11 : sound volume up
// F8 : restart game

void Game::Update(float deltaSeconds)
{
// 	float fontSize = 20.0f;
// 	float vertical = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
	Vec3 pos;

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
		case LOBBY:
			m_gameState = LOBBY;
			break;
		case RESTART:
			m_nextState = PLAYING; // reset to avoid churn and then fall through
		case PLAYING:
			Shutdown();
			m_gameState = PLAYING;
			Startup();
//			g_theGame->m_soundPlaybackID[WELCOME] = g_theAudio->StartSound(g_theGame->m_soundID[WELCOME]);
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
	else // any game play mode (including LOBBY)
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
	if (g_theInput->WasKeyJustPressed(KEYCODE_F9))
	{
		m_volume *= 0.5f;
		m_volume = Clamp(m_volume, 0.0f, 1.0f);
		g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F11))
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
		if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE))
		{
			JoinPlayer((int)ControlType::MOUSE_KEYBOARD);
			m_nextState = LOBBY;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::START))
		{
			JoinPlayer((int)ControlType::CONTROLLER);
			m_nextState = LOBBY;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			m_isQuitting = true;  // pressing Q tell the program to quit
			Shutdown();
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		// update camera last
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_WIDTH", 200.0f), g_gameConfigBlackboard.GetValue("ATTRACT_CAMERA_HEIGHT", 100.0f)));
		break;

	case LOBBY:
		PlayAttractMusic();
		if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE))
		{
			Player* player = GetJoinedPlayer((int)ControlType::MOUSE_KEYBOARD);
			if (player)
			{
				InitializePlayers();
				m_nextState = PLAYING;
			}
			else
			{
				JoinPlayer((int)ControlType::MOUSE_KEYBOARD);
			}
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::START))
		{
			Player* player = GetJoinedPlayer((int)ControlType::CONTROLLER);
			if (player)
			{
				InitializePlayers();
				m_nextState = PLAYING;
			}
			else
			{
				JoinPlayer((int)ControlType::CONTROLLER);
			}
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
		{
			Player* player = GetJoinedPlayer((int) ControlType::MOUSE_KEYBOARD);
			if (player)
			{
				RemovePlayer(player->m_playerIndex);
			}
			if (GetNumPlayers() == 0)
			{
				m_nextState = ATTRACT;
			}
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			Player* player = GetJoinedPlayer((int)ControlType::CONTROLLER);
			if (player)
			{
				RemovePlayer(player->m_playerIndex);
			}
			if (GetNumPlayers() == 0)
			{
				m_nextState = ATTRACT;
			}
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}
		// Update screen camera
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f), g_gameConfigBlackboard.GetValue("SCREEn_CAMERA_HEIGHT", 800.0f)));
		break;

	case PLAYING:
		PlayGameMusic();
		if (g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			if (g_systemClock.IsPaused())
			{
				m_nextState = ATTRACT;
			}
		}

		// Only 'P' will resume game because ESC and BACK will leave in test above
		if (g_theInput->WasKeyJustPressed('P') || g_theInput->WasKeyJustPressed(KEYCODE_ESC) || controller.WasButtonJustPressed(XboxButtonID::BACK))
		{
			g_theGame->m_soundPlaybackID[g_systemClock.IsPaused() ? PAUSE : UNPAUSE] = g_theAudio->StartSound(g_theGame->m_soundID[g_systemClock.IsPaused() ? PAUSE : UNPAUSE]);
			g_systemClock.TogglePause();
		}

		// lighting adjustment
		float duration = 4.0f;
		DebugRenderSetVisible();

		if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
		{
			std::string text = "Sun Pitch: ";
			text += std::to_string(g_theRenderer->m_sunDirection.m_pitchDegrees);
			text += " F2/F3 to change\n";
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			text = "Sun Intensity: ";
			text += std::to_string(g_theRenderer->m_sunIntensity);
			text += " F4/F5 to change\n";
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			text = "Sun Intensity: ";
			text += std::to_string(g_theRenderer->m_sunIntensity);
			text += " F6/F7 to change\n";
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F2))
		{
			g_theRenderer->ChangeSunDirection(-15.0f);
			std::string text = "Sun Pitch: "; 
			text += std::to_string(g_theRenderer->m_sunDirection.m_pitchDegrees);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
		{
			g_theRenderer->ChangeSunDirection(15.0f);
			std::string text = "Sun Pitch: ";
			text += std::to_string(g_theRenderer->m_sunDirection.m_pitchDegrees);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
		{
			g_theRenderer->ChangeSunIntensity(-0.1f);
			std::string text = "Sun Intensity: ";
			text += std::to_string(g_theRenderer->m_sunIntensity);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
		{
			g_theRenderer->ChangeSunIntensity(0.1f);
			std::string text = "Sun Intensity: ";
			text += std::to_string(g_theRenderer->m_sunIntensity);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
		{
			g_theRenderer->ChangeAmbientIntensity(-0.1f);
			std::string text = "Ambient Intensity: ";
			text += std::to_string(g_theRenderer->m_ambientIntensity);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
		{
			g_theRenderer->ChangeAmbientIntensity(0.1f);
			std::string text = "Ambient Intensity: ";
			text += std::to_string(g_theRenderer->m_ambientIntensity);
			DebugAddMessage(text, duration, Rgba8::WHITE, Rgba8::WHITE);
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		// update
		UpdateEntities(static_cast<float>(g_systemClock.GetDeltaTime()));

		if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
		{
			m_nextState = RESTART;
			g_theGame->m_soundPlaybackID[CLICK] = g_theAudio->StartSound(g_theGame->m_soundID[CLICK]);
		}

		UpdateCameras(deltaSeconds);
		break;
	}
}

void Game::UpdateEntities(float deltaSeconds)
{
	m_map->Update(deltaSeconds);

	for (int index = 0; index < 101; index++)
	{
		if (m_x[index])
		{
			m_x[index]->Update(deltaSeconds);
		}
	}
	for (int index = 0; index < 101; index++)
	{
		if (m_y[index])
		{
			m_y[index]->Update(deltaSeconds);
		}
	}
}

void Game::UpdateCameras(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Game::Render() const
{
//	float screenHeight = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
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
 		//TestTexture();
		AdvisoryTexture();
		//TestSprites();


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

	case LOBBY:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
		g_theRenderer->BeginCamera(m_screenCamera);
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);

//		std::string str;
//		std::vector<Vertex_PCU> vertexArray;
//		std::vector<Vertex_PCU> textVerts;

		if (m_player[0])
		{
			if (m_player[0]->m_controllerIndex == (int)ControlType::CONTROLLER)
			{
				ControllerPlayerInfo(0, 100.0f);
			}
			else
			{
				KeyboardPlayerInfo(0, 100.0f);
			}
		}
		
		if (m_player[1])
		{
			if (m_player[1]->m_controllerIndex == (int)ControlType::CONTROLLER)
			{
				ControllerPlayerInfo(1, 800.0f);
			}
			else
			{
				KeyboardPlayerInfo(1, 800.0f);
			}
		}

// 		TestTexture();

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

	case PLAYING:
		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
//-------------------------------------------------------------------------
		for (int index = 0; index < m_numPlayers; index++)
		{
			if (m_player[index] == nullptr)
			{
				continue;
			}
			m_renderingPlayer = m_player[index]; // used to test who is rendering
			g_theRenderer->BeginCamera(*m_player[index]->m_cameraWorld);
			g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
			g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);

			RenderEntities();

			if (m_isDebug)
			{
				DebugRender();
			}

			DebugRenderWorld(*m_player[index]->m_cameraWorld);

			g_theRenderer->EndCamera(*m_player[index]->m_cameraWorld);
		}
//---------------------------------------------------------------------------
		g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
		g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
		//RenderLives();
		if (g_systemClock.IsPaused())
		{
			std::vector<Vertex_PCU> pauseVerts;
			g_theRenderer->BindTexture(nullptr); // disable texturing while drawing map tiles
			g_theRenderer->SetBlendMode(BlendMode::ALPHA);

			AABB2 bounds = AABB2(m_screenCamera.GetOrthoBottomLeft(), m_screenCamera.GetOrthoTopRight());
			AddVertsForAABB2D(pauseVerts, bounds, Rgba8(0, 0, 0, 96));

			g_theRenderer->DrawVertexArray(static_cast<int>(pauseVerts.size()), pauseVerts.data());
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
	}
}

void Game::RenderEntities() const
{
	for (int index = 0; index < 101; index++)
	{
		if (m_x[index])
		{
			m_x[index]->Render();
		}
	}
	for (int index = 0; index < 101; index++)
	{
		if (m_y[index])
		{
			m_y[index]->Render();
		}
	}

	// map textured on a box to test
// 	std::vector<Vertex_PCU> vertexArray;
// 	AddVertsForAABB3D(vertexArray, AABB3(Vec3::ZERO, Vec3(5.0f, 5.0f, 5.0f)));
// 
// 	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Maps/TestMap.png");
// 	g_theRenderer->BindTexture(testTexture);
// 	g_theRenderer->SetModelMatrix(Mat44());
// 	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
// 	vertexArray.clear();

	if (m_map)
	{
		m_map->Render();
	}
}

void Game::DebugRender() const
{
	// Draw entity debug info
}

void Game::Shutdown()
{
	for (int index = 0; index < 101; index++)
	{
		if (m_x[index])
		{
			delete m_x[index];
			m_x[index] = nullptr;
		}
	}
	for (int index = 0; index < 101; index++)
	{
		if (m_y[index])
		{
			delete m_y[index];
			m_y[index] = nullptr;
		}
	}

// 	for (int index = 0; index < g_maxPlayers; index++)
// 	{
// 		if (m_player[index])
// 		{
// 			delete m_player[index];
// 			m_player[index] = nullptr;
// 		}
// 	}

	if (m_gameMap)
	{
		delete m_gameMap;
		m_gameMap = nullptr;
	}	

	if (m_map)
	{
		delete m_map;
		m_map = nullptr;
	}
}

void Game::JoinPlayer(int controllerIndex)
{
	if ((controllerIndex < -1) && (controllerIndex > 3))
	{
		return;
	}

	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (m_player[index] == nullptr)
		{
			AddPlayer(index, controllerIndex);
			break;
		}
	}
}

Player* Game::GetJoinedPlayer(int controllerIndex)
{
	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (m_player[index] && m_player[index]->m_controllerIndex == controllerIndex)
		{
			return m_player[index];
		}
	}
	return nullptr; // player does not exist
}

void Game::AddPlayer(int playerIndex, int controllerIndex)
{
	m_numPlayers++; // should never be a problem if code is written right
	m_player[playerIndex] = new Player();
	m_player[playerIndex]->m_playerIndex = playerIndex;
	m_player[playerIndex]->m_controllerIndex = controllerIndex;
}

void Game::RemovePlayer(int playerIndex)
{
	m_numPlayers--;
	if (m_numPlayers < 0)
	{
		ERROR_AND_DIE("Less than zero players!");
	}
	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (m_player[index] && m_player[index]->m_playerIndex == playerIndex)
		{
			delete m_player[index];
			m_player[index] = nullptr;
		}
	}
}

void Game::SetPlayerViewports()
{

}

Player* Game::GetPlayer(int playerIndex) const
{
	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (m_player[index] && m_player[index]->m_playerIndex == playerIndex)
		{
			return m_player[index];
		}
	}
	return nullptr;
}

Player* Game::GetRenderingPlayer() const
{
	return m_renderingPlayer;
}

int Game::GetNumPlayers() const
{
	return m_numPlayers;
}

void Game::InitializePlayers()
{
	if (m_numPlayers == 2)
	{
		SetCameras(0);
		SetCameras(1);
		g_theAudio->SetNumListeners(2);
	}
	else
	{
		if (m_player[0])
		{
			SetCameras(0);
		}
		else
		{
			SetCameras(1);
		}
		g_theAudio->SetNumListeners(1);
	}
}

void Game::SetCameras(int playerNum)
{
	m_player[playerNum]->m_cameraWorld = new Camera();
	m_player[playerNum]->m_cameraHud = new Camera();
	m_player[playerNum]->m_cameraWorld->SetOrthoView(Vec2(-1.0f, -1.0f), Vec2(1.0f, 1.0f));
	m_player[playerNum]->m_cameraWorld->SetRenderTransform(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));
	m_player[playerNum]->m_cameraHud->SetOrthoView(Vec2(0.0f, 0.0f), Vec2(g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_WIDTH", 1600.0f), g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f)));
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
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/AttractScreen.jpg");
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

void Game::DeathScreen() const
{
	Rgba8 tintColor(192, 192, 192, 255);
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoBottomLeft().x, m_screenCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(m_screenCamera.GetOrthoTopRight().x, m_screenCamera.GetOrthoTopRight().y, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw texture
	Texture* splashTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/YouDiedScreen.png");
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(splashTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(nullptr); // disables texturing in OpenGL (for now) bad hack to disable except here
}

void Game::TestTexture() const
{
	Rgba8 tintColor(255, 255, 255, static_cast<unsigned char>(127.0 + 127.0 * sin(GetCurrentTimeSeconds() + 1.0)));
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(300.0f, 50.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(600.0f, 350.0f, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(300.0f, 350.0f, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(300.0f, 50.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(600.0f, 50.0f, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(600.0f, 350.0f, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw test texture
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Test_StbiFlippedAndOpenGL.png");
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

void Game::AdvisoryTexture() const
{
	Rgba8 tintColor(255, 255, 255, static_cast<unsigned char>(127.0 + 127.0 * cos(GetCurrentTimeSeconds() + 1.0)));
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(165.0f, 5.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(195.0f, 25.0f, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(165.0f, 25.0f, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(165.0f, 5.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(195.0f, 5.0f, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(195.0f, 25.0f, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw test texture
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/parental-advisory.png");
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(testTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	//g_theRenderer->BindTexture(nullptr);
}

void Game::TestSprites() const
{
	// Initialization
	int spriteIndex = 5;
	Texture* testTexture_8x2 = g_theRenderer->CreateOrGetTextureFromFile("Data/Images/Actor_Pinky_8x9.png");
	SpriteSheet* testSpriteSheet = new SpriteSheet(*testTexture_8x2, IntVec2(8, 9));
	const SpriteDefinition& testSpriteDef = testSpriteSheet->GetSpriteDefinition(spriteIndex);

	// Drawing
	Rgba8 spriteTint(Rgba8::WHITE);
	AABB2 spriteBounds = AABB2(160.0f, 60.0f, 190.0f, 90.0f);
	Vec2 uvAtMins, uvAtMaxs;
	testSpriteDef.GetUVs(uvAtMins, uvAtMaxs);
	std::vector<Vertex_PCU> testVerts;
	AddVertsForAABB2D(testVerts, spriteBounds, spriteTint, uvAtMins, uvAtMaxs);
	//TransformVertexArray();
	g_theRenderer->BindTexture(&testSpriteDef.GetTexture());
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray(static_cast<int>(testVerts.size()), testVerts.data());
	g_theRenderer->BindTexture(nullptr);
}

void Game::CreateMap()
{
	ActorDefinition::InitializeDefinitions("Data/Definitions/ProjectileActorDefinitions.xml");
	WeaponDefinition::InitializeDefinitions("Data/Definitions/WeaponDefinitions.xml");
	ActorDefinition::InitializeDefinitions("Data/Definitions/ActorDefinitions.xml");

	MapDefinition::InitializeDefinitions();
	std::string mapName = g_gameConfigBlackboard.GetValue("DEFAULT_MAP_NAME", "Data/Fonts/MyFixedFont");
	m_map = new Map(this, MapDefinition::GetByName(mapName));
}

void Game::KeyboardPlayerInfo(int playerNum, float printOffset) const
{
	float screenHeight = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
	float row0 = 200.0f;
	std::string str = "Player " + std::to_string(playerNum + 1);
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;
	  
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0), 80.f, str, Rgba8::RED, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 1.0f), 40.f, "Mouse and Keyboard", Rgba8::GREEN, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 2.0f), 40.f, "WASD movement", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 3.0f), 40.f, "1/2/3 select weapons", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 4.0f), 40.f, "</> swap weapons", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 5.0f), 40.f, "L mouse fire weapon", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 6.0f), 40.f, "R mouse alt attack", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 7.0f), 40.f, "ESC pause/exit", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 8.0f), 40.f, "SPACE to start game", Rgba8::GOLD, 0.6f);
	if (m_numPlayers == 1)
	{
		g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 9.0f), 40.f, "START to join player 2", Rgba8::GOLD, 0.6f);
	}

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
}

void Game::ControllerPlayerInfo(int playerNum, float printOffset) const
{
	float screenHeight = g_gameConfigBlackboard.GetValue("SCREEN_CAMERA_HEIGHT", 800.0f);
	float row0 = 200.0f;
	std::string str = "Player " + std::to_string(playerNum + 1);
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0), 80.f, str, Rgba8::RED, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 1.0f), 40.f, "Xbox Controller", Rgba8::GREEN, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 2.0f), 40.f, "L stick movement", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 3.0f), 40.f, "X/Y/B select weapons", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 4.0f), 40.f, "L/R bumper swap weapons", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 5.0f), 40.f, "R trigger fire weapon", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 6.0f), 40.f, "L trigger alt attack", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 7.0f), 40.f, "BACK pause/exit", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 8.0f), 40.f, "START to start game", Rgba8::GOLD, 0.6f);
	if (m_numPlayers == 1)
	{
		g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f + printOffset, screenHeight - row0 - 50.0f * 9.0f), 40.f, "SPACE to join player 2", Rgba8::GOLD, 0.6f);
	}

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
}