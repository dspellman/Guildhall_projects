#pragma once
#include "Controller.hpp"
#include "Engine/Core/Stopwatch.hpp"
#include "Map.hpp"
#include "ActorUID.hpp"
#include "Engine/Core/TileHeatMap.hpp"

class AI : public Controller
{
public:
	AI(Map* map, ActorUID uid);
	virtual ~AI();

	virtual void Update( float deltaSeconds ) override;
	void UpdateDemon(float deltaSeconds);
	void UpdateBoss(float deltaSeconds);
	Actor* GetResurrectionTarget(Vec3 position, float minRange);

	Stopwatch m_meleeStopwatch;
};

