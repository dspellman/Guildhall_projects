#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Cueball.hpp"
#include "GameCommon.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "CubicBezierCurve2D.hpp"

constexpr int RAY_DISC_TOTAL = 10;
constexpr int RAY_AABB2D_TOTAL = 10;
constexpr int SEGMENT_TOTAL = 10;
constexpr int OBB2_TOTAL = 10;
constexpr float RANGE = 10.0f;
constexpr int PACHENKO = 10;

enum Shape
{
	NONE = -1,
	BOX,
	SPHERE,
	CYLINDER,
	SHAPES
};

struct Shapes3D
{
	~Shapes3D() {};
	Shapes3D() {};

	Shape shape = NONE;
	Vec3 nearestPoint = Vec3::ZERO;
	RaycastResult3D raycastResult;
	bool overlap = false;

	union
	{
		AABB3 box;
		Vec3 center;
	};

	union
	{
		EulerAngles orientation;
		float length;
	};

	float radius;
};

enum GameState
{
	ATTRACT,
	NEAREST_POINT_2D,
	RAYCAST_VS_DISCS,
	RAYCAST_VS_AABB2D,
	RAYCAST_VS_Lines,
	RAYCAST_VS_OBB2,
	BILLIARDS_2D,
	PACHENKO_2D,
	TEST_SHAPES_3D,
	SPLINES,
	RESTART,
	STATES
};

class Game
{
public:
	GameState m_gameState = GameState::RAYCAST_VS_DISCS; // default start in attract mode
	GameState m_nextState = GameState::ATTRACT;

	float shakeFraction = 0.0f;

	// flags for actions selected by user
	bool m_isDebug = false;
	bool m_isQuitting = false;
	bool m_isPaused = false;
	bool m_isSlowMo = false;
	double m_gameOverWait = 0.0; // time the end game wait
	bool m_waitingToEnd = false;

	Camera m_attractCamera;
	Camera m_worldCamera2D;
	Camera m_worldCamera3D;
	Camera m_screenCamera; 

	// splines
	Camera m_worldCamera1;
	Camera m_worldCamera2;
	Camera m_worldCamera3;
	int m_selectedFunction = 0;
	int m_numSubdivisions = 64;
	bool m_showColors = false;
	float m_t = 0.0f;
	int m_segment = 0;
	CubicBezierCurve2D m_bezier;
	std::vector<CubicBezierCurve2D> m_hermite;
	std::vector<Vec2> m_controlPoints;

	Vec2 m_testPoint;
	float m_moveScale = 20.0f; // this define won't compile!!! MOVE_SPEED;

	// Shapes
	AABB2 m_aabb2;
	OBB2 m_obb2;
	Capsule2 m_capsule;
	LineSegment2 m_lineSegment;
	LineSegment2 m_infiniteLine;

	// disc parameters
	Vec2 m_discCenter;
	float m_discRadius;

	// ray cast disc list (and billiards 2D)
	Vec2 m_rayDiscCenters[RAY_DISC_TOTAL];
	float m_rayDiscRadii[RAY_DISC_TOTAL];
	float m_rayDiscElasticity[RAY_DISC_TOTAL];
	std::vector<Cueball*> cueballs;
	float m_ballRadius = 2.0f;
	Vec2 m_startPos = Vec2(40.0f, 40.0f);
	Vec2 m_endPos = Vec2(100.0f, 55.0f);
	Vec2 m_fwdNormal;
	float m_maxDist;
	float m_physicsTimeDebt = 0.0f;
	float m_physicsTimeStep = 0.001f;
	OBB2 m_bumperOBB[PACHENKO];
	float m_obbElasticity[PACHENKO];
	Capsule2 m_bumperCapsule[PACHENKO];
	float m_capsuleElasticity[PACHENKO];
	float m_deltaSeconds = 0.0f;
	int m_balls = 0;

	// ray cast rectangle list
	AABB2 m_rayRects[RAY_AABB2D_TOTAL];

	// ray cast line segments
	LineSegment2 m_lineSegments[SEGMENT_TOTAL];

	// ray cast OBB2
	OBB2 m_obb2s[OBB2_TOTAL];

	// test shapes 3D
	mutable Vec3 m_position = Vec3::ZERO;
	mutable EulerAngles m_orientation = EulerAngles::ZERO;
	float m_speed = 4.0f;
	float m_rotate = 45.0f;

	std::vector<Shapes3D*> m_shapes;
	int m_nearestRaycast = NONE;
	int m_nearestPoint = 0;
	int m_selectedShape = NONE;
	Vec3 m_selectedOffset = Vec3::ZERO;
	bool m_rayLock = false;
	Vec3 m_lockedPosition = Vec3::ZERO;
	EulerAngles m_lockedOrientation = EulerAngles::ZERO;
	mutable Vec3 m_truePosition = Vec3::ZERO;
	mutable EulerAngles m_trueOrientation = EulerAngles::ZERO;
	bool m_UseAxialCameraControls = true; // vs 6 DOF camera

public:
	Game();
	void Startup();
	void Update(float deltaSeconds);
	void UpdateCameras( float deltaSeconds);
	void Render() const;
//	void RenderEntities(GameState gameState) const;
	void DebugRender() const;
	void Shutdown();

	void UpdateAttractMode(float deltaSeconds);
	void Update_NEAREST_POINT_2D_Mode(float deltaSeconds);
	void Update_SPLINES_Mode(float deltaSeconds);
	void Update_RAYCAST_VS_DISCS_Mode(float deltaSeconds);
	void Update_RAYCAST_VS_AABB2D_Mode(float deltaSeconds);
	void Update_RAYCAST_VS_Lines_Mode(float deltaSeconds);
	void Update_RAYCAST_VS_OBB2_Mode(float deltaSeconds);
	void Update_BILLIARDS_2D_Mode(float deltaSeconds);
	void Update_PACHENKO_2D_Mode(float deltaSeconds);
	void UpdatePachenkoPhysics(float deltaSeconds);
	void SixDOF(float deltaSeconds);
	void Axial(float deltaSeconds);
	void Update_TEST_SHAPES_3D_Mode(float deltaSeconds);

	void RenderAttractMode() const;
	void Render_NEAREST_POINT_2D_Mode() const;
	void testWash(Camera const& camera, Rgba8 color) const;
	void Render_SPLINES_Mode() const;
	void Render_RAYCAST_VS_DISCS_Mode() const;
	void Render_RAYCAST_VS_AABB2D_Mode() const;
	void Render_RAYCAST_VS_Lines_Mode() const;
	void Render_RAYCAST_VS_OBB2_Mode() const;
	void Render_BILLIARDS_2D_Mode() const;
	void Render_PACHENKO_2D_Mode() const;
	void Render_TEST_SHAPES_3D_Mode() const;

private:
	void TestTexture() const;
	Vec2 GetRandomVec2ZeroToOne() const;
	Vec2 GetRandomPoint2D(float buffer);
	Vec3 GetRandomPoint3D(float limit);
	void Swap(float& a, float& b);
	void CreateRandomShapes();
	void CreateRandomDiscs();
	void CreateRandomRectangles();
	void CreateRandomLineSegments();
	void CreateRandomOBB2s();
	void CreateRandomBalls();
	void CreatePachenko();
	void CreateRandom3DShapes();
	void CreateHermiteSpline();
};