#include "Game/GameCommon.hpp"
#include "Game/App.hpp"
#include "Game/Game.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/Core/Time.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/SimpleTriangleFont.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Game.hpp"
#include "Curve.hpp"
#include "Engine/Math/Easing.hpp"
#include "CubicBezierCurve2D.hpp"
//#include "Engine/Renderer/Window.hpp"

RandomNumberGenerator random; // singleton for now only used by entities in the Game

Game::Game()
{
	g_testFont = g_theRenderer->CreateOrGetBitmapFont("Data/Fonts/MyFixedFont"); // DO NOT SPECIFY FILE EXTENSION!!  (Important later on.)
}

void Game::Startup()
{
	float aspect = 2.0f;
	g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative

	switch (m_gameState)
	{
	case ATTRACT:
		m_isDebug = false; // only meaningful in playing modes
		m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		break;

	case NEAREST_POINT_2D:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		CreateRandomShapes();
		break;

	case SPLINES:
		m_worldCamera1.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_worldCamera2.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_worldCamera3.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH * 2.0f, WORLD_CAMERA_HEIGHT));

		m_worldCamera1.SetViewport(AABB2(0.01f, 0.47f, 0.495f, 0.92f));
		m_worldCamera2.SetViewport(AABB2(0.505f, 0.47f, 0.99f, 0.92f));
		m_worldCamera3.SetViewport(AABB2(0.01f, 0.01f, 0.99f, 0.45f));

		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		m_bezier = CubicBezierCurve2D(GetRandomVec2ZeroToOne(), GetRandomVec2ZeroToOne(), GetRandomVec2ZeroToOne(), GetRandomVec2ZeroToOne());
		CreateHermiteSpline();
		m_selectedFunction = random.RollRandomIntInRange(0, 15);
		break;

	case RAYCAST_VS_DISCS:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		CreateRandomDiscs();
		break;

	case RAYCAST_VS_AABB2D:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		CreateRandomRectangles();
		break;

	case RAYCAST_VS_Lines:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		CreateRandomLineSegments();
		break;

	case RAYCAST_VS_OBB2:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		CreateRandomOBB2s();
		break;

	case BILLIARDS_2D:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		// camera defaults to depth testing false and backface culling false as needed for this mode
		CreateRandomBalls();
		break;

	case PACHENKO_2D:
		m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
		// camera defaults to depth testing false and backface culling false as needed for this mode
		CreatePachenko();
		m_balls = 0;
		break;

	case TEST_SHAPES_3D:
		m_nearestRaycast = NONE;
		m_nearestPoint = 0;
		m_selectedShape = NONE;
		m_selectedOffset = Vec3::ZERO;
		m_rayLock = false;

		aspect = static_cast<float>(WORLD_CAMERA_WIDTH) / static_cast<float>(WORLD_CAMERA_HEIGHT);
		m_worldCamera3D.SetPerspectiveView(aspect, 60.0f, 0.1f, 100.0f);  // zNear cannot be 0.0f
		m_worldCamera3D.SetRenderTransform(Vec3(0.0f, 0.0f, 1.0f), Vec3(-1.0f, 0.0f, 0.0f), Vec3(0.0f, 1.0f, 0.0f));

		m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));
// 		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
// 		g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		CreateRandom3DShapes();
		m_position = Vec3(-1.0f, -1.0f, 0.0f); // TEST DEBUG
		m_orientation = EulerAngles(45.0f, 0.0f, 0.0f);
		break;

		// renderer camera defaults
// 		SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
// 		BindShader(nullptr);
// 		SetModelColor(Rgba8::WHITE);
// 		SetSamplerMode(SamplerMode::POINTCLAMP);
// 		SetModelMatrix(Mat44());
// 		SetDepthStencilState(DepthTest::ALWAYS, false);
// 		SetBlendMode(BlendMode::ALPHA);

	}
}

void Game::Update(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();

	if (m_nextState != m_gameState)
	{
		switch (m_nextState)
		{
		case ATTRACT:
			Shutdown();
			m_gameState = ATTRACT;
			Startup();
			m_isPaused = false;
			break;

		case RESTART:
			Shutdown();
			m_nextState = m_gameState; // keep old game state
			Startup();
			break;

		case NEAREST_POINT_2D:
			Shutdown();
			m_gameState = NEAREST_POINT_2D;
			Startup();
			break;

		case SPLINES:
			Shutdown();
			m_gameState = SPLINES;
			Startup();
			break;

		case RAYCAST_VS_DISCS:
			Shutdown();
			m_gameState = RAYCAST_VS_DISCS;
			Startup();
			break;

		case RAYCAST_VS_AABB2D:
			Shutdown();
			m_gameState = RAYCAST_VS_AABB2D;
			Startup();
			break;

		case RAYCAST_VS_Lines:
			Shutdown();
			m_gameState = RAYCAST_VS_Lines;
			Startup();
			break;

		case RAYCAST_VS_OBB2:
			Shutdown();
			m_gameState = RAYCAST_VS_OBB2;
			Startup();
			break;

		case BILLIARDS_2D:
			Shutdown();
			m_gameState = BILLIARDS_2D;
			Startup();
			break;

		case PACHENKO_2D:
			Shutdown();
			m_gameState = PACHENKO_2D;
			Startup();
			break;

		case TEST_SHAPES_3D:
			Shutdown();
			m_gameState = TEST_SHAPES_3D;
			Startup();
			break;
		}
	}

	// this all needs to be changed to use the system clock!!!!!!!!!!!!!!!!!!!!!!!!!!
	if (g_theInput->IsKeyDown('T'))
	{
		m_isSlowMo = true; // holding T key causes slow motion
	}

	if (g_theInput->WasKeyJustPressed('P'))
	{
		m_isPaused = m_isPaused ? false : true; // pause key toggles state
	}

	if (g_theInput->WasKeyJustReleased('T'))
	{
		m_isSlowMo = false;  // slow motion ends when T is released
	}

	if (m_isSlowMo)
	{
		deltaSeconds *= 0.1f; // slow motion is one tenth as fast as normal
	}

	// set mouse state
	if (g_theWindow->HasFocus() == false)
	{
		g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
	}
	else if (m_gameState == TEST_SHAPES_3D)
	{
		g_theInput->SetMouseMode(true, true, true); // mouse hidden, clipped and relative
	}
	else // any other game play mode
	{
		g_theInput->SetMouseMode(false, false, false); // mouse visible and not clipped or relative
	}

	// state specific updates
	switch (m_gameState)
	{
	case ATTRACT:
		UpdateAttractMode(deltaSeconds);
		break;

	case NEAREST_POINT_2D:
		Update_NEAREST_POINT_2D_Mode(deltaSeconds);
		break;

	case SPLINES:
		Update_SPLINES_Mode(deltaSeconds);
		break;

	case RAYCAST_VS_DISCS:
		Update_RAYCAST_VS_DISCS_Mode(deltaSeconds);
		break;

	case RAYCAST_VS_AABB2D:
		Update_RAYCAST_VS_AABB2D_Mode(deltaSeconds);
		break;

	case RAYCAST_VS_Lines:
		Update_RAYCAST_VS_Lines_Mode(deltaSeconds);
		break;

	case RAYCAST_VS_OBB2:
		Update_RAYCAST_VS_OBB2_Mode(deltaSeconds);
		break;

	case BILLIARDS_2D:
		Update_BILLIARDS_2D_Mode(deltaSeconds);
		break;

	case PACHENKO_2D:
		Update_PACHENKO_2D_Mode(deltaSeconds);
		break;

	case TEST_SHAPES_3D:
		Update_TEST_SHAPES_3D_Mode(deltaSeconds);
		break;
	}
}

void Game::UpdateCameras(float deltaSeconds)
{
	m_worldCamera2D.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
	m_screenCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(SCREEN_CAMERA_WIDTH, SCREEN_CAMERA_HEIGHT));

	// add random variation if camera is shaking
	float xShake = random.RollRandomFloatInRange(-WORLD_CAMERA_WIDTH * shakeFraction, WORLD_CAMERA_WIDTH * shakeFraction);
	float yShake = random.RollRandomFloatInRange(-WORLD_CAMERA_HEIGHT * shakeFraction, WORLD_CAMERA_HEIGHT * shakeFraction);
	m_worldCamera2D.Translate2d(Vec2(xShake, yShake));
	if (shakeFraction > 0.0f)
	{
		shakeFraction -= WORLD_SCREEN_SHAKE_REDUCTION * deltaSeconds;
		Clamp(shakeFraction, 0.0f, WORLD_SCREEN_SHAKE_FRACTION);
	}
}

void Game::Render() const
{
	switch (m_gameState)
	{
	case ATTRACT:
		RenderAttractMode();
		break;

	case NEAREST_POINT_2D:
		Render_NEAREST_POINT_2D_Mode();
		break;

	case SPLINES:
		Render_SPLINES_Mode();
		break;

	case RAYCAST_VS_DISCS:
		Render_RAYCAST_VS_DISCS_Mode();
		break;

	case RAYCAST_VS_AABB2D:
		Render_RAYCAST_VS_AABB2D_Mode();
		break;

	case RAYCAST_VS_Lines:
		Render_RAYCAST_VS_Lines_Mode();
		break;

	case RAYCAST_VS_OBB2:
		Render_RAYCAST_VS_OBB2_Mode();
		break;

	case BILLIARDS_2D:
		Render_BILLIARDS_2D_Mode();
		break;

	case PACHENKO_2D:
		Render_PACHENKO_2D_Mode();
		break;

	case TEST_SHAPES_3D:
		Render_TEST_SHAPES_3D_Mode();
		break;

	default:
		std::string str;
		std::vector<Vertex_PCU> vertexArray;
		float cellHeight = 0.0f;
		float textWdith = 0.0f;
		std::vector<Vertex_PCU> textVerts;

		g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
		g_theRenderer->BeginCamera(m_worldCamera2D); 
		cellHeight = 160.0f;
		str = "Unknown Game State!";
		textWdith = GetSimpleTriangleStringWidth(str, cellHeight);
		AddVertsForTextTriangles2D(vertexArray, str, Vec2((SCREEN_CAMERA_WIDTH - textWdith) * 0.5f, (SCREEN_CAMERA_HEIGHT - cellHeight) * 0.5f), cellHeight, Rgba8(255, 0, 0, 255));
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		g_theRenderer->EndCamera(m_screenCamera);
		break;
	}
}

void Game::DebugRender() const
{
	// Draw entity debug info
}

void Game::Shutdown()
{
	for (Cueball* cueball : cueballs)
	{
		if (cueball)
		{
			delete cueball;
		}
		cueballs.clear();
	}

	for (Shapes3D* shape : m_shapes)
	{
		if (shape)
		{
			delete shape;
		}
		m_shapes.clear();
	}
}

// Update game state functions /////////////////////////////////////////////////////////////////////////////
void Game::UpdateAttractMode(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	XboxController controller = g_theInput->GetController(0); // leap of faith
	if (g_theInput->WasKeyJustReleased('1'))
	{
		m_nextState = GameState::NEAREST_POINT_2D;
	}

	if (g_theInput->WasKeyJustReleased('2'))
	{
		m_nextState = GameState::RAYCAST_VS_DISCS;
	}

	if (g_theInput->WasKeyJustReleased('3'))
	{
		m_nextState = GameState::RAYCAST_VS_AABB2D;
	}

	if (g_theInput->WasKeyJustReleased('4'))
	{
		m_nextState = GameState::RAYCAST_VS_Lines;
	}

	if (g_theInput->WasKeyJustReleased('5'))
	{
		m_nextState = GameState::RAYCAST_VS_OBB2;
	}

	if (g_theInput->WasKeyJustReleased('6'))
	{
		m_nextState = GameState::BILLIARDS_2D;
	}

	if (g_theInput->WasKeyJustReleased('7'))
	{
		m_nextState = GameState::PACHENKO_2D;
	}

	if (g_theInput->WasKeyJustReleased('8'))
	{
		m_nextState = GameState::TEST_SHAPES_3D;
	}

	if (g_theInput->WasKeyJustReleased('9'))
	{
		m_nextState = GameState::SPLINES;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_isQuitting = true;  // pressing Q tell the program to quit
		Shutdown();
	}

	// update camera last
	m_attractCamera.SetOrthoView(Vec2(0.0f, 0.0f), Vec2(WORLD_CAMERA_WIDTH, WORLD_CAMERA_HEIGHT));
}

void Game::Update_NEAREST_POINT_2D_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// Handle movement of the point on the screen
	if (g_theInput->IsKeyDown('W'))
	{
		m_testPoint.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_testPoint.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_testPoint.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_testPoint.x += deltaSeconds * m_moveScale;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = SPLINES;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = RAYCAST_VS_DISCS;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_SPLINES_Mode(float deltaSeconds)
{
	m_t += deltaSeconds * 0.5f;
	if (m_t >= 1.0f)
	{
		m_t -= 1.0f;
		m_segment++;
		if (m_segment >= (int)m_hermite.size())
		{
			m_segment = 0;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	m_showColors = g_theInput->IsKeyDown('C');

	// Handle movement of the point on the screen
	if (g_theInput->WasKeyJustPressed('W'))
	{
		m_selectedFunction -= 1;
		m_selectedFunction = m_selectedFunction < 0 ? FUNCTION_COUNT - 1 : m_selectedFunction;
	}

	if (g_theInput->WasKeyJustPressed('E'))
	{
		m_selectedFunction += 1;
		m_selectedFunction = m_selectedFunction >= FUNCTION_COUNT ? 0 : m_selectedFunction;
	}

	if (g_theInput->WasKeyJustPressed('N'))
	{
		if (m_numSubdivisions > 1)
		{
			m_numSubdivisions >>= 1;
		}
	}

	if (g_theInput->WasKeyJustPressed('M'))
	{
			m_numSubdivisions <<= 1;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = TEST_SHAPES_3D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = NEAREST_POINT_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_RAYCAST_VS_DISCS_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = NEAREST_POINT_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = RAYCAST_VS_AABB2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_RAYCAST_VS_AABB2D_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = RAYCAST_VS_DISCS;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = RAYCAST_VS_Lines;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_RAYCAST_VS_Lines_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = RAYCAST_VS_AABB2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = RAYCAST_VS_OBB2;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_RAYCAST_VS_OBB2_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	// update objects here

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = RAYCAST_VS_Lines;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = BILLIARDS_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_BILLIARDS_2D_Mode(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// check for cue ball poked
	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE))
	{
		Cueball* cueball = new Cueball(m_startPos, m_endPos - m_startPos, m_ballRadius, false);
		cueballs.push_back(cueball);
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	// update objects here
	for (int index = 0; index < static_cast<int>(cueballs.size()); index++)
	{
		cueballs[index]->UpdateBall(deltaSeconds);
		for (int bumper = 0; bumper < RAY_DISC_TOTAL; bumper++)
		{
			cueballs[index]->BounceOffBumperDisc(m_rayDiscCenters[bumper], m_rayDiscRadii[bumper], m_rayDiscElasticity[bumper]);
		}
	}

	for (int index = 0; index < static_cast<int>(cueballs.size()) - 1; index++)
	{
		for (int other = index + 1; other < cueballs.size(); other++)
		{
			cueballs[index]->BounceOffBall(*cueballs[other]);
		}
	}
	// end objects update

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = RAYCAST_VS_OBB2;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = PACHENKO_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::Update_PACHENKO_2D_Mode(float deltaSeconds)
{
	if (m_deltaSeconds == 0.0f)
	{
		m_deltaSeconds = deltaSeconds;
	}
	m_deltaSeconds = m_deltaSeconds * 0.97f + deltaSeconds * 0.03f; // save for screen print

	XboxController controller = g_theInput->GetController(0); // leap of faith
	AnalogJoystick leftJoystick = controller.GetLeftStick();
	// check joystick first because see next section behavior...
	if (leftJoystick.GetMagnitude() > 0.0f)
	{
		m_testPoint += leftJoystick.GetPosition() * deltaSeconds * m_moveScale;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F1))
	{
		m_isDebug = !m_isDebug; // toggle debug rendering
	}

	// check for cue ball creation
	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE))
	{
		Cueball* cueball = new Cueball(m_startPos, m_endPos - m_startPos, m_ballRadius  * random.RollRandomFloatInRange(0.5f, 1.5f), true);
		cueballs.push_back(cueball);
		m_balls++;
	}

	if (g_theInput->IsKeyDown('N'))
	{
		Cueball* cueball = new Cueball(m_startPos, m_endPos - m_startPos, m_ballRadius  * random.RollRandomFloatInRange(0.5f, 1.5f), true);
		cueballs.push_back(cueball);
		m_balls++;
	}

	if (g_theInput->WasKeyJustReleased('B'))
	{
		Cueball::s_bottomless = !Cueball::s_bottomless;
	}

	// Handle movement of the points on the screen
	//m_startPos
	if (g_theInput->IsKeyDown('W'))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('A'))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('S'))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('D'))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
	}

	// m_endPos
	if (g_theInput->IsKeyDown('I'))
	{
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('J'))
	{
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('K'))
	{
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown('L'))
	{
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// arrow keys
	if (g_theInput->IsKeyDown(KEYCODE_UPARROW))
	{
		m_startPos.y += deltaSeconds * m_moveScale;
		m_endPos.y += deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFTARROW))
	{
		m_startPos.x -= deltaSeconds * m_moveScale;
		m_endPos.x -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_DOWNARROW))
	{
		m_startPos.y -= deltaSeconds * m_moveScale;
		m_endPos.y -= deltaSeconds * m_moveScale;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHTARROW))
	{
		m_startPos.x += deltaSeconds * m_moveScale;
		m_endPos.x += deltaSeconds * m_moveScale;
	}

	// handle mouse clicks
	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_startPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_startPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	if (g_theInput->IsKeyDown(KEYCODE_RIGHT_MOUSE))
	{
		Vec2 mouseCoords = g_theWindow->GetNormalizedCursorPos();
		m_endPos.x = mouseCoords.x * WORLD_CAMERA_WIDTH;
		m_endPos.y = mouseCoords.y * WORLD_CAMERA_HEIGHT;
	}

	// handle single step
	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = false; // remove pause on the game if it is paused so that it will move
	}

	m_fwdNormal = (m_endPos - m_startPos).GetNormalized();
	m_maxDist = (m_endPos - m_startPos).GetLength();

	/////////////////////////////////////////////////////////////////
	// update objects here
	m_physicsTimeDebt += deltaSeconds;
	while (m_physicsTimeDebt > m_physicsTimeStep)
	{
		UpdatePachenkoPhysics(m_physicsTimeStep);
		m_physicsTimeDebt -= m_physicsTimeStep;
	}
	// end objects update
	/////////////////////////////////////////////////////////////////

	if (g_theInput->WasKeyJustPressed('O'))
	{
		m_isPaused = true; // always pause the game afterwards to single step
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F3))
	{
		m_physicsTimeStep -= 0.00005f;
		if (m_physicsTimeStep < 0.00005f)
		{
			m_physicsTimeStep = 0.00005f;
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_physicsTimeStep += 0.0001f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F5))
	{
		m_physicsTimeStep = 0.001f;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = BILLIARDS_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = TEST_SHAPES_3D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}

	UpdateCameras(deltaSeconds);
}

void Game::UpdatePachenkoPhysics(float deltaSeconds)
{
	for (int index = 0; index < static_cast<int>(cueballs.size()); index++)
	{
		cueballs[index]->UpdateBall(deltaSeconds);
		for (int bumper = 0; bumper < PACHENKO; bumper++)
		{
			cueballs[index]->BounceOffBumperDisc(m_rayDiscCenters[bumper], m_rayDiscRadii[bumper], m_rayDiscElasticity[bumper]);
		}
		for (int bumper = 0; bumper < PACHENKO; bumper++)
		{
			cueballs[index]->BounceOffBumperOBB(m_bumperOBB[bumper], m_obbElasticity[bumper]);
		}
		for (int bumper = 0; bumper < PACHENKO; bumper++)
		{
			cueballs[index]->BounceOffBumperCapsule(m_bumperCapsule[bumper], m_capsuleElasticity[bumper]);
		}
	}

	for (int index = 0; index < static_cast<int>(cueballs.size()) - 1; index++)
	{
		for (int other = index + 1; other < cueballs.size(); other++)
		{
			cueballs[index]->BounceOffBall(*cueballs[other]);
		}
	}
}

void Game::SixDOF(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith

	if (g_theInput->IsKeyDown('H'))
	{
		m_position = Vec3(0.0f, 0.0f, 0.0f); // reset to origin
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_SHIFT) || controller.WasButtonJustPressed(A))
	{
		m_speed *= 4.0f;
		m_rotate *= 2.0f;
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_SHIFT) || controller.WasButtonJustReleased(A))
	{
		m_speed *= 0.25f;
		m_rotate *= 0.5f;
	}

	// Translate position
	Vec3 i;
	Vec3 j;
	Vec3 k;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);

	if (g_theInput->IsKeyDown('W') || controller.GetLeftStick().GetPosition().y > 0.1f)
	{
		m_position += m_speed * deltaSeconds * i;
	}
	if (g_theInput->IsKeyDown('S') || controller.GetLeftStick().GetPosition().y < -0.1f)
	{
		m_position += -1.0f * m_speed * deltaSeconds * i;
	}

	if (g_theInput->IsKeyDown('A') || controller.GetLeftStick().GetPosition().x < -0.1f)
	{
		m_position += m_speed * deltaSeconds * j;
	}
	if (g_theInput->IsKeyDown('D') || controller.GetLeftStick().GetPosition().x > 0.1f)
	{
		m_position += -1.0f * m_speed * deltaSeconds * j;
	}
	// up down
	if (g_theInput->IsKeyDown('Z') || controller.GetButton(RIGHT_SHOULDER).isKeyPressed)
	{
		m_position += m_speed * deltaSeconds * k;
	}
	if (g_theInput->IsKeyDown('C') || controller.GetButton(LEFT_SHOULDER).isKeyPressed)
	{
		m_position += -1.0f * m_speed * deltaSeconds * k;
	}

	// Euler angle orientation
	// roll
	if (g_theInput->IsKeyDown('Q') || controller.GetLeftTrigger() > 0.1f)
	{
		EulerAngles roll(0.0f, 0.0f, -1.0f * m_rotate * deltaSeconds);
		m_orientation += roll;
	}
	if (g_theInput->IsKeyDown('E') || controller.GetRightTrigger() > 0.1f)
	{
		EulerAngles roll(0.0f, 0.0f, m_rotate * deltaSeconds);
		m_orientation += roll;
	}
	m_orientation.m_rollDegrees = Clamp(m_orientation.m_rollDegrees, -45.0, 45.0);

	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + mouseDelta.y / 2.0f + controller.GetRightStick().GetPosition().y, -85.0, 85.0);

	// yaw
	m_orientation.m_yawDegrees -= (mouseDelta.x / 2.0f + controller.GetRightStick().GetPosition().x);

	m_worldCamera3D.SetPostion(m_position);
	m_worldCamera3D.SetOrientation(m_orientation);
}

void Game::Axial(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith

	if (g_theInput->IsKeyDown('H'))
	{
		m_position = Vec3(0.0f, 0.0f, 0.0f); // reset to origin
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_SHIFT) || controller.WasButtonJustPressed(A))
	{
		m_speed *= 4.0f;
		m_rotate *= 2.0f;
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_SHIFT) || controller.WasButtonJustReleased(A))
	{
		m_speed *= 0.25f;
		m_rotate *= 0.5f;
	}

	// Translate position
	Vec3 i;
	Vec3 j;
	Vec3 k;
	//m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);

	if (g_theInput->IsKeyDown('W') || controller.GetLeftStick().GetPosition().y > 0.1f)
	{
		m_position.x += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('S') || controller.GetLeftStick().GetPosition().y < -0.1f)
	{
		m_position.x += -1.0f * m_speed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('A') || controller.GetLeftStick().GetPosition().x < -0.1f)
	{
		m_position.y += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('D') || controller.GetLeftStick().GetPosition().x > 0.1f)
	{
		m_position.y += -1.0f * m_speed * deltaSeconds;
	}
	// up down
	if (g_theInput->IsKeyDown('Q') || controller.GetButton(RIGHT_SHOULDER).isKeyPressed)
	{
		m_position.z += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('E') || controller.GetButton(LEFT_SHOULDER).isKeyPressed)
	{
		m_position.z += -1.0f * m_speed * deltaSeconds;
	}

	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + mouseDelta.y / 2.0f + controller.GetRightStick().GetPosition().y, -89.9f, 89.9f);

	// yaw
	m_orientation.m_yawDegrees -= (mouseDelta.x / 2.0f + controller.GetRightStick().GetPosition().x);

	//m_position = Vec3(0.0f, 0.0f, -2.0f); // Test to get yellow box
	m_worldCamera3D.SetPostion(m_position);
	m_worldCamera3D.SetOrientation(m_orientation);
}

void Game::Update_TEST_SHAPES_3D_Mode(float deltaSeconds)
{
	// camera controls /////////////////////////////////////////
	if (m_UseAxialCameraControls)
	{
		Axial(deltaSeconds);
	}
	else
	{
		SixDOF(deltaSeconds);
	}

	// check SPACE for toggling raycast lock
	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		m_rayLock = !m_rayLock;
		if (m_rayLock)
		{
			m_lockedPosition = m_position;
			m_lockedOrientation = m_orientation;
		}
	}

	// swap position if ray locked //////////////////////////////////////////////
	if (m_rayLock)
	{
		m_truePosition = m_position;
		m_trueOrientation = m_orientation;
		m_position = m_lockedPosition;
		m_orientation = m_lockedOrientation;
	}

	// update ray cast and nearest point results //////////////////////////////////////////////
	Vec3 fwdNormal = m_orientation.GetForwardNormal();
	m_nearestRaycast = 0;
	m_nearestPoint = 0;

	for (int index = 0; index < static_cast<int>(m_shapes.size()); index++)
	{
		switch (m_shapes[index]->shape)
		{
		case BOX:
			m_shapes[index]->raycastResult = RaycastVsAABB3D(m_position, fwdNormal, RANGE, m_shapes[index]->box);
			if (m_shapes[index]->raycastResult.m_impactDist < m_shapes[m_nearestRaycast]->raycastResult.m_impactDist)
			{
				m_nearestRaycast = index;
			}
			m_shapes[index]->nearestPoint = GetNearestPointOnAABB3D(m_position, m_shapes[index]->box);
			if ((m_shapes[index]->nearestPoint - m_position).GetLengthSquared() < (m_shapes[m_nearestPoint]->nearestPoint - m_position).GetLengthSquared())
			{
				m_nearestPoint = index;
			}
			break;
		case SPHERE:
			m_shapes[index]->raycastResult = RaycastVsSphere(m_position, fwdNormal, RANGE, m_shapes[index]->center, m_shapes[index]->radius);
			if (m_shapes[index]->raycastResult.m_impactDist < m_shapes[m_nearestRaycast]->raycastResult.m_impactDist)
			{
				m_nearestRaycast = index;
			}
			m_shapes[index]->nearestPoint = GetNearestPointOnSphere(m_position, m_shapes[index]->center, m_shapes[index]->radius);
			if ((m_shapes[index]->nearestPoint - m_position).GetLengthSquared() < (m_shapes[m_nearestPoint]->nearestPoint - m_position).GetLengthSquared())
			{
				m_nearestPoint = index;
			}			
			break;
		case CYLINDER:
			m_shapes[index]->raycastResult = RaycastVsZCylinder(m_position, fwdNormal, RANGE, m_shapes[index]->center, m_shapes[index]->length, m_shapes[index]->radius);
			if (m_shapes[index]->raycastResult.m_impactDist < m_shapes[m_nearestRaycast]->raycastResult.m_impactDist)
			{
				m_nearestRaycast = index;
			}
			m_shapes[index]->nearestPoint = GetNearestPointOnCylinder(m_position, m_shapes[index]->center, m_shapes[index]->length, m_shapes[index]->radius);
			if ((m_shapes[index]->nearestPoint - m_position).GetLengthSquared() < (m_shapes[m_nearestPoint]->nearestPoint - m_position).GetLengthSquared())
			{
				m_nearestPoint = index;
			}			
			break;
		}
	}

	// if no raycast actually impacted, then default to NONE instead of zero
	if (m_shapes[m_nearestRaycast]->raycastResult.m_didImpact == false)
	{
		m_nearestRaycast = NONE;
	}

	// restore position if ray locked /////////////////////////////////
	if (m_rayLock)
	{
		m_position = m_truePosition;
		m_orientation = m_trueOrientation;
	}

	// check mouse grab //////////////////////////////////////
	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		if (m_selectedShape == NONE)
		{
			if (m_nearestRaycast != NONE)
			{
				m_selectedShape = m_nearestRaycast;
				if (m_shapes[m_nearestRaycast]->shape == BOX)
				{
					m_selectedOffset = m_shapes[m_nearestRaycast]->box.GetCenter() - m_position;
				}
				else
				{
					m_selectedOffset = m_shapes[m_nearestRaycast]->center - m_position;
				}
			}
		}
		else
		{
			m_selectedShape = NONE;
		}
	}

	// move selected shape with camera ///////////////////////////////////
	if (m_selectedShape != NONE)
	{
		if (m_shapes[m_selectedShape]->shape == BOX)
		{
			m_shapes[m_selectedShape]->box.SetCenter(m_position + m_selectedOffset);
		}
		else
		{
			m_shapes[m_selectedShape]->center = m_position + m_selectedOffset;
		}
	}

	// check overlaps ///////////////////////////////////
	for (int index = 0; index < static_cast<int>(m_shapes.size()); index++)
	{
		m_shapes[index]->overlap = false;
	}

	bool overlapping = false;
	for (int index = 0; index < static_cast<int>(m_shapes.size()) - 1; index++)
	{
		for (int test = index + 1; test < static_cast<int>(m_shapes.size()); test++)
		{
			switch (m_shapes[index]->shape)
			{
			case BOX:
				if (m_shapes[test]->shape == BOX)
				{
					overlapping = DoAABB3BoxesOverlap(m_shapes[index]->box, m_shapes[test]->box);
				}
				if (m_shapes[test]->shape == SPHERE)
				{
					overlapping = DoesSphereOverlapAABB3(m_shapes[test]->center, m_shapes[test]->radius, m_shapes[index]->box);
				}
				if (m_shapes[test]->shape == CYLINDER)
				{
					overlapping = DoesZCylinderOverlapAABB3(m_shapes[test]->center, m_shapes[test]->length, m_shapes[test]->radius, m_shapes[index]->box);
				}
				break;
			case SPHERE:
				if (m_shapes[test]->shape == BOX)
				{
					overlapping = DoesSphereOverlapAABB3(m_shapes[index]->center, m_shapes[index]->radius, m_shapes[test]->box);
				}
				if (m_shapes[test]->shape == SPHERE)
				{
					overlapping = DoSpheresOverlap3D(m_shapes[index]->center, m_shapes[index]->radius, m_shapes[test]->center, m_shapes[test]->radius);
				}
				if (m_shapes[test]->shape == CYLINDER)
				{
					overlapping = DoesSphereOverlapZCylinder(m_shapes[index]->center, m_shapes[index]->radius, m_shapes[test]->center, m_shapes[test]->length, m_shapes[test]->radius);
				}
				break;
			case CYLINDER:
				if (m_shapes[test]->shape == BOX)
				{
					overlapping = DoesZCylinderOverlapAABB3(m_shapes[index]->center, m_shapes[index]->length, m_shapes[index]->radius, m_shapes[test]->box);
				}
				if (m_shapes[test]->shape == SPHERE)
				{
					overlapping = DoesSphereOverlapZCylinder(m_shapes[test]->center, m_shapes[test]->radius, m_shapes[index]->center, m_shapes[index]->length, m_shapes[index]->radius);
				}
				if (m_shapes[test]->shape == CYLINDER)
				{
					overlapping = DoZcylindersOverlap(m_shapes[index]->center, m_shapes[index]->length, m_shapes[index]->radius, m_shapes[test]->center, m_shapes[test]->length, m_shapes[test]->radius);
				}
				break;
			}
			m_shapes[index]->overlap |= overlapping;
			m_shapes[test]->overlap |= overlapping;
		}
	}

	// scene change controls ///////////////////////////////////
	if (g_theInput->WasKeyJustPressed(KEYCODE_F4))
	{
		m_UseAxialCameraControls = !m_UseAxialCameraControls;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F6))
	{
		m_nextState = PACHENKO_2D;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F7))
	{
		m_nextState = SPLINES;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_F8))
	{
		m_nextState = GameState::RESTART;
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_ESC))
	{
		m_nextState = GameState::ATTRACT;
	}
}

void Game::RenderAttractMode() const
{
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(m_screenCamera);

	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 100.0f), 80.f, "Welcome to math visual tests!", Rgba8::RED, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 160.0f), 40.f, "Select the test mode by pressing the number key", Rgba8::GREEN, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 220.0f), 40.f, "1 : Nearest Point (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 280.0f), 40.f, "2 : Raycast vs Discs (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 340.0f), 40.f, "3 : Raycast vs AABB (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 400.0f), 40.f, "4 : Raycast vs Line Segments (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 460.0f), 40.f, "5 : Raycast vs OBB2 (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 520.0f), 40.f, "6 : Billiards (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 580.0f), 40.f, "7 : Pachenko (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 640.0f), 40.f, "8 : Test Shapes (3D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 700.0f), 40.f, "9 : Easing, Curves, and Splines (2D)", Rgba8::BLUE, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(100.0f, SCREEN_CAMERA_HEIGHT - 760.0f), 40.f, "Press ESC to exit program", Rgba8::GREEN, 0.6f);

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_NEAREST_POINT_2D_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera2D); // to fulfill requirements of assignment
											   
	// draw shapes in background first as lit/unlit
	if (GetNearestPointOnAABB2D(m_testPoint, m_aabb2) == m_testPoint)
	{
		AddVertsForAABB2D(drawVerts, m_aabb2, litColor);
	}
	else
	{
		AddVertsForAABB2D(drawVerts, m_aabb2, unlitColor);
	}

	if (GetNearestPointOnOBB2D(m_testPoint, m_obb2) == m_testPoint)
	{
		AddVertsForOBB2D(drawVerts, m_obb2, litColor);
	}
	else
	{
		AddVertsForOBB2D(drawVerts, m_obb2, unlitColor);
	}

	if (GetNearestPointOnDisc2D(m_testPoint, m_discCenter, m_discRadius) == m_testPoint)
	{
		AddVertsForDisc2D(drawVerts, m_discCenter, m_discRadius, litColor);
	}
	else
	{
		AddVertsForDisc2D(drawVerts, m_discCenter, m_discRadius, unlitColor);
	}

	if (GetNearestPointOnCapsule2D(m_testPoint, m_capsule) == m_testPoint)
	{
		AddVertsForCapsule2D(drawVerts, m_capsule, litColor);
	}
	else
	{
		AddVertsForCapsule2D(drawVerts, m_capsule, unlitColor);
	}

	if (GetNearestPointOnLineSegment2D(m_testPoint, m_lineSegment) == m_testPoint)
	{
		AddVertsForLineSegment2D(drawVerts, m_lineSegment, 1.0f, litColor);
	}
	else
	{
		AddVertsForLineSegment2D(drawVerts, m_lineSegment, 1.0f, unlitColor);
	}

	if (GetNearestPointOnInfiniteline2D(m_testPoint, m_infiniteLine) == m_testPoint)
	{
		AddVertsForLineSegment2D(drawVerts, m_infiniteLine, 1.0f, litColor);
	}
	else
	{
		AddVertsForLineSegment2D(drawVerts, m_infiniteLine, 1.0f, unlitColor);
	}

	// draw dots for nearest points with lines segments on top of shapes
	nearestPoint = GetNearestPointOnAABB2D(m_testPoint, m_aabb2);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	nearestPoint = GetNearestPointOnCapsule2D(m_testPoint, m_capsule);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	nearestPoint = GetNearestPointOnDisc2D(m_testPoint, m_discCenter, m_discRadius);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	nearestPoint = GetNearestPointOnInfiniteline2D(m_testPoint, m_infiniteLine);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	nearestPoint = GetNearestPointOnLineSegment2D(m_testPoint, m_lineSegment);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	nearestPoint = GetNearestPointOnOBB2D(m_testPoint, m_obb2);
	AddVertsForLineSegment2D(drawVerts, LineSegment2(m_testPoint, nearestPoint), 0.3f, dimWhite);
	AddVertsForDisc2D(drawVerts, nearestPoint, 0.5f, goldColor);

	// draw the test point last to overlay everything
	AddVertsForDisc2D(drawVerts, m_testPoint, 0.25f, Rgba8::WHITE);
	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	g_theRenderer->EndCamera(m_worldCamera2D);
	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
												//RenderLives();
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Nearest Point (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "WASD to move point (or joystick); hold T for slow", Rgba8(0, 255, 192, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::testWash(Camera const& camera, Rgba8 color) const
{
	if (m_showColors == false)
	{
		return;
	}

	// Draw dark background on entire screen when dead
	std::vector<Vertex_PCU> overlayVerts;
	AABB2 bounds(camera.GetOrthoBottomLeft(), camera.GetOrthoTopRight());
// 	g_theRenderer->BindTexture(nullptr);
// 	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	AddVertsForAABB2D(overlayVerts, bounds, color);
	g_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
}

void Game::Render_SPLINES_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Curve curve;
	curve.SetSubdivisions(m_numSubdivisions);

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment

	// Draw Easing Function (upper left pane)
	g_theRenderer->BeginCamera(m_worldCamera1); // to fulfill requirements of assignment
	curve.SetViewPort(m_worldCamera1, AABB2(0.3f, 0.1f, 0.7f, 0.985f));
	testWash(m_worldCamera1, Rgba8::RED);
	curve.DrawEasingFunction(easingFunction[m_selectedFunction], m_t);
	g_theRenderer->EndCamera(m_worldCamera1);

	// Draw cubic Bezier curve (upper right pane)
	g_theRenderer->BeginCamera(m_worldCamera2); // to fulfill requirements of assignment
	curve.SetViewPort(m_worldCamera2, AABB2(0.01f, 0.02f, 0.99f, 0.98f));
	testWash(m_worldCamera2, Rgba8::GREEN);
	curve.DrawCubicBezierCurve(m_bezier, m_t);
	g_theRenderer->EndCamera(m_worldCamera2);

	// Draw Hermite curve (lower half pane)
	g_theRenderer->BeginCamera(m_worldCamera3); // to fulfill requirements of assignment
	curve.SetViewPort(m_worldCamera3, AABB2(0.01f, 0.02f, 0.9f, 0.98f));
	testWash(m_worldCamera3, Rgba8::BLUE);
	curve.DrawCubicHermiteCurve(m_hermite, m_t, m_segment);
	g_theRenderer->EndCamera(m_worldCamera3);

	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
												//RenderLives();
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	std::string prompt = "W/E = prev/next Easing function; N/M = curve subdivisions (";
	prompt += std::to_string(m_numSubdivisions);
	prompt += "), hold T for slow, hold C for tinted view ports";
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Easing, Curves, and Splines (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, prompt, Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(402.f - 9.0f * (int)easingFunctionName[m_selectedFunction].size(), SCREEN_CAMERA_HEIGHT - 424.0f), 30.f, easingFunctionName[m_selectedFunction], Rgba8::GREEN, 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_RAYCAST_VS_DISCS_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	float nearestDistance = 99999.0f;
	int nearestDisc = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera2D); // to fulfill requirements of assignment

	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[index], m_rayDiscRadii[index]);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestDisc = index;
			}
		}
	}

	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[index], m_rayDiscRadii[index], unlitColor);
	}

	if (nearestDisc != -1)
	{
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc], litColor);
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc]);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}
	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);
	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Raycast vs Discs (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD to move ray start; Rmouse or IJKL to move ray end", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_RAYCAST_VS_AABB2D_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	RaycastResult2D raycastHitSave;
	float nearestDistance = 99999.0f;
	int nearestRect = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera2D); // to fulfill requirements of assignment

	for (int index = 0; index < RAY_AABB2D_TOTAL; index++)
	{
		raycastHit = RaycastVsAABB2D(m_startPos, m_fwdNormal, m_maxDist, m_rayRects[index]);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestRect = index;
				raycastHitSave = raycastHit;
			}
		}
	}

	for (int index = 0; index < RAY_AABB2D_TOTAL; index++)
	{
		AddVertsForAABB2D(drawVerts, m_rayRects[index], unlitColor);
	}

	if (nearestRect != -1)
	{
		AddVertsForAABB2D(drawVerts, m_rayRects[nearestRect], litColor);
		raycastHit = RaycastVsAABB2D(m_startPos, m_fwdNormal, m_maxDist, m_rayRects[nearestRect]);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}
	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);
	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Raycast vs AABB (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD to move ray start; Rmouse or IJKL to move ray end", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_RAYCAST_VS_Lines_Mode() const
{
	float lineThickness = 0.5f;
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	RaycastResult2D raycastHitSave;
	float nearestDistance = 99999.0f;
	int nearestLine = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera2D); // to fulfill requirements of assignment

	for (int index = 0; index < SEGMENT_TOTAL; index++)
	{
		raycastHit = RaycastVsLineSegment2D(m_startPos, m_fwdNormal, m_maxDist, m_lineSegments[index].m_startPos, m_lineSegments[index].m_endPos);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestLine = index;
				raycastHitSave = raycastHit;
			}
		}
	}

	for (int index = 0; index < SEGMENT_TOTAL; index++)
	{
		AddVertsForLineSegment2D(drawVerts, m_lineSegments[index], lineThickness, unlitColor);
	}

	if (nearestLine != -1)
	{
		AddVertsForLineSegment2D(drawVerts, m_lineSegments[nearestLine], lineThickness, litColor);
		raycastHit = RaycastVsLineSegment2D(m_startPos, m_fwdNormal, m_maxDist, m_lineSegments[nearestLine].m_startPos, m_lineSegments[nearestLine].m_endPos);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}
	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);
	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Raycast vs Line Segment (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD to move ray start; Rmouse or IJKL to move ray end", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_RAYCAST_VS_OBB2_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	RaycastResult2D raycastHitSave;
	float nearestDistance = 99999.0f;
	int nearestRect = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera2D); // to fulfill requirements of assignment

	for (int index = 0; index < OBB2_TOTAL; index++)
	{
		raycastHit = RaycastVsOBB2D(m_startPos, m_fwdNormal, m_maxDist, m_obb2s[index]);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestRect = index;
				raycastHitSave = raycastHit;
			}
		}
	}

	for (int index = 0; index < OBB2_TOTAL; index++)
	{
		AddVertsForOBB2D(drawVerts, m_obb2s[index], unlitColor);
	}

	if (nearestRect != -1)
	{
		AddVertsForOBB2D(drawVerts, m_obb2s[nearestRect], litColor);
		raycastHit = RaycastVsOBB2D(m_startPos, m_fwdNormal, m_maxDist, m_obb2s[nearestRect]);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}
	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);
	g_theRenderer->BeginCamera(m_screenCamera); // to fulfill requirements of assignment
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Raycast vs OBB2 (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD to move ray start; Rmouse or IJKL to move ray end", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_BILLIARDS_2D_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	float nearestDistance = 99999.0f;
	int nearestDisc = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  
	g_theRenderer->BeginCamera(m_worldCamera2D);

	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[index], m_rayDiscRadii[index]);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestDisc = index;
			}
		}
	}

	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[index], m_rayDiscRadii[index], Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[index]));
	}

	if (nearestDisc != -1)
	{
		Rgba8 collideColor = Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[nearestDisc]);
		collideColor.b = 127;
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc], collideColor);
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc]);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}

	for (Cueball* cueball : cueballs)
	{
	   if (cueball)
	   {
		cueball->AddVerts(drawVerts);
	   }
	}

	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);

	g_theRenderer->BeginCamera(m_screenCamera); 
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Billiards (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD moves ray start; Rmouse or IJKL moves ray end, Space to shoot ball", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_PACHENKO_2D_Mode() const
{
	Rgba8 unlitColor(0, 0, 128, 196);
	Rgba8 litColor(0, 0, 255, 196);
	Rgba8 goldColor(255, 165, 55, 255);
	Rgba8 dimWhite(255, 255, 255, 30);
	std::vector<Vertex_PCU> drawVerts;
	Vec2 nearestPoint;

	RaycastResult2D raycastHit;
	float nearestDistance = 99999.0f;
	int nearestDisc = -1;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));
	g_theRenderer->BeginCamera(m_worldCamera2D);

	for (int index = 0; index < PACHENKO; index++)
	{
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[index], m_rayDiscRadii[index]);
		if (raycastHit.m_didImpact)
		{
			if (raycastHit.m_impactDist < nearestDistance)
			{
				nearestDistance = raycastHit.m_impactDist;
				nearestDisc = index;
			}
		}
	}

	for (int index = 0; index < PACHENKO; index++)
	{
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[index], m_rayDiscRadii[index], Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[index]));
		AddVertsForOBB2D(drawVerts, m_bumperOBB[index], Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[index]));
		AddVertsForCapsule2D(drawVerts, m_bumperCapsule[index], Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[index]));
	}

	if (nearestDisc != -1)
	{
		Rgba8 collideColor = Rgba8::ColorLerp(Rgba8::RED, Rgba8::GREEN, m_rayDiscElasticity[nearestDisc]);
		collideColor.b = 127;
		AddVertsForDisc2D(drawVerts, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc], collideColor);
		raycastHit = RaycastVsDisc2D(m_startPos, m_fwdNormal, m_maxDist, m_rayDiscCenters[nearestDisc], m_rayDiscRadii[nearestDisc]);
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8(127, 127, 127, 255));
		AddVertsForArrow2D(drawVerts, m_startPos, raycastHit.m_impactPos, 2.0f, 0.3f, Rgba8::RED);
		AddVertsForArrow2D(drawVerts, raycastHit.m_impactPos, raycastHit.m_impactPos + 7.0f * raycastHit.m_impactNormal, 2.0f, 0.3f, Rgba8::YELLOW);
		AddVertsForDisc2D(drawVerts, raycastHit.m_impactPos, 0.5f, Rgba8::WHITE);
	}
	else
	{
		AddVertsForArrow2D(drawVerts, m_startPos, m_endPos, 2.0f, 0.3f, Rgba8::GREEN);
	}
	AddVertsForRing2D(drawVerts, m_startPos, m_ballRadius, Rgba8::WHITE, 0.3f, 16);

	for (Cueball* cueball : cueballs)
	{
		if (cueball)
		{
			cueball->AddVerts(drawVerts);
		}
	}

	g_theRenderer->DrawVertexArray(int(drawVerts.size()), drawVerts.data());

	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->EndCamera(m_worldCamera2D);
	///////////////////////////////////////////
	g_theRenderer->BeginCamera(m_screenCamera);
	char buffer[80];
	sprintf_s(buffer, "Physics step: %.5fs Delta: %.5fs Frames: %.1f Balls: %i", m_physicsTimeStep, m_deltaSeconds, 1.0f / m_deltaSeconds, m_balls);
	g_testFont->AddVertsForText2D(textVerts, Vec2(800.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, buffer, Rgba8::RED, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Billiards (2D) Mode: (F6/F7 for prev/next), F8 to randomize;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "Lmouse or WASD moves ray start; Rmouse or IJKL moves ray end", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "Arrow keys translate ray endpoints together; hold T for slow", Rgba8(0, 128, 255, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 120.0f), 20.f, "Space to create ball, hold N for multiple balls, B toggles bottom, F3/F4 adjust time step", Rgba8(0, 128, 128, 255), 0.6f);

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::Render_TEST_SHAPES_3D_Mode() const
{
	std::string str;
	std::vector<Vertex_PCU> vertexArray;
	std::vector<Vertex_PCU> textVerts;

	g_theRenderer->ClearScreen(Rgba8(0, 0, 0, 255));  // as requested by the assignment
	g_theRenderer->BeginCamera(m_worldCamera3D);
	// set world camera state
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
	g_theRenderer->SetSamplerMode(SamplerMode::BILINEARWRAP);
	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);

// 	SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
// 	BindShader(nullptr);
// 	SetModelColor(Rgba8::WHITE);
// 	SetSamplerMode(SamplerMode::POINTCLAMP);
// 	SetModelMatrix(Mat44());
// 	SetDepthStencilState(DepthTest::ALWAYS, false);
// 	SetBlendMode(BlendMode::ALPHA);

	// create basis at origin TEST DEBUG
	AddVertsForSphere(vertexArray, Vec3::ZERO, EulerAngles::ZERO, 0.03f);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(1.0f, 0.0f, 0.0f), 0.02f, Rgba8::RED);
//	AddVertsForCylinder3D(vertexArray, Vec3(0.5f, 0.0f, 0.0f), EulerAngles(0.0f, 90.0f, 0.0f), 1.0f, 0.02f, Rgba8::GOLD, 16);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(0.0f, 1.0f, 0.0f), 0.02f, Rgba8::GREEN);
	AddVertsForCylinder3D(vertexArray, Vec3::ZERO, Vec3(0.0f, 0.0f, 1.0f), 0.02f, Rgba8::BLUE);

	// create basis at camera 	AddVertsForCylinder3D(verts, position, orientation, length, radius, color, edges);
	Vec3 i, j, k;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	Vec3 wcPos = m_position + 0.2f * i;
	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.01f, 0.0f, 0.0f), 0.0005f, Rgba8::RED);
	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.0f, 0.01f, 0.0f), 0.0005f, Rgba8::GREEN);
	AddVertsForCylinder3D(vertexArray, wcPos, wcPos + Vec3(0.0f, 0.0f, 0.01f), 0.0005f, Rgba8::BLUE);

	g_theRenderer->BindTexture(nullptr);
	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
	vertexArray.clear();

	// draw textured objects
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);

	for (int index = 0; index < static_cast<int>(m_shapes.size() / 2); index++)
	{
		Rgba8 color = index == m_selectedShape ? Rgba8::RED : index == m_nearestRaycast ? Rgba8::BLUE : Rgba8::WHITE;
		if (m_shapes[index]->overlap)
		{
			color = color.dim(static_cast<float>((sin(4.0 * GetCurrentTimeSeconds()) + 1.0) * 0.375 + 0.25));
		}
		switch (m_shapes[index]->shape)
		{
		case BOX:
			AddVertsForAABB3D(vertexArray, m_shapes[index]->box, color);
			break;
		case SPHERE:
			AddVertsForSphere(vertexArray,  m_shapes[index]->center,  m_shapes[index]->orientation,  m_shapes[index]->radius, 16, 8, color);
			break;
		case CYLINDER:
			AddVertsForCylinder3D(vertexArray, m_shapes[index]->center, EulerAngles::ZERO, m_shapes[index]->length, m_shapes[index]->radius, color);
			break;
		}
	}

	if (vertexArray.size())
	{
		Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Test_StbiFlippedAndOpenGL.png");
		g_theRenderer->BindTexture(testTexture);
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		vertexArray.clear();
	}

	// draw wire frame objects //////////////////
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);

	for (int index = static_cast<int>(m_shapes.size() / 2); index < static_cast<int>(m_shapes.size()) - 1; index++)
	{
		Rgba8 color = index == m_selectedShape ? Rgba8::RED : index == m_nearestRaycast ? Rgba8::BLUE : Rgba8::WHITE;
		if (m_shapes[index]->overlap)
		{
			color = color.dim(static_cast<float>((sin(4.0 * GetCurrentTimeSeconds()) + 1.0) * 0.375 + 0.25));
		}
		switch (m_shapes[index]->shape)
		{
		case BOX:
			AddVertsForAABB3D(vertexArray, m_shapes[index]->box, color);
			break;
		case SPHERE:
			AddVertsForSphere(vertexArray, m_shapes[index]->center, m_shapes[index]->orientation, m_shapes[index]->radius, 16, 8, color);
			break;
		case CYLINDER:
			AddVertsForCylinder3D(vertexArray, m_shapes[index]->center, EulerAngles::ZERO, m_shapes[index]->length, m_shapes[index]->radius, color);
			break;
		}
	}

	g_theRenderer->BindTexture(0);
	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
	vertexArray.clear();

	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	int last = static_cast<int>(m_shapes.size()) - 1;
	Rgba8 color = last == m_selectedShape ? Rgba8::RED : last == m_nearestRaycast ? Rgba8::BLUE : Rgba8::WHITE;
	if (m_shapes[last]->overlap)
	{
		color = color.dim(static_cast<float>((sin(4.0 * GetCurrentTimeSeconds()) + 1.0) * 0.375 + 0.25));
	}
	EulerAngles rotation(static_cast<float>(fmod(15.0 * GetCurrentTimeSeconds(), 360.0)), 0.0f, 0.0f);
	AddVertsForSphere(vertexArray, m_shapes[last]->center, rotation, m_shapes[last]->radius, 32, 16, color);
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/worldmap.png");
	g_theRenderer->BindTexture(testTexture);
	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
	vertexArray.clear();

	// draw hit test indicators
	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);

	// swap position if ray locked //////////////////////////////////////////////
	if (m_rayLock)
	{
		m_truePosition = m_position;
		m_trueOrientation = m_orientation;
		m_position = m_lockedPosition;
		m_orientation = m_lockedOrientation;
	}

	// draw nearest point and lines to
	for (int index = 0; index < static_cast<int>(m_shapes.size()); index++)
	{
		AddVertsForSphere(vertexArray, m_shapes[index]->nearestPoint, EulerAngles::ZERO, 0.05f, 16, 8, index == m_nearestPoint ? Rgba8::GREEN : Rgba8::RED);
		AddVertsForLine3D(vertexArray, m_position, m_shapes[index]->nearestPoint, 0.0125f, index == m_nearestPoint ? Rgba8::GREEN : Rgba8::RED);
	}

	if (m_nearestRaycast != NONE)
	{
		Vec3 tailPos = m_shapes[m_nearestRaycast]->raycastResult.m_impactPos;
		Vec3 tipPos = m_shapes[m_nearestRaycast]->raycastResult.m_impactPos + m_shapes[m_nearestRaycast]->raycastResult.m_impactNormal * 0.5f;
		AddVertsForSphere(vertexArray, tailPos, EulerAngles::ZERO, 0.05f, 16, 8, Rgba8::WHITE);
		AddVertsForArrow3D(vertexArray, tailPos, tipPos, 0.125f, 0.025f, Rgba8::YELLOW);
		AddVertsForLine3D(vertexArray, m_position, tailPos, 0.025f, Rgba8::RED);
	}
	else
	{
		AddVertsForArrow3D(vertexArray, m_position, m_position + m_orientation.GetForwardNormal() * RANGE, 0.125f, 0.025f, Rgba8::GREEN);
	}

	if (m_rayLock)
	{
		m_position = m_truePosition;
		m_orientation = m_trueOrientation;
	}

	g_theRenderer->BindTexture(0);
	g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
	vertexArray.clear();

	g_theRenderer->EndCamera(m_worldCamera3D);

	// text rendering //////////////////////
	g_theRenderer->BeginCamera(m_screenCamera);
	// set back to screen camera state
	g_theRenderer->SetRasterizerState(CullMode::NONE, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
	g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);

	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 30.0f), 20.f, "Test Shapes (3D) Mode: (F6/F7 for prev/next), F8 to randomize; F4 to toggle 6 DOF Camera;", Rgba8::YELLOW, 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 60.0f), 20.f, "WS to move on X axis, AD to move on Y axis QE to move on Z axis, Mouse to yaw or pitch view", Rgba8(0, 255, 192, 255), 0.6f);
	g_testFont->AddVertsForText2D(textVerts, Vec2(10.f, SCREEN_CAMERA_HEIGHT - 90.0f), 20.f, "H to move to origin, L mouse to toggle grab object, SPACE to toggle ray lock, hold T for slow", Rgba8(0, 255, 192, 255), 0.6f);

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
	g_theRenderer->EndCamera(m_screenCamera);
}

void Game::TestTexture() const
{
	Rgba8 tintColor(255, 255, 255, 255);
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(10.0f, 10.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(40.0f, 40.0f, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(10.0f, 40.0f, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle
	vertex[3] = Vertex_PCU(Vec3(10.0f, 10.0f, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(40.0f, 10.0f, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(40.0f, 40.0f, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw test texture
	Texture* testTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Test_StbiFlippedAndOpenGL.png");
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(testTexture);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(nullptr); // disables texturing in OpenGL (for now) bad hack to disable except here
}

Vec2 Game::GetRandomVec2ZeroToOne() const
{
	float xCoord = random.RollRandomFloatZeroToOne();
	float yCoord = random.RollRandomFloatZeroToOne();
	return Vec2(xCoord, yCoord);
}

Vec2 Game::GetRandomPoint2D(float buffer)
{
	float xCoord = random.RollRandomFloatInRange(buffer, WORLD_SIZE_X - buffer);
	float yCoord = random.RollRandomFloatInRange(buffer, WORLD_SIZE_Y - buffer);
	return Vec2(xCoord, yCoord);
}

Vec3 Game::GetRandomPoint3D(float limit)
{
	float x = random.RollRandomFloatInRange(-limit, limit);
	float y = random.RollRandomFloatInRange(-limit, limit);
	float z = random.RollRandomFloatInRange(-limit * 0.5f, limit * 0.5f);
	return Vec3(x, y, z);
}

void Game::Swap(float& a, float& b)
{
float temp = a;
a = b;
b = temp;
}

void Game::CreateRandomShapes()
{
	float buffer;

	// AABB2
	buffer = random.RollRandomFloatInRange(10.0f, 20.0f);
	Vec2 vectorA = GetRandomPoint2D(buffer);
	Vec2 VectorB = GetRandomPoint2D(buffer);
	while(fabsf(VectorB.x - vectorA.x) > 50.0f || fabsf(VectorB.x - vectorA.x) < 10.0f || fabsf(VectorB.y - vectorA.y) > 30.0f || fabsf(VectorB.y - vectorA.y) < 10.0f)
		VectorB = GetRandomPoint2D(buffer);
	if (vectorA.x > VectorB.x)
		Swap(vectorA.x, VectorB.x);
	if (vectorA.y > VectorB.y)
		Swap(vectorA.y, VectorB.y);
	m_aabb2 = AABB2(vectorA, VectorB);

	// OBB2
	buffer = random.RollRandomFloatInRange(5.0f, 20.0f);
	Vec2 normal = GetRandomPoint2D(buffer);
	normal.Normalize();
	vectorA = GetRandomPoint2D(buffer);
	VectorB = Vec2(random.RollRandomFloatInRange(5.0f, 15.0f), random.RollRandomFloatInRange(5.0f, 15.0f));
	m_obb2 = OBB2(vectorA, normal, VectorB);

	// Capsule2
	buffer = random.RollRandomFloatInRange(5.0f, 20.0f);
	vectorA = GetRandomPoint2D(buffer);
	VectorB = GetRandomPoint2D(buffer);
	while (fabsf(VectorB.x - vectorA.x) > 30.0f || fabsf(VectorB.x - vectorA.x) < 10.0f || fabsf(VectorB.y - vectorA.y) > 20.0f || fabsf(VectorB.y - vectorA.y) < 5.0f)
		VectorB = GetRandomPoint2D(buffer);
	if (vectorA.x > VectorB.x)
		Swap(vectorA.x, VectorB.x);
	if (vectorA.y > VectorB.y)
		Swap(vectorA.y, VectorB.y);
	m_capsule = Capsule2(vectorA, VectorB, buffer);

	// LineSegment2
	buffer = random.RollRandomFloatInRange(10.0f, 20.0f);
	buffer = random.RollRandomFloatInRange(5.0f, 20.0f);
	vectorA = GetRandomPoint2D(buffer);
	VectorB = GetRandomPoint2D(buffer);
	while (fabsf(VectorB.x - vectorA.x) > 50.0f || fabsf(VectorB.x - vectorA.x) < 10.0f || fabsf(VectorB.y - vectorA.y) > 30.0f || fabsf(VectorB.y - vectorA.y) < 10.0f)
		VectorB = GetRandomPoint2D(buffer);
	m_lineSegment = LineSegment2(vectorA, VectorB);

	// Infinite line
	buffer = random.RollRandomFloatInRange(10.0f, 20.0f);
	buffer = random.RollRandomFloatInRange(5.0f, 20.0f);
	vectorA = GetRandomPoint2D(buffer);
	VectorB = GetRandomPoint2D(buffer);
	while (fabsf(VectorB.x - vectorA.x) > 50.0f || fabsf(VectorB.x - vectorA.x) < 10.0f || fabsf(VectorB.y - vectorA.y) > 30.0f || fabsf(VectorB.y - vectorA.y) < 10.0f)
		VectorB = GetRandomPoint2D(buffer);
	Vec2 segment = VectorB -  vectorA;
	VectorB += 20.0f * segment;
	vectorA -= 20.0f * segment;
	m_infiniteLine = LineSegment2(vectorA, VectorB);

	// disc parameters
	buffer = random.RollRandomFloatInRange(5.0f, 20.0f);
	m_discCenter = GetRandomPoint2D(buffer);
	m_discRadius = buffer;

	buffer = random.RollRandomFloatInRange(10.0f, 20.0f);
	m_testPoint = GetRandomPoint2D(buffer);
}

void Game::CreateRandomDiscs()
{
	float buffer;

	// disc parameters
	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		buffer = random.RollRandomFloatInRange(2.0f, 12.0f);
		m_rayDiscCenters[index] = GetRandomPoint2D(buffer);
		m_rayDiscRadii[index] = buffer;
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
}

void Game::CreateRandomRectangles()
{
	float buffer;

	// rectangle parameters
	for (int index = 0; index < RAY_AABB2D_TOTAL; index++)
	{
		buffer = random.RollRandomFloatInRange(16.0f, 40.0f);
		Vec2 origin = GetRandomPoint2D(buffer);
		Vec2 corner;
		corner.x = origin.x + random.RollRandomFloatInRange(8.0f, buffer);
		corner.y = origin.y + random.RollRandomFloatInRange(8.0f, buffer);
		m_rayRects[index] = AABB2(origin, corner);
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
}

void Game::CreateRandomLineSegments()
{
	float buffer;

	// rectangle parameters
	for (int index = 0; index < SEGMENT_TOTAL; index++)
	{
		buffer = random.RollRandomFloatInRange(4.0f, 20.0f);
		Vec2 origin = GetRandomPoint2D(buffer);
		Vec2 corner = GetRandomPoint2D(buffer);
		m_lineSegments[index] = LineSegment2(origin, corner);
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
}

void Game::CreateRandomOBB2s()
{
	float buffer;

	// rectangle parameters
	for (int index = 0; index < OBB2_TOTAL; index++)
	{
		buffer = random.RollRandomFloatInRange(16.0f, 20.0f);
		Vec2 center = GetRandomPoint2D(buffer);
		Vec2 normal = random.GenerateRandomUnitVector2D();
		Vec2 halfDims;
		halfDims.x = random.RollRandomFloatInRange(2.0f, 16.0f);
		halfDims.y = random.RollRandomFloatInRange(2.0f, 16.0f);
		m_obb2s[index] = OBB2(center, normal, halfDims);
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);

// 	Vec2 n(1.0f, 0.0f);
// //	n.RotateDegrees(30.0f);
// 	m_obb2s[0] = OBB2(Vec2(100.0f, 50.0f), n, Vec2(10.0f, 10.0f));
// 	m_startPos = Vec2(100.0f, 80.0f);
// 	m_endPos = Vec2(100.0f, 20.0f);
}

void Game::CreateRandomBalls()
{
	float buffer;

	// disc parameters
	for (int index = 0; index < RAY_DISC_TOTAL; index++)
	{
		buffer = random.RollRandomFloatInRange(2.0f, 12.0f);
		m_rayDiscCenters[index] = GetRandomPoint2D(buffer);
		m_rayDiscRadii[index] = buffer;
		m_rayDiscElasticity[index] = random.RollRandomFloatZeroToOne();
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
}

void Game::CreatePachenko()
{
	float buffer;

	// disc parameters
	for (int index = 0; index < PACHENKO; index++)
	{
		buffer = random.RollRandomFloatInRange(2.0f, 8.0f);
		m_rayDiscCenters[index] = GetRandomPoint2D(buffer);
		m_rayDiscRadii[index] = buffer;
		m_rayDiscElasticity[index] = random.RollRandomFloatZeroToOne();
	}

	for (int index = 0; index < PACHENKO; index++)
	{
		buffer = random.RollRandomFloatInRange(2.0f, 12.0f);
		Vec2 center = GetRandomPoint2D(buffer);
		Vec2 normal = random.GenerateRandomUnitVector2D();
		Vec2 halfDims;
		halfDims.x = random.RollRandomFloatInRange(2.0f, buffer);
		halfDims.y = random.RollRandomFloatInRange(2.0f, buffer);
		m_bumperOBB[index] = OBB2(center, normal, halfDims);
		m_obbElasticity[index] = random.RollRandomFloatZeroToOne();
	}

	for (int index = 0; index < PACHENKO; index++)
	{
		buffer = random.RollRandomFloatInRange(2.0f, 12.0f);
		Vec2 start = GetRandomPoint2D(buffer);
		Vec2 end = start + random.GenerateRandomUnitVector2D() * random.RollRandomFloatInRange(2.0f, buffer);
		float radius = random.RollRandomFloatInRange(2.0f, buffer * 0.5f);
		m_bumperCapsule[index] = Capsule2(start, end, radius);
		m_capsuleElasticity[index] = random.RollRandomFloatZeroToOne();
	}

	m_startPos = GetRandomPoint2D(30.0f);
	m_endPos = GetRandomPoint2D(30.0f);
// 	m_startPos = GetRandomPoint2D(30.0f);
// 	m_endPos = GetRandomPoint2D(30.0f);
}

void Game::CreateRandom3DShapes()
{
	Shapes3D* thing;

	for (int index = 0; index < 4; index++)
	{
		Vec3 origin = GetRandomPoint3D(random.RollRandomFloatInRange(3.0f, 16.0f));
		Vec3 corner;
		corner.x = origin.x + random.RollRandomFloatInRange(1.0f, 2.0f);
		corner.y = origin.y + random.RollRandomFloatInRange(1.0f, 2.0f);
		corner.z = origin.z + random.RollRandomFloatInRange(1.0f, 2.0f);
		thing = new Shapes3D();
		thing->shape = BOX;
		thing->box = AABB3(origin, corner);
		m_shapes.push_back(thing);

		thing = new Shapes3D();
		thing->shape = SPHERE;
		thing->center = GetRandomPoint3D(random.RollRandomFloatInRange(3.0f, 16.0f));
		thing->radius = random.RollRandomFloatInRange(0.5f, 2.0f);
		thing->orientation = EulerAngles::ZERO;
		m_shapes.push_back(thing);

		thing = new Shapes3D();
		thing->shape = CYLINDER;
		thing->center = GetRandomPoint3D(random.RollRandomFloatInRange(3.0f, 16.0f));
		thing->length = random.RollRandomFloatInRange(1.0f, 3.0f);
		thing->radius = random.RollRandomFloatInRange(0.5f, 2.0f);
		m_shapes.push_back(thing);
	}

// 	thing = new Shapes3D();
// 	thing->shape = BOX;
// 	thing->box = AABB3::ZERO_TO_ONE;
// 	m_shapes.push_back(thing);

	thing = new Shapes3D();
	thing->shape = SPHERE;
	thing->center = Vec3(20.0f, 20.0f, 0.0f);
	thing->radius = 3.0f;
	thing->orientation = EulerAngles::ZERO;
	m_shapes.push_back(thing);
}

void Game::CreateHermiteSpline()
{
	m_hermite.clear();
	m_controlPoints.clear();

	int points = random.RollRandomIntInRange(4, 10);
	float segment = 1.0f / (float)(points + 2);
	for (int index = 1; index <= points; index++)
	{
		Vec2 pt;
		pt.y = random.RollRandomFloatInRange(0.1f, 0.9f);
		pt.x = ((float)(random.RollRandomIntInRange(index - 1, index + 1)) + random.RollRandomFloatZeroToOne()) * segment;
		m_controlPoints.push_back(pt);
	}
	CubicBezierCurve2D hermiteCurve;
	Vec2 S;
	Vec2 E;
	Vec2 U = Vec2::ZERO;
	Vec2 V = Vec2::ZERO;

	for (int index = 0; index < points - 1; index++)
	{
		U = V; // shift velocity back
		S = index > 0 ? m_controlPoints[index] - m_controlPoints[index - 1] : Vec2::ZERO;
		E = (index + 1 < points) ? m_controlPoints[index + 1] - m_controlPoints[index] : Vec2::ZERO;
		V = (S + E) * 0.5f; // save next velocity
		hermiteCurve.CubicHermiteCurve2D(m_controlPoints[index], U, m_controlPoints[index + 1], V);
		m_hermite.push_back(hermiteCurve);
	}
}