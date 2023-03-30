#pragma once
#include "Game/Actor.hpp"

class Map;

class Controller
{
	friend class Actor;

public:
	Controller();
	virtual ~Controller();

	virtual void Update( float deltaSeconds );
	void Possess( Actor* actor );
	Actor* GetActor() const;

	ActorUID m_actorUID = ActorUID::INVALID;
	Map* m_map = nullptr;
};

