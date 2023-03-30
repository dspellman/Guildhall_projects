#include "Game/Player.hpp"
#include "Game/GameCommon.hpp"
#include "Game/Game.hpp"
#include <vector>
#include "Engine/InputSystem/InputSystem.hpp"
#include "Engine/InputSystem/XboxController.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Controller.hpp"
#include "Game/Map.hpp"
#include "Actor.hpp"
#include "WeaponDefinition.hpp"

Player::Player(Vec3 const& position, EulerAngles orientation) 
	: m_position(position), m_orientation(orientation)
{
	if (m_animationClock)
	{
		delete m_animationClock;
		m_animationClock = nullptr;
	}
	m_animationClock = new Clock(g_systemClock);
}

Player::Player()
{
	if (m_animationClock)
	{
		delete m_animationClock;
		m_animationClock = nullptr;
	}
	m_animationClock = new Clock(g_systemClock);
}

Player::Player(int playerIndex, int controllerIndex)
	: m_playerIndex(playerIndex), m_controllerIndex(controllerIndex)
{
	if (m_animationClock)
	{
		delete m_animationClock;
		m_animationClock = nullptr;
	}
	m_animationClock = new Clock(g_systemClock);
}

Player::~Player()
{
	if (m_animationClock)
	{
		delete m_animationClock;
		m_animationClock = nullptr;
	}
}

void Player::Update(float deltaSeconds)
{
	m_grenadeTimer += deltaSeconds;
	if (m_grenadeTimer > GTIME)
	{
		m_grenadeTimer = 0.0f;
		GetActor()->m_grenadeCount++;
	}

	if (g_theInput->WasKeyJustReleased('F') && g_theGame->m_numPlayers == 1)
	{
		m_freeFlyCameraMode = !m_freeFlyCameraMode;
		if (m_freeFlyCameraMode)
		{
			m_position.z = GetActor()->m_definition->m_eyeHeight;
		}
	}

	// movement controls ////////////////////////////
	if (m_freeFlyCameraMode)
	{
		SixDOF(deltaSeconds);
	}
	else
	{
		if (GetActor()->m_isDead)
		{
			return; // can't do anything with a corpse
		}
		if (m_controllerIndex == (int)ControlType::MOUSE_KEYBOARD)
		{
			Keyboard(deltaSeconds);
		}
		if (m_controllerIndex == (int)ControlType::CONTROLLER)
		{
			Controller(deltaSeconds);
		}
	}
	g_theAudio->UpdateListener(m_playerIndex, m_position, m_orientation.GetForwardNormal(), Vec3(0.0f, 0.0f, 1.0f));
}

void Player::UpdateCameras()
{
	float fovDegrees = GetActor()->m_definition->m_cameraFOVDegrees;
	if (g_theGame->m_numPlayers == 2)
	{
		m_cameraWorld->SetPerspectiveView(4.0f, fovDegrees, 0.1f, 100.0f);
//		m_cameraHud->SetPerspectiveView(4.0f, fovDegrees, 0.1f, 100.0f);
		if (m_playerIndex == 1)
		{
			m_cameraWorld->SetViewport(AABB2(0.0f, 0.0f, 1.0f, 0.5f));
			m_cameraHud->SetViewport(AABB2(0.0f, 0.0f, 1.0f, 0.5f));
		}
		else
		{
			m_cameraWorld->SetViewport(AABB2(0.0f, 0.5f, 1.0f, 1.0f));
			m_cameraHud->SetViewport(AABB2(0.0f, 0.5f, 1.0f, 1.0f));
		}
	}
	else
	{
		m_cameraWorld->SetPerspectiveView(2.0f, fovDegrees, 0.1f, 100.0f);
		m_cameraWorld->SetViewport(AABB2::ZERO_TO_ONE);
		m_cameraHud->SetViewport(AABB2::ZERO_TO_ONE);
	}
}

void TestTexture()
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

void Player::HudScreen() const
{
	Rgba8 tintColor(255, 255, 255, 255);
	Vertex_PCU vertex[6];
	// upper triangle
	vertex[0] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoBottomLeft().x, m_cameraHud->GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[1] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoTopRight().x, m_cameraHud->GetOrthoTopRight().y * FRACTION, 0.0f), tintColor, Vec2(1.f, 1.f));
	vertex[2] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoBottomLeft().x, m_cameraHud->GetOrthoTopRight().y * FRACTION, 0.0f), tintColor, Vec2(0.f, 1.f));
	// lower triangle					  
	vertex[3] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoBottomLeft().x, m_cameraHud->GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(0.f, 0.f));
	vertex[4] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoTopRight().x, m_cameraHud->GetOrthoBottomLeft().y, 0.0f), tintColor, Vec2(1.f, 0.f));
	vertex[5] = Vertex_PCU(Vec3(m_cameraHud->GetOrthoTopRight().x, m_cameraHud->GetOrthoTopRight().y * FRACTION, 0.0f), tintColor, Vec2(1.f, 1.f));

	// Draw  texture
	Texture* hudTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Hud_Base.png");
	std::vector<Vertex_PCU> testVerts;
	for (int vertIndex = 0; vertIndex < 6; vertIndex++)
	{
		testVerts.push_back(vertex[vertIndex]);
	}

	//TransformVertexArrayXY3D(SHIP_VERTS, tvertex, m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->BindTexture(hudTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)testVerts.size(), testVerts.data());
	g_theRenderer->BindTexture(nullptr);

	if (GetActor()->m_isDead)
	{
		// Draw dark background on entire screen when dead
		std::vector<Vertex_PCU> overlayVerts;
		AABB2 bounds(m_cameraHud->GetOrthoBottomLeft(), m_cameraHud->GetOrthoTopRight());
		g_theRenderer->BindTexture(nullptr); 
		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		AddVertsForAABB2D(overlayVerts, bounds, Rgba8(0, 0, 0, 192));
		g_theRenderer->DrawVertexArray(static_cast<int>(overlayVerts.size()), overlayVerts.data());
	}
}

void Player::DrawReticle()
{
	std::vector<Vertex_PCU> vertexArray;
	AABB2 bounds(792.0f, 392.0f, 808.0, 408.0f);
	AddVertsForAABB2D(vertexArray, bounds, Rgba8::WHITE);

	Texture* reticleTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Reticle.png");
	g_theRenderer->BindTexture(reticleTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)vertexArray.size(), vertexArray.data());
	g_theRenderer->BindTexture(nullptr);
}

void Player::HudTextOverlay(int health, int frags) const
{
	if (health < 0)
	{
		health = 0;
	}
	Rgba8 color = health < 20 ? Rgba8::RED : health < 60 ? Rgba8::GOLD : Rgba8::GREEN;
	if (health > 50.0)
	{
		color = Rgba8::ColorLerp(Rgba8::GOLD, Rgba8::GREEN, ((float)health - 50.0f) * 0.02f);
	}
	else
	{
		color = Rgba8::ColorLerp(Rgba8::RED, Rgba8::GOLD, (float)health * 0.02f);
	}
	std::vector<Vertex_PCU> textVerts;
	AABB2 healthBox(330.0f, 45.0f, 430.0f, 95.0f);
	AABB2 fragBox(590.0f, 45.0f, 650.0f, 95.0f);
	AABB2 grenadeBox(85.0f, 45.0f, 135.0f, 95.0f);

	g_testFont->AddVertsForTextInBox2D(textVerts, healthBox, 50.f, std::to_string(health), color, 0.6f, Vec2(0.5f, 0.5f), SHRINK);
	if (g_theGame->m_numPlayers > 1)
	{
		g_testFont->AddVertsForTextInBox2D(textVerts, fragBox, 50.f, std::to_string(frags), Rgba8::WHITE, 0.6f, Vec2(0.5f, 0.5f), SHRINK);
	}
	g_testFont->AddVertsForTextInBox2D(textVerts, grenadeBox, 50.f, std::to_string(GetActor()->m_grenadeCount), Rgba8::BLUE, 0.6f, Vec2(0.5f, 0.5f), SHRINK);

	g_theRenderer->BindTexture(&g_testFont->GetTexture());
	g_theRenderer->DrawVertexArray(int(textVerts.size()), &textVerts[0]);
	g_theRenderer->BindTexture(0);
}

void Player::DrawMiniMap(IntVec2 origin)
{
	constexpr float mScale = 2.0f;
	std::vector<Vertex_PCU> vertexArray;
	AABB2 bounds((float)origin.x, (float)origin.y, (float)(origin.x + mScale * m_map->m_dimensions.x), (float)(origin.y + mScale * g_theGame->GetNumPlayers() * m_map->m_dimensions.y));
	AddVertsForAABB2D(vertexArray, bounds, Rgba8::WHITE);
	Texture* MiniMapTexture = g_theRenderer->CreateTextureFromImage(m_map->GetMapImage());    //CreateOrGetTextureFromFile((char const*)"Data/Images/Reticle.png");
	g_theRenderer->BindTexture(MiniMapTexture);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);
	g_theRenderer->DrawVertexArray((int)vertexArray.size(), vertexArray.data());

	g_theRenderer->BindTexture(nullptr);
	vertexArray.clear();
	for (int index = 0; index < (int)m_map->m_enemies.size(); index++)
	{
		if (m_map->m_enemies[index]->m_isDead)
		{
			continue;
		}
		Vec2 center(Vec2((float)origin.x, (float)origin.y) + mScale * g_theGame->GetNumPlayers() * Vec2(m_map->m_enemies[index]->m_position.x, m_map->m_enemies[index]->m_position.y));
		AddVertsForDisc2D(vertexArray, center, 1.5f * mScale, m_map->m_enemies[index]->m_definition->m_name == "Boss" ? Rgba8::BLUE : Rgba8::RED);
	}
	for (int index = 0; index < g_theGame->GetNumPlayers(); index++)
	{
		Vec2 center(Vec2((float)origin.x, (float)origin.y) + mScale * g_theGame->GetNumPlayers() * Vec2(g_theGame->GetPlayer(index)->m_position.x, g_theGame->GetPlayer(index)->m_position.y));
		AddVertsForDisc2D(vertexArray, center, 1.5f * mScale, g_theGame->GetPlayer(index) == g_theGame->GetRenderingPlayer() ? Rgba8::GREEN : Rgba8::GREEN);
	}
	g_theRenderer->DrawVertexArray((int)vertexArray.size(), vertexArray.data());
}

void Player::Render()
{
	if (m_freeFlyCameraMode)
	{
		return;
	}

	g_theRenderer->BeginCamera(*m_cameraHud);
	g_theRenderer->SetDepthStencilState(DepthTest::ALWAYS, false);
	g_theRenderer->SetBlendMode(BlendMode::ALPHA);

	HudScreen();
	DrawReticle();
	HudTextOverlay((int)GetActor()->m_health, m_kills);
	DrawMiniMap(IntVec2(1390, g_theGame->GetNumPlayers() > 1 ? 380 : 590));

	Actor* actor = GetActor();
	int weapon = actor->m_equippedWeaponIndex;
	if (actor->m_alternateWeapon > -1)
	{
		weapon = actor->m_alternateWeapon;
	}
	if (weapon >= 0)
	{
		WeaponDefinition const* weaponDefinition = actor->m_weapons[weapon]->m_definition;
		float elapsedFiringTime = (float)(actor->m_actorClock->GetTotalTime() - actor->m_weaponStart);
		SpriteAnimationDefinition const* animDef = nullptr;
		if (actor->m_weapons[weapon]->m_idle)
		{
			if (actor->m_alternateWeapon == -1)
			{
				animDef = weaponDefinition->m_idleAnimationDefinition;
			}
			else
			{
				weapon = actor->m_equippedWeaponIndex;
				animDef = actor->m_weapons[weapon]->m_definition->m_idleAnimationDefinition;
			}
		}
		else
		{
			if (elapsedFiringTime > weaponDefinition->m_attackAnimationDefinition->GetDuration())
			{
				actor->m_weapons[weapon]->m_idle = true;
				actor->m_alternateWeapon = -1;
				animDef = actor->m_weapons[weapon]->m_definition->m_idleAnimationDefinition;
			}
			else
			{
				animDef = weaponDefinition->m_attackAnimationDefinition;
			}
		}

		// get sprite animation at current time
		SpriteDefinition const& spriteDef = animDef->GetSpriteDefAtTime(elapsedFiringTime);
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
		g_theRenderer->SetModelColor(Rgba8::WHITE);

		// set position for sprite animation
		Vec2 spriteSize = weaponDefinition->m_spriteSize;
		if (weaponDefinition->m_name == "Shotgun" || weapon > 2)
		{
			spriteSize *= 2.0f;
		}
		std::vector<Vertex_PCU> vertexArray;
		Vec2 tr = m_cameraHud->GetOrthoTopRight();
		Vec2 bl = m_cameraHud->GetOrthoBottomLeft();
		float left = bl.x + (bl.x + tr.x) * 0.5f - spriteSize.x * 0.5f;
		float right = bl.x + (bl.x + tr.x) * 0.5f + spriteSize.x * 0.5f;
		float bottom = tr.y * FRACTION;
		float top = tr.y * FRACTION + spriteSize.y;
		AABB2 bounds(left, bottom, right, top);

		Vec2 uv_mins;
		Vec2 uv_maxs;
		spriteDef.GetUVs(uv_mins, uv_maxs);
		AddVertsForAABB2D(vertexArray, bounds, Rgba8::WHITE, uv_mins, uv_maxs);

		// render sprite animation
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
		g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
		g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);

		g_theRenderer->SetBlendMode(BlendMode::ALPHA);
		g_theRenderer->DrawVertexArray((int)vertexArray.size(), vertexArray.data());
		g_theRenderer->BindTexture(nullptr);
 	}

	g_theRenderer->EndCamera(*m_cameraHud);
}

Mat44 Player::GetModelMatrix() const
{
	Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(m_position);
	trans.Append(orient);
	return trans;
}

void Player::SixDOF(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith

	// mouse click controls for ray casts /////////////////////////////
	constexpr float durationSeconds = 10.0f;

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE) && m_freeFlyCameraMode)
	{
		if (g_theGame->m_map)
		{
			RaycastResult3D raycastResult = g_theGame->m_map->RaycastAll(m_position, m_orientation.GetForwardNormal(), 10.0f);
			DebugRaycast(raycastResult, durationSeconds, 10.0f);
		}
		else
		{
			g_theConsole->AddLine(DevConsole::TINT_ERROR, "No map loaded!");
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHT_MOUSE) && m_freeFlyCameraMode)
	{
		if (g_theGame->m_map)
		{
			RaycastResult3D raycastResult = g_theGame->m_map->RaycastAll(m_position, m_orientation.GetForwardNormal(), 0.25f);
			DebugRaycast(raycastResult, durationSeconds, 10.0f);
		}
		else
		{
			g_theConsole->AddLine(DevConsole::TINT_ERROR, "No map loaded!");
		}
	}

	if (g_theInput->IsKeyDown('H'))
	{
		m_position = Vec3(3.0f, 3.0f, 3.0f); // reset to origin
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT) || g_theInput->IsKeyDown('Y') || controller.IsButtonPressed(A))
	{
		m_speed = GetActor()->m_definition->m_runSpeed;
	}
	else
	{
		m_speed = GetActor()->m_definition->m_walkSpeed;
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

	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetPostion(m_position);
	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetOrientation(m_orientation);
}
// end 6 DOF //////////////

void Player::Keyboard(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* actor = GetActor();
	if (!actor)
	{
		ERROR_RECOVERABLE("Null actor in Keyboard handler!");
	}
	m_position = actor->m_position;
	m_orientation = actor->m_orientation;
	if (actor->m_animationDuration < static_cast<float>(actor->m_actorClock->GetTotalTime() - m_animationStart))
	{
		actor->m_animation = "Idle";
		m_animationStart = actor->m_actorClock->GetTotalTime();
	}

	// combat-oriented commands
	if (g_theInput->WasKeyJustReleased('1'))
	{
		if (actor->m_weapons.size())
		{
			actor->m_equippedWeaponIndex = 0;
		}
	}

	if (g_theInput->WasKeyJustReleased('2'))
	{
		if (actor->m_weapons.size() > 1)
		{
			actor->m_equippedWeaponIndex = 1;
		}
	}

	if (g_theInput->WasKeyJustReleased('3'))
	{
		if (actor->m_weapons.size() > 2)
		{
			actor->m_equippedWeaponIndex = 2;
		}
	}

	if (g_theInput->WasKeyJustReleased('4'))
	{
		if (actor->m_weapons.size() > 2)
		{
			actor->m_equippedWeaponIndex = 3;
		}
	}

	if (g_theInput->WasKeyJustReleased('5'))
	{
		if (actor->m_weapons.size() > 2)
		{
			actor->m_equippedWeaponIndex = 4;
		}
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFTARROW))
	{
		actor->EquipPreviousWeapon();
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHTARROW))
	{
		actor->EquipNextWeapon();
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_MOUSE))
	{
		// fire weapon
		Weapon* weapon = actor->GetEquippedWeapon();
		if (weapon != nullptr)
		{
			weapon->Fire(actor->m_position, actor->GetForward(), actor);
		}
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		// fire alternate weapon
		Weapon* weapon = actor->GetAllternateWeapon();
		if (weapon != nullptr)
		{
			weapon->Fire(actor->m_position, actor->GetForward(), actor);
		}
	}

	if (g_theInput->IsKeyDown(KEYCODE_LEFT_SHIFT))
	{
		m_speed = GetActor()->m_definition->m_runSpeed;
	}
	else
	{
		m_speed = GetActor()->m_definition->m_walkSpeed;
	}

//	Actor* actor = GetActor();
	Vec3 forward = actor->GetForward();
	Vec3 left = actor->GetLeft();

	if (g_theInput->IsKeyDown('W'))
	{
		GetActor()->MoveInDirection(forward, m_speed);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		GetActor()->MoveInDirection(forward, -m_speed);
	}

	if (g_theInput->IsKeyDown('A'))
	{
		GetActor()->MoveInDirection(left, m_speed);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		GetActor()->MoveInDirection(left, -m_speed);
	}

	// Euler angle orientation
	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + mouseDelta.y / 2.0f, -85.0, 85.0);

	// yaw
	m_orientation.m_yawDegrees -= (mouseDelta.x / 2.0f);

	Vec3 eyeLevel(m_position.x, m_position.y, GetActor()->m_definition->m_eyeHeight); // force correct eye level no matter what else happens
	//	Vec3 eyeLevel(m_position.x, m_position.y, 0.5f); // force correct eye level no matter what else happens
	//m_position.z = GetActor()->m_definition->m_eyeHeight;
	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetPostion(eyeLevel);
	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetOrientation(m_orientation);
	GetActor()->m_position = m_position;
	GetActor()->m_orientation = m_orientation;
}

void Player::Controller(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* actor = GetActor();
	if (!actor)
	{
		ERROR_RECOVERABLE("Null actor in controller handler!");
	}
	m_position = actor->m_position;
	m_orientation = actor->m_orientation;
	if (actor->m_animationDuration < static_cast<float>(actor->m_actorClock->GetTotalTime() - m_animationStart))
	{
		actor->m_animation = "Idle";
		m_animationStart = actor->m_actorClock->GetTotalTime();
	}

	XboxController controller = g_theInput->GetController(0); // leap of faith
															  // combat-oriented commands
	if (controller.WasButtonJustReleased(X) || controller.WasButtonJustReleased(DPAD_UP))
	{
		if (actor->m_weapons.size())
		{
			actor->m_equippedWeaponIndex = 0;
		}
	}

	if (controller.WasButtonJustReleased(Y) || controller.WasButtonJustReleased(DPAD_RIGHT))
	{
		if (actor->m_weapons.size() > 1)
		{
			actor->m_equippedWeaponIndex = 1;
		}
	}

	if (controller.WasButtonJustReleased(B) || controller.WasButtonJustReleased(DPAD_DOWN))
	{
		if (actor->m_weapons.size() > 2)
		{
			actor->m_equippedWeaponIndex = 2;
		}
	}

	if (controller.WasButtonJustReleased(DPAD_LEFT))
	{
		if (actor->m_weapons.size() > 3)
		{
			actor->m_equippedWeaponIndex = 3;
		}
	}

	if (controller.WasButtonJustReleased(LEFT_SHOULDER))
	{
		actor->EquipPreviousWeapon();
	}

	if (controller.WasButtonJustReleased(RIGHT_SHOULDER))
	{
		actor->EquipNextWeapon();
	}

	if (controller.GetRightTrigger() > 0.25f)
	{
		// fire weapon
		Weapon* weapon = actor->GetEquippedWeapon();
		if (weapon != nullptr)
		{
			weapon->Fire(actor->m_position, actor->GetForward(), actor);
		}
	}

	if (controller.GetLeftTrigger() > 0.25f)
	{
		// fire alternate weapon
		Weapon* weapon = actor->GetAllternateWeapon();
		if (weapon != nullptr)
		{
			weapon->Fire(actor->m_position, actor->GetForward(), actor);
		}
	}

	if (controller.IsButtonPressed(A))
	{
		m_speed = GetActor()->m_definition->m_runSpeed;
	}
	else
	{
		m_speed = GetActor()->m_definition->m_walkSpeed;
	}

	//	Actor* actor = GetActor();
	Vec3 forward = actor->GetForward();
	Vec3 left = actor->GetLeft();

	if (controller.GetLeftStick().GetPosition().y > 0.1f)
	{
		GetActor()->MoveInDirection(forward, m_speed);
	}
	if (controller.GetLeftStick().GetPosition().y < -0.1f)
	{
		GetActor()->MoveInDirection(forward, -m_speed);
	}

	if (controller.GetLeftStick().GetPosition().x < -0.1f)
	{
		GetActor()->MoveInDirection(left, m_speed);
	}
	if (controller.GetLeftStick().GetPosition().x > 0.1f)
	{
		GetActor()->MoveInDirection(left, -m_speed);
	}

	// Euler angle orientation
	// pitch
	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + controller.GetRightStick().GetPosition().y, -85.0, 85.0);

	// yaw
	m_orientation.m_yawDegrees -= (controller.GetRightStick().GetPosition().x);

	Vec3 eyeLevel(m_position.x, m_position.y, GetActor()->m_definition->m_eyeHeight); // force correct eye level no matter what else happens
																					  //	Vec3 eyeLevel(m_position.x, m_position.y, 0.5f); // force correct eye level no matter what else happens
																					  //m_position.z = GetActor()->m_definition->m_eyeHeight;
	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetPostion(eyeLevel);
	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetOrientation(m_orientation);
	GetActor()->m_position = m_position;
	GetActor()->m_orientation = m_orientation;
}
