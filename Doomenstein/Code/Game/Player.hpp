#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Math/EulerAngles.hpp"
#include "Controller.hpp"
#include "Engine/Math/AABB2.hpp"

class Player : public Controller
{
public:
	virtual ~Player();
	Player();
	Player(Vec3 const& position, EulerAngles orientation);
	Player(int playerIndex, int controllerIndex);

	void Update(float deltaSeconds);
	void UpdateCameras();
	void HudScreen() const;
	void DrawReticle();
	void HudTextOverlay(int health, int frags) const;
	void DrawMiniMap(IntVec2 origin);
	void Render();

	Mat44 GetModelMatrix() const;
	void		SetNormalizedViewport(const AABB2& viewport);
	AABB2		GetNormalizedViewport() const;
	AABB2		GetViewport() const;

	void SixDOF(float deltaSeconds);
	void Keyboard(float deltaSeconds);
	void Controller(float deltaSeconds);

public:
	Vec3 m_position;
	EulerAngles m_orientation;

	int			m_playerIndex = 0;
	int			m_controllerIndex = -1; // -1 is keyboard, otherwise controller number
	int			m_kills = 0;
	int			m_deaths = 0;

	Clock* m_animationClock;
	double m_animationStart;
	float m_grenadeTimer = 0.0f;

	AABB2		m_normalizedViewport = AABB2::ZERO_TO_ONE;

	Camera* m_cameraWorld = nullptr;
	Camera* m_cameraHud = nullptr;
	bool m_freeFlyCameraMode = false;
	float m_speed = 4.0f;
	float m_rotate = 45.0f;
};