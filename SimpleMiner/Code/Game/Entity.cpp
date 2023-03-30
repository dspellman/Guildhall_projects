#include "Game/Entity.hpp"
#include "Game/Game.hpp"
#include "BlockIterator.hpp"

Entity::~Entity()
{

}

Entity::Entity(Controls control)
	: m_contol(control)
{

}

void Entity::SetSizeAABB3(AABB3 bounds, float eyeLevel)
{
	m_bounds = bounds;
	Vec3 center = bounds.GetCenter();
	center.z = 0.0f;
	m_bounds.Translate(-center);
	m_eyeLevel = eyeLevel;
}

void Entity::CreateGeometry()
{
	AABB3 bounds = m_bounds;
	bounds.Translate(m_position);
	AddVertsForAABB3D(m_vertexes, bounds, Rgba8::CYAN);
}

//----------------------------------------------------------------------
void Entity::CyclePhysics()
{
	switch (m_physics)
	{
	case WALKING:
		m_physics = FLYING;
		break;
	case FLYING:
		m_physics = NOCLIP;
		break;
	case NOCLIP:
		m_physics = WALKING;
		break;
	default:
		m_physics = WALKING;
		break;
	}
}

//----------------------------------------------------------------------
void Entity::CycleCameraView()
{
	switch (m_cameraView)
	{
	case SPECTATOR:
		m_cameraView = INDEPENDENT;
		break;
	case INDEPENDENT:
		m_cameraView = FIRST_PERSON;
		break;
	case FIRST_PERSON:
		m_cameraView = FIXED_ANGLE_TRACKING;
		break;
	case FIXED_ANGLE_TRACKING:
		m_cameraView = OVER_SHOULDER;
		break;
	case OVER_SHOULDER:
		m_cameraView = SPECTATOR;
		break;
	default:
		m_cameraView = SPECTATOR;
		break;
	}
}

//----------------------------------------------------------------------
void Entity::Update(float deltaSeconds)
{
	switch (m_cameraView)
	{
	case SPECTATOR: // mode 1
		if (m_UseAxialCameraControls)
		{
			Axial(deltaSeconds);
		}
		else
		{
			SixDOF(deltaSeconds);
		}
	break;
	case INDEPENDENT: // mode 2
		Keyboard(deltaSeconds);
		break;
	case FIRST_PERSON: // mode 3
		Keyboard(deltaSeconds);
		break;
	case FIXED_ANGLE_TRACKING: // mode 4
		Keyboard(deltaSeconds);
		break;
	case OVER_SHOULDER: // mode 5
		Keyboard(deltaSeconds);
		break;
	default:
		if (m_UseAxialCameraControls)
		{
			Axial(deltaSeconds);
		}
		else
		{
			SixDOF(deltaSeconds);
		}
		break;
	}

	// Do raycast from camera position
	if (!m_rayFrozen)
	{
		Vec3 eyePosition(m_position.x, m_position.y, m_position.z + m_eyeLevel);
		BlockIterator cameraBlockIterator(eyePosition);
		m_raycastHit = cameraBlockIterator.RaycastVsBlocksFlawless(eyePosition, m_orientation.GetForwardNormal(), MAX_RAYCAST_DISTANCE);
		m_raycastStart = eyePosition;
		m_raycastFwdNormal = m_orientation.GetForwardNormal();
	}
	if (g_theInput->WasKeyJustReleased('R'))
	{
		m_rayFrozen = !m_rayFrozen;
	}

	// Test block selection keys
	for (uint8_t key = 1; key <= 9; key++)
	{
		if (g_theInput->WasKeyJustReleased('0' + key))
		{
			m_blockSelection = key;
		}
	}
}

//----------------------------------------------------------------------
void Entity::UpdatePhysics(float deltaSeconds)
{
	if (m_physics == WALKING)
	{
		AddForce(Vec3(0.0f, 0.0f, -1.0f) * GRAVITY);
		AddForce(-m_velocity * m_drag);
	}
	else
	{
		AddForce(-m_velocity * m_drag * 0.1f);
	}
	m_velocity += m_acceleration * deltaSeconds;

	float length = m_velocity.GetLength();
// 	if (m_flying == false)
// 	{
// 		m_velocity.z = 0.0f;
// 	}
	m_velocity = m_velocity.GetNormalized() * length;
// 	m_position += m_velocity * deltaSeconds;

	Vec3 speedTest = m_velocity;
	speedTest.z = 0.0f;
	if (speedTest.GetLength() > m_maxSpeed)
	{
		speedTest.GetClamped(m_maxSpeed);
		m_velocity.x = speedTest.x;
		m_velocity.y = speedTest.y;
	}

	if (m_physics == NOCLIP)
	{
		m_position += m_velocity * deltaSeconds;
	}
	else
	{
		Vec3 maxMove = PreventativePhysicsTest(deltaSeconds);
		m_position += maxMove;
	}
	if (m_position.z < 0.0f)
	{
		m_position.z = 0.0f;
	}
	if (m_position.z > 126.0f)
	{
		m_position.z = 126.0f;
	}

	m_acceleration = Vec3::ZERO;
}

//----------------------------------------------------------------------
// Vec3 Entity::PreventtativePhysicsTest(float deltaSeconds)
// {
// 	Vec3 testPoint[12];
// 	testPoint[0] =  m_position + m_bounds.m_mins;
// 	testPoint[1] =  m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, m_bounds.m_mins.z);
// 	testPoint[2] =  m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);
// 	testPoint[3] =  m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);
// 	
// 	testPoint[4] =  m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
// 	testPoint[5] =  m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
// 	testPoint[6] =  m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
// 	testPoint[7] =  m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
// 	
// 	testPoint[8] =  m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, m_bounds.m_maxs.z);
// 	testPoint[9] =  m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, m_bounds.m_maxs.z);
// 	testPoint[10] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, m_bounds.m_maxs.z);
// 	testPoint[11] = m_position + m_bounds.m_maxs;
// 
// 	Vec3 allowedMove = m_velocity;
// 	float fraction = 1.0f;
// 	RaycastHit closestHit;
// 	RaycastHit testHit;
// 
// 	Vec3 verticalNormal = m_velocity;
// 	verticalNormal.x = 0.0f;
// 	verticalNormal.y = 0.0f;
// 	verticalNormal.Normalize();
// 	float zDistance = fabsf(m_velocity.z) * deltaSeconds;
// 	closestHit.m_distance = 2.0f * zDistance;
// 	closestHit.m_hit = false;
// 
// 	// test vertical collision for bottom vertices
// 	for (int index = 0; index < 4; index++)
// 	{
// 		BlockIterator testBlock(testPoint[index]);
// 		testHit = testBlock.RaycastVsBlocksFlawless(testPoint[index], verticalNormal, zDistance);
// 		if (testHit.m_self)
// 		{
// 			ERROR_RECOVERABLE("Need vertical corrective physics!");
// 		}
// 		if (testHit.m_hit)
// 		{
// 			if (testHit.m_distance < closestHit.m_distance)
// 			{
// 				closestHit = testHit;
// 			}
// 		}
// 	}
// 
//  	if (closestHit.m_hit)
// 	{
// 		fraction = closestHit.m_distance / (zDistance + TOLERANCE);
// 		allowedMove.z *= fraction;
// 	}
// 	// ADD Z MOVE TO POINTS FOR TESTING ACCURATELY!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// 	// SPlit X and Y move tests so not stuck on walls
// 	// different test of buffer--points, not m_position when added
// 
// 	Vec3 horizontalNormal = m_velocity;
// 	horizontalNormal.z = 0.0f; // ignore z component if we are at ground level
// 	horizontalNormal.Normalize();
// 	Vec3 xyPlane = m_velocity;
// 	xyPlane.z = 0.0f;
// 	float xyDistance = xyPlane.GetLength() * deltaSeconds;
// 	closestHit.m_distance = 2.0f * xyDistance;
// 	closestHit.m_hit = false;
// 
// 	// test horizontal collision for all vertices
// 	for (int index = 0; index < 12; index++)
// 	{
// 		BlockIterator testBlock(testPoint[index]);
// 		testHit = testBlock.RaycastVsBlocksFlawless(testPoint[index], horizontalNormal, xyDistance);
// 		if (testHit.m_self)
// 		{
// 			ERROR_RECOVERABLE("Need corrective physics!");
// 		}
// 		if (testHit.m_hit)
// 		{
// 			if (testHit.m_distance < closestHit.m_distance)
// 			{
// 				closestHit = testHit;
// 			}
// 		}
// 	}
// 
// 	if (closestHit.m_hit)
// 	{
// 		fraction = closestHit.m_distance / (xyDistance + TOLERANCE);
// 		allowedMove.x *= fraction;
// //		float testx = fabsf(fmodf(m_position.x + allowedMove.x * deltaSeconds, 1.0f) - 0.5f);
// 		float testx = fabsf(fmodf(m_position.x + allowedMove.x * deltaSeconds, 1.0f));
// 		if (testx > 0.9f || testx < 0.1f)
// 		{
// 			allowedMove.x = 0.0f;
// 		}
// 		allowedMove.y *= fraction;
// //		float testy = fabsf(fmodf(m_position.y + allowedMove.y * deltaSeconds, 1.0f) - 0.5f);
// 		float testy = fabsf(fmodf(m_position.y + allowedMove.y * deltaSeconds, 1.0f));
// 		if (testy > 0.9f || testy < 0.1f)
// 		{
// 			allowedMove.y = 0.0f;
// 		}
// 	}
// 
// 	return allowedMove * deltaSeconds; // move as far in each dimension as allowed by collision constraints
// }

//----------------------------------------------------------------------
Vec3 Entity::PreventativePhysicsTest(float deltaSeconds)
{
	m_isGrounded = false; // this will be updated every time physics is calculated

	Vec3 testPoint[12];
	testPoint[0] = m_position + m_bounds.m_mins;
	testPoint[1] = m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, m_bounds.m_mins.z);
	testPoint[2] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);
	testPoint[3] = m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, m_bounds.m_mins.z);

	testPoint[4] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
	testPoint[5] = m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
	testPoint[6] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);
	testPoint[7] = m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_maxs.y, (m_bounds.m_mins.z + m_bounds.m_maxs.z) * 0.5f);

	testPoint[8] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_mins.y, m_bounds.m_maxs.z);
	testPoint[9] = m_position + Vec3(m_bounds.m_maxs.x, m_bounds.m_mins.y, m_bounds.m_maxs.z);
	testPoint[10] = m_position + Vec3(m_bounds.m_mins.x, m_bounds.m_maxs.y, m_bounds.m_maxs.z);
	testPoint[11] = m_position + m_bounds.m_maxs;

	Vec3 allowedMove = m_velocity;
	RaycastHit closestHit;
	RaycastHit testHit;
	Vec3 normal = m_velocity.GetNormalized();
	float maxDistance = m_velocity.GetLength() * deltaSeconds;

	// test collision for all vertices with current axes and remove any axis that hits
	for (int axes = 3; axes > 0; axes--)
	{
		closestHit.m_distance = 2.0f * maxDistance;
		closestHit.m_hit = false;
		for (int index = 0; index < 12; index++)
		{
			BlockIterator testBlock(testPoint[index]);
			testHit = testBlock.RaycastVsBlocksFlawless(testPoint[index], normal, maxDistance);
			if (testHit.m_self)
			{
				return Vec3(0.0f, 0.0f, 1.0f); // corrective physics is to pop up a block until free
			}
			if (testHit.m_hit)
			{
				if (testHit.m_distance < closestHit.m_distance)
				{
					closestHit = testHit;
				}
			}
		}

		if (closestHit.m_hit)
		{
			if (closestHit.m_impactSurfaceNormal.z != 0.0f)
			{
				if (allowedMove.z < 0.0f)
				{
					m_isGrounded = true;
				}
				allowedMove.z = 0.0f;
				normal.z = 0.0f;
			}		
			if (closestHit.m_impactSurfaceNormal.y != 0.0f)
			{
				allowedMove.y = 0.0f;
				normal.y = 0.0f;
			}		
			if (closestHit.m_impactSurfaceNormal.x != 0.0f)
			{
				allowedMove.x = 0.0f;
				normal.x = 0.0f;
			}
			maxDistance = allowedMove.GetLength() * deltaSeconds;
			normal.Normalize();
		}
		else
		{
			return allowedMove * deltaSeconds; // move as far in each dimension as allowed by collision constraints
		}
	}

	return Vec3::ZERO; // this should be zero since all axes apparently hit
}

//----------------------------------------------------------------------
void Entity::UpdateCameras()
{
	BlockIterator blockHead(Vec3(m_position.x, m_position.y, m_position.z + m_eyeLevel));
	RaycastHit los;
	EulerAngles losAngle(40.0f, -30.0f, 0.0f);
	Vec3 eyePosition(m_position.x, m_position.y, m_position.z + m_eyeLevel);

	switch (m_cameraView)
	{
	case SPECTATOR: // mode 1
		break;
	case INDEPENDENT: // mode 2
		break;
	case FIRST_PERSON: // mode 3
		m_cameraPosition = Vec3(m_position.x, m_position.y, m_position.z + m_eyeLevel);
		m_cameraOrientation = m_orientation;
		g_theGame->m_worldCamera.SetPostion(m_cameraPosition);
		g_theGame->m_worldCamera.SetOrientation(m_cameraOrientation);
		break;
	case FIXED_ANGLE_TRACKING: // mode 4
		los = blockHead.RaycastVsBlocksFlawless(eyePosition, losAngle.GetForwardNormal(), FIXED_DISTANCE);
		if (los.m_hit)
		{
			m_cameraPosition = eyePosition + losAngle.GetForwardNormal() * (los.m_distance - 0.15f);
		}
		else
		{
			m_cameraPosition = eyePosition + losAngle.GetForwardNormal() * FIXED_DISTANCE;
		}
		m_cameraOrientation = -losAngle.GetForwardNormal();
		g_theGame->m_worldCamera.SetPostion(m_cameraPosition);
		g_theGame->m_worldCamera.SetOrientation(m_cameraOrientation);
		break;
	case OVER_SHOULDER: // mode 5
		los = blockHead.RaycastVsBlocksFlawless(eyePosition, -m_orientation.GetForwardNormal(), 4.0f);
		if (los.m_hit)
		{
			m_cameraPosition = eyePosition - m_orientation.GetForwardNormal() * (los.m_distance - 0.15f);
		}
		else
		{
			m_cameraPosition = eyePosition - m_orientation.GetForwardNormal() * 4.0f;
		}
		m_cameraOrientation = m_orientation;
		g_theGame->m_worldCamera.SetPostion(m_cameraPosition);
		g_theGame->m_worldCamera.SetOrientation(m_cameraOrientation);
		break;
	}
}

//----------------------------------------------------------------------
void Entity::Render()
{
	std::vector<Vertex_PCU> vertexArray;
	// draw hit normal if needed
	if (m_raycastHit.m_hit)
	{
 		if (m_cameraView != FIRST_PERSON)
		{
			AddVertsForCylinder3D(vertexArray, m_raycastStart, m_raycastHit.m_hitPoint, 0.01f, Rgba8::RED);
			AddVertsForCylinder3D(vertexArray, m_raycastHit.m_hitPoint, m_raycastStart + MAX_RAYCAST_DISTANCE * m_raycastFwdNormal, 0.01f, Rgba8::GRAY);
		}
		AddVertsForSphere(vertexArray, m_raycastHit.m_hitPoint, EulerAngles::ZERO, 0.03f);
		AddVertsForCylinder3D(vertexArray, m_raycastHit.m_hitPoint, m_raycastHit.m_hitPoint + 0.35f * m_raycastHit.m_impactSurfaceNormal, 0.02f, Rgba8::BLUE);
	}
	else if (m_cameraView != FIRST_PERSON)
	{
		AddVertsForCylinder3D(vertexArray, m_raycastStart, m_raycastStart + MAX_RAYCAST_DISTANCE * m_raycastFwdNormal, 0.01f, Rgba8::RED);
	}

	if (vertexArray.size())
	{
		g_theRenderer->BindTexture(nullptr);
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
		g_theRenderer->DrawVertexArray(int(vertexArray.size()), &vertexArray[0]);
		vertexArray.clear();
	}

	// draw player
	if (m_cameraView != FIRST_PERSON)
	{
		m_vertexes.clear();
		Vec3 i, j, k;
		m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
		Vec3 wcPos = m_position;
		AddVertsForCylinder3D(m_vertexes, wcPos, wcPos + Vec3(0.3f, 0.0f, 0.0f), 0.02f, Rgba8::RED);
		AddVertsForCylinder3D(m_vertexes, wcPos, wcPos + Vec3(0.0f, 0.3f, 0.0f), 0.02f, Rgba8::GREEN);
		AddVertsForCylinder3D(m_vertexes, wcPos, wcPos + Vec3(0.0f, 0.0f, 0.3f), 0.02f, Rgba8::BLUE);
		if (m_vertexes.size())
		{
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->DrawVertexArray(int(m_vertexes.size()), &m_vertexes[0]);
		}

		m_vertexes.clear();
		CreateGeometry();
		if (m_vertexes.size())
		{
			g_theRenderer->BindTexture(nullptr);
			g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
			g_theRenderer->DrawVertexArray(int(m_vertexes.size()), &m_vertexes[0]);
		}
	}
}

//----------------------------------------------------------------------
void Entity::SixDOF(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		Dig();
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHT_MOUSE))
	{
		Place(m_blockSelection);
	}

// 	if (g_theInput->IsKeyDown('H'))
// 	{
// 		m_cameraPosition = Vec3(-3.0f, -3.0f, -3.0f); // reset near origin
// 	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || controller.WasButtonJustPressed(A))
	{
		m_speed *= 10.0f;
		m_rotate *= 2.0f;
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE) || controller.WasButtonJustReleased(A))
	{
		m_speed *= 0.1f;
		m_rotate *= 0.5f;
	}

	// Translate position
	Vec3 i;
	Vec3 j;
	Vec3 k;
	m_cameraOrientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);

	if (g_theInput->IsKeyDown('W') || controller.GetLeftStick().GetPosition().y > 0.1f)
	{
		m_cameraPosition += m_speed * deltaSeconds * i;
	}
	if (g_theInput->IsKeyDown('S') || controller.GetLeftStick().GetPosition().y < -0.1f)
	{
		m_cameraPosition += -1.0f * m_speed * deltaSeconds * i;
	}

	if (g_theInput->IsKeyDown('A') || controller.GetLeftStick().GetPosition().x < -0.1f)
	{
		m_cameraPosition += m_speed * deltaSeconds * j;
	}
	if (g_theInput->IsKeyDown('D') || controller.GetLeftStick().GetPosition().x > 0.1f)
	{
		m_cameraPosition += -1.0f * m_speed * deltaSeconds * j;
	}
	// up down
	if (g_theInput->IsKeyDown('Q') || controller.GetButton(RIGHT_SHOULDER).isKeyPressed)
	{
		m_cameraPosition += m_speed * deltaSeconds * k;
	}
	if (g_theInput->IsKeyDown('E') || controller.GetButton(LEFT_SHOULDER).isKeyPressed)
	{
		m_cameraPosition += -1.0f * m_speed * deltaSeconds * k;
	}

	// Euler angle orientation
	// roll
	if (g_theInput->IsKeyDown('Z') || controller.GetLeftTrigger() > 0.1f)
	{
		EulerAngles roll(0.0f, 0.0f, -1.0f * m_rotate * deltaSeconds);
		m_cameraOrientation += roll;
	}
	if (g_theInput->IsKeyDown('C') || controller.GetRightTrigger() > 0.1f)
	{
		EulerAngles roll(0.0f, 0.0f, m_rotate * deltaSeconds);
		m_cameraOrientation += roll;
	}
	m_cameraOrientation.m_rollDegrees = Clamp(m_cameraOrientation.m_rollDegrees, -45.0, 45.0);

	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_cameraOrientation.m_pitchDegrees = Clamp(m_cameraOrientation.m_pitchDegrees + mouseDelta.y / 2.0f + controller.GetRightStick().GetPosition().y, -85.0, 85.0);

	// yaw
	m_cameraOrientation.m_yawDegrees -= (mouseDelta.x / 2.0f + controller.GetRightStick().GetPosition().x);

	g_theGame->m_worldCamera.SetPostion(m_cameraPosition);
	g_theGame->m_worldCamera.SetOrientation(m_cameraOrientation);
} // SixDOF

  //----------------------------------------------------------------------
void Entity::Axial(float deltaSeconds)
{
	XboxController controller = g_theInput->GetController(0); // leap of faith

	if (g_theInput->WasKeyJustReleased(KEYCODE_LEFT_MOUSE))
	{
		Dig();
	}

	if (g_theInput->WasKeyJustReleased(KEYCODE_RIGHT_MOUSE))
	{
		Place(m_blockSelection);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE) || controller.WasButtonJustPressed(A))
	{
		m_speed *= 10.0f;
		m_rotate *= 0.5f;
	}
	if (g_theInput->WasKeyJustReleased(KEYCODE_SPACE) || controller.WasButtonJustReleased(A))
	{
		m_speed *= 0.1f;
		m_rotate *= 2.0f;
	}

	// Translate position
	Vec3 i;
	Vec3 j;
	Vec3 k;
	//m_cameraOrientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);

	if (g_theInput->IsKeyDown('W') || controller.GetLeftStick().GetPosition().y > 0.1f)
	{
		m_cameraPosition.x += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('S') || controller.GetLeftStick().GetPosition().y < -0.1f)
	{
		m_cameraPosition.x += -1.0f * m_speed * deltaSeconds;
	}

	if (g_theInput->IsKeyDown('A') || controller.GetLeftStick().GetPosition().x < -0.1f)
	{
		m_cameraPosition.y += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('D') || controller.GetLeftStick().GetPosition().x > 0.1f)
	{
		m_cameraPosition.y += -1.0f * m_speed * deltaSeconds;
	}
	// up down
	if (g_theInput->IsKeyDown('Q') || controller.GetButton(RIGHT_SHOULDER).isKeyPressed)
	{
		m_cameraPosition.z += m_speed * deltaSeconds;
	}
	if (g_theInput->IsKeyDown('E') || controller.GetButton(LEFT_SHOULDER).isKeyPressed)
	{
		m_cameraPosition.z += -1.0f * m_speed * deltaSeconds;
	}

	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_cameraOrientation.m_pitchDegrees = Clamp(m_cameraOrientation.m_pitchDegrees + mouseDelta.y / 2.0f + controller.GetRightStick().GetPosition().y, -89.9f, 89.9f);

	// yaw
	m_cameraOrientation.m_yawDegrees -= (mouseDelta.x / 2.0f + controller.GetRightStick().GetPosition().x);

	g_theGame->m_worldCamera.SetPostion(m_cameraPosition);
	g_theGame->m_worldCamera.SetOrientation(m_cameraOrientation);
} // Axial

  //----------------------------------------------------------------------
void Entity::Keyboard(float deltaSeconds)
{
	UNUSED(deltaSeconds);

	if (g_theInput->WasKeyJustPressed(KEYCODE_LEFT_MOUSE))
	{
		Dig();
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_RIGHT_MOUSE))
	{
		Place(m_blockSelection);
	}

	Vec3 forward = GetForward();
	forward.z = 0.0f;
	forward.Normalize();
	Vec3 left = GetLeft();
	left.z = 0.0f;
	left.Normalize();

	// forward backward
	if (g_theInput->IsKeyDown('W'))
	{
		MoveInDirection(forward, m_speed);
	}
	if (g_theInput->IsKeyDown('S'))
	{
		MoveInDirection(forward, -m_speed);
	}
	// right left
	if (g_theInput->IsKeyDown('A'))
	{
		MoveInDirection(left, m_speed);
	}
	if (g_theInput->IsKeyDown('D'))
	{
		MoveInDirection(left, -m_speed);
	}
	// up down
	if (g_theInput->IsKeyDown('Q'))
	{
		MoveInDirection(Vec3(0.0f, 0.0f, 1.0f), m_speed);
	}
	if (g_theInput->IsKeyDown('E'))
	{
		MoveInDirection(Vec3(0.0f, 0.0f, 1.0f), -m_speed);
	}

	if (g_theInput->WasKeyJustPressed(KEYCODE_SPACE))
	{
		if (m_isGrounded && m_physics == WALKING)
		{
			AddImpulse(Vec3(0.0f, 0.0f, JUMP_IMPULSE));
		}
	}

	// Euler angle orientation
	Vec2 mouseDelta = g_theInput->GetMouseClientDelta();
	// pitch
	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + mouseDelta.y / 2.0f, -85.0, 85.0);
	// yaw
	m_orientation.m_yawDegrees -= (mouseDelta.x / 2.0f);
}

// void Entity::Controller(float deltaSeconds)
// {
// 	UNUSED(deltaSeconds);
// 	Actor* actor = GetActor();
// 	if (!actor)
// 	{
// 		ERROR_RECOVERABLE("Null actor in controller handler!");
// 	}
// 	m_position = actor->m_position;
// 	m_orientation = actor->m_orientation;
// 	if (actor->m_animationDuration < static_cast<float>(actor->m_actorClock->GetTotalTime() - m_animationStart))
// 	{
// 		actor->m_animation = "Idle";
// 		m_animationStart = actor->m_actorClock->GetTotalTime();
// 	}
// 
// 	XboxController controller = g_theInput->GetController(0); // leap of faith
// 															  // combat-oriented commands
// 	if (controller.WasButtonJustReleased(X) || controller.WasButtonJustReleased(DPAD_UP))
// 	{
// 		if (actor->m_weapons.size())
// 		{
// 			actor->m_equippedWeaponIndex = 0;
// 		}
// 	}
// 
// 	if (controller.WasButtonJustReleased(Y) || controller.WasButtonJustReleased(DPAD_RIGHT))
// 	{
// 		if (actor->m_weapons.size() > 1)
// 		{
// 			actor->m_equippedWeaponIndex = 1;
// 		}
// 	}
// 
// 	if (controller.WasButtonJustReleased(B) || controller.WasButtonJustReleased(DPAD_DOWN))
// 	{
// 		if (actor->m_weapons.size() > 2)
// 		{
// 			actor->m_equippedWeaponIndex = 2;
// 		}
// 	}
// 
// 	if (controller.WasButtonJustReleased(DPAD_LEFT))
// 	{
// 		if (actor->m_weapons.size() > 3)
// 		{
// 			actor->m_equippedWeaponIndex = 3;
// 		}
// 	}
// 
// 	if (controller.WasButtonJustReleased(LEFT_SHOULDER))
// 	{
// 		actor->EquipPreviousWeapon();
// 	}
// 
// 	if (controller.WasButtonJustReleased(RIGHT_SHOULDER))
// 	{
// 		actor->EquipNextWeapon();
// 	}
// 
// 	if (controller.GetRightTrigger() > 0.25f)
// 	{
// 
// 	}
// 
// 	if (controller.GetLeftTrigger() > 0.25f)
// 	{
// 
// 	}
// 
// 	if (controller.IsButtonPressed(A))
// 	{
// 		m_speed = GetActor()->m_definition->m_runSpeed;
// 	}
// 	else
// 	{
// 		m_speed = GetActor()->m_definition->m_walkSpeed;
// 	}
// 
// 	//	Actor* actor = GetActor();
// 	Vec3 forward = actor->GetForward();
// 	Vec3 left = actor->GetLeft();
// 
// 	if (controller.GetLeftStick().GetPosition().y > 0.1f)
// 	{
// 		GetActor()->MoveInDirection(forward, m_speed);
// 	}
// 	if (controller.GetLeftStick().GetPosition().y < -0.1f)
// 	{
// 		GetActor()->MoveInDirection(forward, -m_speed);
// 	}
// 
// 	if (controller.GetLeftStick().GetPosition().x < -0.1f)
// 	{
// 		GetActor()->MoveInDirection(left, m_speed);
// 	}
// 	if (controller.GetLeftStick().GetPosition().x > 0.1f)
// 	{
// 		GetActor()->MoveInDirection(left, -m_speed);
// 	}
// 
// 	// Euler angle orientation
// 	// pitch
// 	m_orientation.m_pitchDegrees = Clamp(m_orientation.m_pitchDegrees + controller.GetRightStick().GetPosition().y, -85.0, 85.0);
// 
// 	// yaw
// 	m_orientation.m_yawDegrees -= (controller.GetRightStick().GetPosition().x);
// 
// 	Vec3 eyeLevel(m_position.x, m_position.y, GetActor()->m_definition->m_eyeHeight); // force correct eye level no matter what else happens
// 																					  //	Vec3 eyeLevel(m_position.x, m_position.y, 0.5f); // force correct eye level no matter what else happens
// 																					  //m_position.z = GetActor()->m_definition->m_eyeHeight;
// 	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetPostion(eyeLevel);
// 	g_theGame->m_player[m_playerIndex]->m_cameraWorld->SetOrientation(m_orientation);
// 	GetActor()->m_position = m_position;
// 	GetActor()->m_orientation = m_orientation;
// }

//----------------------------------------------------------------------
void Entity::Dig()
{
	if (!m_raycastHit.m_hit)
	{
		return;
	}

	BlockIterator block = m_raycastHit.blockIterator;
	block.m_chunk->SetBlock(block.m_x, block.m_y, block.m_z, AIR);
	block.m_chunk->m_needsMesh = true;
	block.m_chunk->m_dirty = true;
	// test if on chunk boundary and dirty adjacent chunk if so
	block.m_chunk->TestNeighborNeedsMesh(block.m_x, block.m_y);

	// set sky flags if open to sky
	BlockIterator blockIterator = block;
	g_theGame->m_world->MarkLightingDirty(blockIterator);
	if (blockIterator.GetTopNeighbor().GetBlock() && blockIterator.GetTopNeighbor().GetBlock()->IsSky())
	{
		while (blockIterator.GetBlock() && blockIterator.GetBlock()->IsOpaque() == false)
		{
			blockIterator.GetBlock()->SetSky(true);
			g_theGame->m_world->MarkLightingDirty(blockIterator);
			blockIterator = blockIterator.GetBottomNeighbor();
		}
	}
}

//----------------------------------------------------------------------
void Entity::Place(uint8_t blockType)
{
	if (!m_raycastHit.m_hit)
	{
		return;
	}

	BlockIterator block = GetAdjacentBlockByNormal(m_raycastHit);

	block.m_chunk->SetBlock(block.m_x, block.m_y, block.m_z, blockType);
	block.m_chunk->m_needsMesh = true;
	block.m_chunk->m_dirty = true;
	// test if on chunk boundary and dirty adjacent chunk if so
	block.m_chunk->TestNeighborNeedsMesh(block.m_x, block.m_y);

	// reset sky flags if open to sky
	BlockIterator blockIterator = block;
	g_theGame->m_world->MarkLightingDirty(blockIterator);

	if (blockIterator.GetBlock()->IsSky())
	{
		while (blockIterator.GetBlock() && blockIterator.GetBlock()->IsOpaque() == false)
		{
			blockIterator.GetBlock()->SetSky(false);
			g_theGame->m_world->MarkLightingDirty(blockIterator);
			blockIterator = blockIterator.GetBottomNeighbor();
		}
	}
}

//----------------------------------------------------------------------
BlockIterator Entity::GetAdjacentBlockByNormal(RaycastHit hit)
{
	BlockIterator block = hit.blockIterator;
	if (hit.m_impactSurfaceNormal.x < 0.0f)
	{
		return block.GetWestNeighbor();
	}
	if (hit.m_impactSurfaceNormal.x > 0.0f)
	{
		return block.GetEastNeighbor();
	}

	if (hit.m_impactSurfaceNormal.y < 0.0f)
	{
		return block.GetSouthNeighbor();
	}
	if (hit.m_impactSurfaceNormal.y > 0.0f)
	{
		return block.GetNorthNeighbor();
	}

	if (hit.m_impactSurfaceNormal.z < 0.0f)
	{
		return block.GetBottomNeighbor();
	}
	if (hit.m_impactSurfaceNormal.z > 0.0f)
	{
		return block.GetTopNeighbor();
	}
	return block; // this would be an error but at least its something
}

//----------------------------------------------------------------------
Vec3 Entity::GetForward() const
{
	return m_orientation.GetForwardNormal();
}

Vec3 Entity::GetLeft() const
{
	Vec3 i;
	Vec3 j;
	Vec3 k;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	return j;
}

Vec3 Entity::GetUp() const
{
	Vec3 i;
	Vec3 j;
	Vec3 k;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	return k;
}

void Entity::MoveInDirection(Vec3 direction, float speed)
{
	AddForce(direction * speed * m_drag);
}

void Entity::AddForce(const Vec3& force)
{
	m_acceleration += force;
}

void Entity::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}
