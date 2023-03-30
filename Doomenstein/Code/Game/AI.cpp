#include "Game/AI.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/Actor.hpp"
#include "Game/Map.hpp"
#include <vector>
#include "Game.hpp"

AI::AI(Map* map, ActorUID uid)
{
	m_map = map;
	m_actorUID = uid;
	m_meleeStopwatch.Start(GetActor()->m_definition->m_meleeDelay);
}

AI::~AI()
{

}

void AI::Update(float deltaSeconds)
{
	if (GetActor()->m_definition->m_name == "Demon")
	{
		UpdateDemon(deltaSeconds);
	}
	else
	{
		UpdateBoss(deltaSeconds);
	}
}

void AI::UpdateDemon(float deltaSeconds)
{
	// look for enemy to follow (LOS) or attack (LOS and range)
	Actor* actor = GetActor();
	if (!actor || actor->m_isDead)
	{
		return;
	}

	Actor* closestEnemy = m_map->GetClosestVisibleEnemy(*actor);

	if (closestEnemy)
	{
		// turn towards enemy in FOV
		Vec3 lineTo = closestEnemy->m_position - actor->m_position;
		float ay = actor->m_orientation.m_yawDegrees;
		float ey = lineTo.GetEulerAngles().m_yawDegrees;
		float d = GetTurnedTowardDegrees(ay, ey, deltaSeconds * actor->m_definition->m_turnSpeed);
		actor->m_orientation.m_yawDegrees = d;

		float personalSpace = closestEnemy->m_definition->m_physicsRadius + actor->m_definition->m_physicsRadius;
		if ((closestEnemy->m_position - actor->m_position).GetLength() <= actor->m_definition->m_meleeRange + personalSpace)
		{
			// attack if in melee range
			if (m_meleeStopwatch.CheckDurationElapsedAndDecrement())
			{
				actor->SetAnimation("Attack");
				m_meleeStopwatch.Restart();
				closestEnemy->Damage(random.RollRandomFloatInRange(actor->m_definition->m_meleeDamage));
				closestEnemy->SetAnimation("Pain");
				SoundID attackSound = g_theAudio->CreateOrGetSound(actor->m_definition->m_attackSoundName);
				g_theAudio->StartSoundAt(attackSound, actor->m_position, false);
			}
		}
		else
		{
			// move toward actor if not in melee range yet
			actor->SetAnimation("Walk");
			actor->MoveInDirection(actor->m_orientation.GetForwardNormal(), actor->m_definition->m_runSpeed);
		}
	}
	else
	{
		actor->SetAnimation("Idle");
	}
}

void AI::UpdateBoss(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	Actor* actor = GetActor();
	if (!actor || actor->m_isDead)
	{
		return;
	}

	IntVec2 tile = actor->GetTileCoords();
	float heat = actor->m_map->nearestBody->Get(tile);
	if (heat < BOSS_MAX)
	{
		Vec3 target = actor->m_map->PickTarget(*actor->m_map->nearestBody, tile);
		// turn towards enemy in FOV
		Vec3 lineTo = target - actor->m_position;
		float ay = actor->m_orientation.m_yawDegrees;
		float ey = lineTo.GetEulerAngles().m_yawDegrees;
		float d = GetTurnedTowardDegrees(ay, ey, deltaSeconds * actor->m_definition->m_turnSpeed);
		actor->m_orientation.m_yawDegrees = d;

		// resurrect if body within melee range
 		float personalSpace = actor->m_definition->m_meleeRange + actor->m_definition->m_physicsRadius;
		Actor* body = GetResurrectionTarget(actor->m_position, personalSpace);
		if (body)
		{
			// resurrect if timer expired
			if (m_meleeStopwatch.CheckDurationElapsedAndDecrement())
			{
				actor->SetAnimation("Attack");
				m_meleeStopwatch.Restart();
				// resurrect the mob
				body->m_isDead = false;
				body->m_isDestroyed = false;
				body->m_health = body->m_definition->m_health;
				body->m_animation ="Pain";
				actor->SetAnimation("Attack");
				SoundID attackSound = g_theAudio->CreateOrGetSound(actor->m_definition->m_attackSoundName);
				g_theAudio->StartSoundAt(attackSound, actor->m_position, false);
				actor->m_map->CreateGoalHeatMap(*actor->m_map->nearestBody);
			}
		}
		else
		{
			// move toward actor if not in melee range yet
			actor->SetAnimation("Walk");
			actor->MoveInDirection(actor->m_orientation.GetForwardNormal(), actor->m_definition->m_runSpeed);
		}
	}
	else
	{
		if (m_meleeStopwatch.CheckDurationElapsedAndDecrement())
		{
			actor->SetAnimation("Attack");
			m_meleeStopwatch.Restart();
			for (int index = 0; index < g_theGame->m_numPlayers; index++)
			{
				if (g_theGame->m_player[index] == nullptr)
				{
					continue;
				}
				g_theGame->m_player[index]->GetActor()->Damage(1.0f);
				g_theGame->m_player[index]->GetActor()->SetAnimation("Pain");
			}
			SoundID attackSound = g_theAudio->CreateOrGetSound(actor->m_definition->m_attackSoundName);
			g_theAudio->StartSoundAt(attackSound, actor->m_position, false);
		}
		else
		{
			actor->SetAnimation("Idle");
		}
	}
}

Actor* AI::GetResurrectionTarget(Vec3 position, float minRange)
{
	float distance = HEAT_MAX;
	Actor* target = nullptr;

	for (int index = 0; index < (int)m_map->m_enemies.size(); index++)
	{
		Actor* candidate = m_map->m_enemies[index];
		if (candidate && candidate->m_isDead)
		{
			if ((candidate->m_position - position).GetLength() < distance)
			{
				distance = (candidate->m_position - position).GetLength();
				target = candidate;
			}
		}
	}

	// sanity check in case the closest body is not actually close
	if (distance > minRange)
	{
		return nullptr;
	}
	return target;
}

