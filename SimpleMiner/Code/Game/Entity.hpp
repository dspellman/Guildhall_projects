#pragma once
#include "Game/GameCommon.hpp"
#include "BlockIterator.hpp"

enum Controls
{
	STATIC,
	KEYBOARD_XBOX,
	KEYBOARD,
	XBOX,
	NPC_AI,
};

enum PhysicsMode
{
	NOCLIP,
	WALKING,
	FLYING,
	SWIMMING,
	CRAWLING,
};

enum CameraView
{
	SPECTATOR,
	INDEPENDENT,
	FIRST_PERSON,
	FIXED_ANGLE_TRACKING,
	OVER_SHOULDER,
};

class Entity
{
public:
	~Entity();
	Entity(Controls control);
	void SetSizeAABB3(AABB3 bounds, float eyeLevel);
	void CreateGeometry();
	void CyclePhysics();
	void CycleCameraView();
	void Update(float deltaSeconds);
	void UpdatePhysics(float deltaSeconds);
	Vec3 PreventativePhysicsTest(float deltaSeconds);
	void UpdateCameras();
	void Render();
	void SixDOF(float deltaSeconds);
	void Axial(float deltaSeconds);
	void Keyboard(float deltaSeconds);
// 	void Controller(float deltaSeconds);
	void Dig();
	void Place(uint8_t blockType);
	BlockIterator GetAdjacentBlockByNormal(RaycastHit hit);
	Vec3 GetForward() const;
	Vec3 GetLeft() const;
	Vec3 GetUp() const;
	void MoveInDirection(Vec3 direction, float speed);
	void AddForce(const Vec3& force);
	void AddImpulse(const Vec3& impulse);

	Controls m_contol = STATIC;
	PhysicsMode m_physics = NOCLIP;
	AABB3 m_bounds = AABB3::ZERO_TO_ONE;
	std::vector<Vertex_PCU> m_vertexes;
	float m_eyeLevel = 1.0f;

	Vec3 m_position = Vec3(0.0f, 0.0f, 75.0f); // see above
	EulerAngles m_orientation = EulerAngles(45.0f, 0.0f, 0.0f);

	CameraView m_cameraView = FIRST_PERSON;
	bool m_UseAxialCameraControls = false; // vs 6 DOF camera
	Vec3 m_cameraPosition = Vec3(0.0f, 0.0f, 75.0f);
	EulerAngles m_cameraOrientation = EulerAngles(45.0f, 30.0f, 0.0f);

	float m_speed = 4.0f;
	float m_rotate = 45.0f;
	Vec3 m_acceleration = Vec3::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
	float m_drag = DRAG;
	bool m_flying = false;
	bool m_isGrounded = false;
	float m_maxSpeed = 8.0f;

	RaycastHit m_raycastHit = {};
	Vec3 m_raycastStart = Vec3::ZERO;
	Vec3 m_raycastFwdNormal = Vec3::ZERO;
	bool m_rayFrozen = false;
	uint8_t m_blockSelection = 1;
};