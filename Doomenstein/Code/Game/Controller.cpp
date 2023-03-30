#include "Controller.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Map.hpp"

Controller::Controller()
{
//	ERROR_AND_DIE("called constructor");
}

Controller::~Controller()
{

}

void Controller::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
}

void Controller::Possess(Actor* actor)
{
	if (actor == nullptr)
	{
		m_actorUID = ActorUID::INVALID;
//		m_map = nullptr;
	}
	else
	{
		m_actorUID = actor->m_uid;
		m_map = actor->m_map;
	}
}

Actor* Controller::GetActor() const
{
	return m_map->FindActorByUID(m_actorUID);
}

