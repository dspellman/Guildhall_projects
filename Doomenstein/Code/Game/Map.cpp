#pragma once
#include "Map.hpp"
#include "Engine/Core/Vertex_PNCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "TileDefinition.hpp"
#include "TileMaterialDefinition.hpp"
#include "MapDefinition.hpp"
#include "TileSetDefinition.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Tile.hpp"
#include <stdexcept>
#include "Player.hpp"
#include "Game.hpp"
#include "Game/AI.hpp"

bool indexedDraw = true;

Map::Map(Game* game, const MapDefinition* definition)
	: m_game(game), m_definition(definition)
{
	g_theRenderer->SetAmbientIntensity(0.8f);
	g_theRenderer->SetSunIntensity(0.4f);
//	g_theRenderer->SetSunDirection(EulerAngles(0.0f, 135.0f, 0.0f).GetForwardNormal());

	RandomizeMap();
	CreateTiles();
	CreateGeometry();
	CreateBuffers();

	nearestBody = new TileHeatMap(m_dimensions);
	CreateGoalHeatMap(*nearestBody);

// 	for (int index = 0; index < static_cast<int>(definition->m_spawnInfos.size()); index++)
// 	{
// 		if (definition->m_spawnInfos[index].m_definition && definition->m_spawnInfos[index].m_definition->m_faction == Faction::DEMON)
// 		{
// 			SpawnDemon(definition->m_spawnInfos[index]);
// 		}
// 	}

// 	for (int index = 0; index < g_maxPlayers; index++)
// 	{
// 		if (g_theGame->m_player[index])
// 		{
// 			SpawnPlayer(index);
// 		}
// 	}

	constexpr int demonsPerQuadrant = 12;

	for (int index = 0; index < demonsPerQuadrant; index++)
	{
		SpawnRandomDemon("Demon", IntVec2(1, 1), IntVec2(48, 48));
	}
	SpawnRandomDemon("Boss", IntVec2(1, 1), IntVec2(48, 48));

	for (int index = 0; index < demonsPerQuadrant; index++)
	{
		SpawnRandomDemon("Demon", IntVec2(50, 1), IntVec2(48, 48));
	}
	SpawnRandomDemon("Boss", IntVec2(50, 1), IntVec2(48, 48));

	for (int index = 0; index < demonsPerQuadrant; index++)
	{
		SpawnRandomDemon("Demon", IntVec2(1, 50), IntVec2(48, 48));
	}
	SpawnRandomDemon("Boss", IntVec2(1, 50), IntVec2(48, 48));

	for (int index = 0; index < demonsPerQuadrant; index++)
	{
		SpawnRandomDemon("Demon", IntVec2(50, 50), IntVec2(48, 48));
	}
	SpawnRandomDemon("Boss", IntVec2(50, 50), IntVec2(48, 48));

	for (int index = 0; index < g_maxPlayers; index++)
	{
		if (g_theGame->m_player[index])
		{
			SpawnRandomPlayer(index);
		}
	}
}

Map::~Map()
{
	if (m_immediateVBO_PCU)
	{
		delete m_immediateVBO_PCU;
		m_immediateVBO_PCU = nullptr;
	}

	if (m_immediateVBO_PNCU)
	{
		delete m_immediateVBO_PNCU;
		m_immediateVBO_PNCU = nullptr;
	}

	if (m_indexBuffer)
	{
		delete m_indexBuffer;
		m_indexBuffer = nullptr;
	}

	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index])
		{
			delete m_actors[index];
			m_actors[index] = nullptr;
		}
	}
	m_actors.clear();

	if (nearestBody)
	{
		delete nearestBody;
		nearestBody = nullptr;
	}
}

Actor* Map::SpawnActor(const SpawnInfo& spawnInfo)
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index] == nullptr)
		{
			Actor* newActor = new Actor(this, spawnInfo);
			newActor->m_uid = ActorUID(index, NextSalt());
			m_actors[index] = newActor;
			return newActor;
		}
	}

	if (static_cast<int>(m_actors.size()) < 0x0000FFFE)
	{		
		Actor* newActor = new Actor(this, spawnInfo);
		newActor->m_uid = ActorUID(static_cast<int>(m_actors.size()), NextSalt());
		m_actors.push_back(newActor);
		return newActor;
	}
	
	return nullptr;
}

int Map::NextSalt()
{
	if (++m_actorSalt == 0x0000FFFF)
	{
		m_actorSalt = 0;
	}
	return m_actorSalt;
}

void Map::DestroyActor(const ActorUID uid)
{
// 	if (!uid.IsValid())
// 	{
// 		return; // uid is not valid, so cannot use it to index
// 	}
	int index = uid.GetIndex();
	if (index > static_cast<int>(m_actors.size()) || m_actors[index] == nullptr)
	{
		return;
	}
	if (m_actors[index]->m_uid != uid)
	{
		return; // uid mismatch, stale reference so don't destroy current actor
	}
	delete m_actors[index];
	m_actors[index] = nullptr;
}

Actor* Map::FindActorByUID(const ActorUID uid) const
{
	if (!uid.IsValid())
	{
		return nullptr;
	}

	int index = uid.GetIndex();
	if (m_actors[index] == nullptr)
	{
		return nullptr;
	}

	if (m_actors[index]->m_uid == uid)
		{
			return m_actors[index];
		}

	return nullptr;
}

Actor* Map::GetClosestVisibleEnemy(Actor const& actor) const
{
	Vec3 fwdNormal = actor.m_orientation.GetForwardNormal();
	float closestDistance = actor.m_definition->m_sightRadius;
	Actor* closestEnemy = nullptr;

	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		// check for enemy faction
		if (!m_actors[index] || !m_actors[index]->m_definition || m_actors[index]->m_definition->m_faction == Faction::NEUTRAL ||
			m_actors[index]->m_definition->m_faction == actor.m_definition->m_faction)
		{
			continue;
		}
		// range check from R squared startm_actors[index]
		Vec3 lineTo = m_actors[index]->m_position - actor.m_position;
		if (lineTo.GetLengthSquared() >= closestDistance * closestDistance)
		{
			continue;
		}
		// FOV check
		Vec2 lineXY(lineTo.x, lineTo.y);
		Vec2 normalXY(fwdNormal.x, fwdNormal.y);
		//normalXY.Normalize();
		if (fabsf(GetAngleDegreesBetweenVectors2D(lineXY, normalXY)) > actor.m_definition->m_sightAngle * 0.5f)
		{
			continue;
		}
		// edge ray casts for LOS and more for gaps
		Actor* target = nullptr;
		RaycastResult3D test = RaycastAll(actor.m_position, lineTo.GetNormalized(), lineTo.GetLength(), &target, RaycastFilter(&actor));
		float range = test.m_impactDist + m_actors[index]->m_definition->m_physicsRadius + 0.01f;
		if (test.m_didImpact && (range * range >= (lineTo.GetLengthSquared())))
		{
			// target will be closestEnemy if right distance (if test is just target non-null)
			closestEnemy = m_actors[index];
			closestDistance = test.m_impactDist;
		}
	}

	return closestEnemy;
}

void Map::Update(float deltaSeconds)
{
	UpdatePlayers(deltaSeconds);
	UpdateAI(deltaSeconds);
	UpdateActors(deltaSeconds);
	UpdatePhysics(deltaSeconds);
	CollideActors();
	CollideActorsWithMap();
	UpdateCameras(deltaSeconds);
	MarkDeadActors();
	DeleteDestroyedActors();
}

void Map::Render()
{
	Texture* terrainTexture = g_theRenderer->CreateOrGetTextureFromFile((char const*)"Data/Images/Terrain_8x8.png");
	g_theRenderer->BindTexture(terrainTexture);
	g_theRenderer->SetModelMatrix(Mat44());
	g_theRenderer->BindShaderByName("Data/Shaders/SpriteLit");
	if (indexedDraw)
	{
		g_theRenderer->DrawIndexedVertexBuffer(m_indexBuffer, m_immediateVBO_PNCU, m_indexCount);	
	}
	else
	{
		g_theRenderer->DrawVertexArray(int(m_vertexes.size()), &m_vertexes[0]);
	}
	g_theRenderer->BindShader(nullptr);

	// render game objects
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index])
		{
			m_actors[index]->Render();
		}
	}

	for (int index = 0; index < g_theGame->GetNumPlayers(); index++)
	{
		if (g_theGame->GetPlayer(index))
		{
			g_theGame->GetPlayer(index)->Render();
		}
	}
}

bool Map::IsTileSolid(int x, int y) const
{
	if (x < 1 || x > m_dimensions.x - 1)
		return true;
	if (y < 1 || y > m_dimensions.y - 1)
		return true;
	if (m_tiles[x + m_dimensions.x * y].IsSolid())
		return true;
	return false;
}

AABB2 Map::GetAABB2ForTile2D(int x, int y) const
{
	float xCoord = static_cast<float>(x);
	float yCoord = static_cast<float>(y);

	Vec2 mins(xCoord, yCoord);
	Vec2 maxs(xCoord + 1.0f, yCoord + 1.0f);

	return AABB2(mins, maxs);
}

bool Map::DoesActorCollide(Actor* actor)
{
	return actor && !actor->m_isDead && actor->m_definition && actor->m_definition->m_collidesWithActors;
}

void Map::PushActorsOutOfEachOther(Actor* a, Actor* b)
{
	bool pushed = false;
//	bool marine = a->m_definition->m_faction == Faction::MARINE || b->m_definition->m_faction == Faction::MARINE;
//	bool demon = a->m_definition->m_faction == Faction::DEMON || b->m_definition->m_faction == Faction::DEMON;

	Vec2 apos(a->m_position.x, a->m_position.y);
	Vec2 bpos(b->m_position.x, b->m_position.y);

	// plasma projectiles do not push anything
	if (!a->m_definition->m_flying && !b->m_definition->m_flying)
	{
		pushed = PushDiscsOutOfEachOther2D(apos, a->m_definition->m_physicsRadius, bpos, b->m_definition->m_physicsRadius);
	}

	if (!a->m_definition->m_flying && b->m_definition->m_flying)
	{
		pushed = PushDiscOutOfDisc2D(bpos, b->m_definition->m_physicsRadius, apos, a->m_definition->m_physicsRadius);
	}

	if (a->m_definition->m_flying && !b->m_definition->m_flying)
	{
		pushed = PushDiscOutOfDisc2D(apos, a->m_definition->m_physicsRadius, bpos, b->m_definition->m_physicsRadius);
	}

	// don't actually update pushed positions for plasma because it dies and it will move actors by impulse
	{
		a->m_position.x = apos.x;
		a->m_position.y = apos.y;
		b->m_position.x = bpos.x;
		b->m_position.y = bpos.y;
	}

// 	if (marine)
// 	{
// 		return; // prevent damage by plasma projectiles
// 	}
	if (a->m_owner == b || b->m_owner == a)
	{
		return; // do not get hit by your own projectiles
	}

	if (pushed)
	{
		Vec3 normal = (b->m_position - a->m_position).GetNormalized();

		a->AddImpulse(b->m_definition->m_impulseOnCollide * DotProduct3D(b->GetForward(), normal) * b->GetForward() * -1.0f);
		b->AddImpulse(a->m_definition->m_impulseOnCollide * DotProduct3D(a->GetForward(), normal) * a->GetForward());

		if (a->Damage(random.RollRandomFloatInRange(b->m_definition->m_damageOnCollide)))
		{
			if (a->m_controller && b->m_owner && b->m_owner->m_controller)
			{
				Player* player = dynamic_cast<Player*>(b->m_owner->m_controller);
				player->m_kills++;
			}
		}
		if (b->Damage(random.RollRandomFloatInRange(a->m_definition->m_damageOnCollide)))
		{
			if (b->m_controller && a->m_controller && a->m_owner->m_controller)
			{
				Player* player = dynamic_cast<Player*>(a->m_owner->m_controller);
				player->m_kills++;
			}
		}

		if (a->m_definition->m_dieOnCollide)
		{
			a->Die();
		}
		if (b->m_definition->m_dieOnCollide)
		{
			b->Die();
		}

		// TEST DEBUG
		if (a->m_definition->m_name == "PlasmaGrenadeProjectile")
		{
			a->m_velocity.Reflect(normal);
		}
		if (b->m_definition->m_name == "PlasmaGrenadeProjectile")
		{
			b->m_velocity.Reflect(-normal);
		}
	}
}

void Map::CollideActors()
{
	// actor vs actor
	for (int a = 0; a < static_cast<int>(m_actors.size()); a++)
	{
		Actor* actorA = m_actors[a];
		if (DoesActorCollide(actorA))
		{
			for (int b = a + 1; b < static_cast<int>(m_actors.size()); b++)
			{
				Actor* actorB = m_actors[b];
				if (DoesActorCollide(actorB))
				{
					if (actorA == actorB)
					{
						continue; // can't push itself
					}
					CollideActors(actorA, actorB);
				}
			}
		}
	}
}

void Map::CollideActors(Actor* actorA, Actor* actorB)
{
	PushActorsOutOfEachOther(actorA, actorB);
}

void Map::CollideActorsWithMap()
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index] && m_actors[index]->m_definition->m_collidesWithWorld)
		{
			CollideActorWithMap(m_actors[index]);
		}
	}
}

void Map::CollideActorWithMap(Actor* actor)
{
	bool pushed = false;
	bool result = false;
	int xCoord = RoundDownToInt(actor->m_position.x);
	int yCoord = RoundDownToInt(actor->m_position.y);
	Vec2 position = Vec2(actor->m_position.x, actor->m_position.y);
	Vec3 normal = Vec3::ZERO;

	if (xCoord < 1 || yCoord < 1 || xCoord > m_dimensions.x - 1 || yCoord > m_dimensions.y - 1)
		return; // ignore problems during early testing...delete this later or clamp to in-bound range

	// cardinal points first
	if (IsTileSolid(xCoord + 1, yCoord))
	{
		pushed |= (result = PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord + 1, yCoord)));
		if (result)
			normal.x = -1.0f;
	}
	if (IsTileSolid(xCoord, yCoord + 1))
	{
		pushed |= (result = PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord, yCoord + 1)));
		if (result)
			normal.y = -1.0f;
	}
	if (IsTileSolid(xCoord - 1, yCoord))
	{
		pushed |= (result = PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord - 1, yCoord)));
		if (result)
			normal.x = 1.0f;
	}
	if (IsTileSolid(xCoord, yCoord - 1))
	{
		pushed |= (result = PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord, yCoord - 1)));
		if (result)
			normal.y = 1.0f;
	}

	// diagonal points second
	if (IsTileSolid(xCoord + 1, yCoord + 1))
	{
		pushed |= PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord + 1, yCoord + 1));
	}
	if (IsTileSolid(xCoord + 1, yCoord - 1))
	{
		pushed |= PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord + 1, yCoord - 1));
	}
	if (IsTileSolid(xCoord - 1, yCoord + 1))
	{
		pushed |= PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord - 1, yCoord + 1));
	}
	if (IsTileSolid(xCoord - 1, yCoord - 1))
	{
		pushed |= PushDiscOutOfAABB2D(position, actor->m_definition->m_physicsRadius, GetAABB2ForTile2D(xCoord - 1, yCoord - 1));
	}

	actor->m_position.x = position.x;
	actor->m_position.y = position.y;

	// test floor and ceiling collision and correct
	if (actor->m_position.z < 0.0f)
	{
		actor->m_position.z = ClampZeroToOne(actor->m_position.z);
		pushed = true;
		normal.z = 1.0f;
	}
	if (actor->m_position.z > 1.0f - actor->m_definition->m_physicsHeight)
	{
		actor->m_position.z = ClampZeroToOne(actor->m_position.z);
		pushed = true;
		normal.z = -1.0f;
	}

	if (pushed)
	{
		if (actor->m_definition->m_dieOnCollide)
		{
			actor->Die();
		}
		if (actor->m_definition->m_name == "PlasmaGrenadeProjectile")
		{
			normal.Normalize();
			actor->m_velocity.Reflect(normal);
		}
	}
}

Player* Map::GetPlayer()
{
	return g_theGame->m_renderingPlayer;
}

Game* Map::GetGame()
{
	return m_game;
}

void Map::CreateTiles()
{
	m_dimensions = m_definition->m_image.GetDimensions();

	for (int y = 0; y < m_dimensions.y; y++)
	{
		for (int x = 0; x < m_dimensions.x; x++)
		{
			Rgba8 tileCode = m_definition->m_image.GetTexelColor(IntVec2(x,y));
			float xCoord = static_cast<float>(x);
			float yCoord = static_cast<float>(y);
			AABB3 bounds(Vec3(xCoord, yCoord, 0.0f), Vec3(xCoord + 1.0f, yCoord + 1.0f, 1.0f));
			Tile tile(bounds, m_definition->m_tileSetDefinition->GetTileDefinitionByColor(tileCode));
			m_tiles.push_back(tile);
		}
	}
}

void Map::CreateGeometry()
{
	if (indexedDraw)
	{
		Rgba8 color = Rgba8::WHITE;

		for (Tile tile : m_tiles)
		{
			AABB3 bounds = tile.m_bounds;
			Vec3 c000(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
			Vec3 c001(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
			Vec3 c010(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
			Vec3 c011(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
			Vec3 c100(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
			Vec3 c101(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
			Vec3 c110(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
			Vec3 c111(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

			if (tile.IsSolid())
			{
				AABB2 UVs = tile.m_definition->m_wallMaterialDefinition->m_uv;
				// add verts for outward facing walls
				AddVertsForQuad3D(m_indexes, m_vertexes, c001, c000, c100, c101, color, UVs);
				AddVertsForQuad3D(m_indexes, m_vertexes, c101, c100, c110, c111, color, UVs);
				AddVertsForQuad3D(m_indexes, m_vertexes, c111, c110, c010, c011, color, UVs);
				AddVertsForQuad3D(m_indexes, m_vertexes, c011, c010, c000, c001, color, UVs);
			}
			if (tile.HasFloor())
			{
				AABB2 UVs = tile.m_definition->m_ceilingMaterialDefinition->m_uv;
				AddVertsForQuad3D(m_indexes, m_vertexes, c011, c111, c101, c001, color, UVs);
				UVs = tile.m_definition->m_floorMaterialDefinition->m_uv;
				AddVertsForQuad3D(m_indexes, m_vertexes, c000, c100, c110, c010, color, UVs);
			}
		}

		m_indexCount = static_cast<unsigned int>(m_indexes.size()); // could eliminate this
		return;
	}

	// non-indexed buffer draw for testing
	Rgba8 color = Rgba8::WHITE;

	for (Tile tile : m_tiles)
	{
		AABB3 bounds = tile.m_bounds;
		Vec3 c000(bounds.m_mins.x, bounds.m_mins.y, bounds.m_mins.z);
		Vec3 c001(bounds.m_mins.x, bounds.m_mins.y, bounds.m_maxs.z);
		Vec3 c010(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_mins.z);
		Vec3 c011(bounds.m_mins.x, bounds.m_maxs.y, bounds.m_maxs.z);
		Vec3 c100(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_mins.z);
		Vec3 c101(bounds.m_maxs.x, bounds.m_mins.y, bounds.m_maxs.z);
		Vec3 c110(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_mins.z);
		Vec3 c111(bounds.m_maxs.x, bounds.m_maxs.y, bounds.m_maxs.z);

		if (tile.IsSolid())
		{
			AABB2 UVs = tile.m_definition->m_wallMaterialDefinition->m_uv;
			// add verts for outward facing walls
			AddVertsForQuad3D(m_vertexes, c001, c000, c100, c101, color, UVs);
			AddVertsForQuad3D(m_vertexes, c101, c100, c110, c111, color, UVs);
			AddVertsForQuad3D(m_vertexes, c111, c110, c010, c011, color, UVs);
			AddVertsForQuad3D(m_vertexes, c011, c010, c000, c001, color, UVs);
		}
		if (tile.HasFloor())
		{
			AABB2 UVs = tile.m_definition->m_ceilingMaterialDefinition->m_uv;
			AddVertsForQuad3D(m_vertexes, c011, c111, c101, c001, color, UVs);
			UVs = tile.m_definition->m_floorMaterialDefinition->m_uv;
			AddVertsForQuad3D(m_vertexes, c000, c100, c110, c010, color, UVs);
		}
	}
}

void Map::CreateBuffers()
{
	if (!indexedDraw)
	{
		return;
	}

	m_indexBuffer = g_theRenderer->CreateIndexBuffer(m_indexes.data(), m_indexCount * sizeof(unsigned int)); // multiply by  to get total count?
//	m_immediateVBO_PCU = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PCU), sizeof(Vertex_PCU));
	m_immediateVBO_PNCU = g_theRenderer->CreateVertexBuffer(sizeof(Vertex_PNCU), sizeof(Vertex_PNCU));

//	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), static_cast<size_t>(m_vertexes.size()) * m_immediateVBO_PCU->GetStride(), m_immediateVBO_PCU);
	g_theRenderer->CopyCPUToGPU(m_vertexes.data(), static_cast<size_t>(m_vertexes.size()) * m_immediateVBO_PNCU->GetStride(), m_immediateVBO_PNCU);
	g_theRenderer->CopyCPUToGPU(m_indexes.data(), static_cast<size_t>(m_indexes.size()) * m_indexBuffer->GetStride(), m_indexBuffer);
}

RaycastResult3D Map::RaycastAll(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, Actor** actorHit, RaycastFilter filter) const
{
	RaycastResult3D closest;
	RaycastResult3D test;
	closest.m_impactDist = 999999.0f;

	test = RaycastWorldXY(startPos, fwdNormal, maxDist, filter);
	if (test.m_impactDist < closest.m_impactDist)
	{
		closest = test;
	}	
	
	test = RaycastWorldZ(startPos, fwdNormal, maxDist, filter);
//	test = RaycastWorldZ(startPos, fwdNormal, closest.m_impactDist, filter);
	if (test.m_impactDist < closest.m_impactDist)
	{
		closest = test;
	}
	
	test = RaycastWorldActors(startPos, fwdNormal, maxDist, actorHit, filter);
//	test = RaycastWorldActors(startPos, fwdNormal, closest.m_impactDist, actorHit, filter);
	if (test.m_impactDist < closest.m_impactDist)
	{
		closest = test;
	}
	return closest;
}

RaycastResult3D Map::RaycastWorldXY(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, RaycastFilter filter) const
{
	UNUSED(filter);
	RaycastResult3D raycastResult;
	// create default for no hit case
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	// set up 2D raycast within level
	RaycastResult2D closest;
	int flags = ISSOLID_FLAGS::TESTXY_NOTZ;
	closest = RaycastVsTiles2D(startPos, fwdNormal, maxDist, flags);
	if (closest.m_didImpact == false)
	{
		return raycastResult;
	}

	// convert 2D impact to 3D version of result
	raycastResult.m_didImpact = closest.m_didImpact;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist * closest.m_fraction;
	raycastResult.m_impactDist = maxDist * closest.m_fraction;
	raycastResult.m_impactNormal = closest.m_fraction == 0.0f ? -fwdNormal : Vec3(closest.m_impactNormal.x, closest.m_impactNormal.y, 0.0f);

	return raycastResult;
}

RaycastResult3D Map::RaycastWorldZ(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, RaycastFilter filter) const
{
	UNUSED(filter);
	RaycastResult3D raycastResult;
	// set up default return for no intersection
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist;
	raycastResult.m_impactPos = startPos + fwdNormal * maxDist;
	raycastResult.m_impactNormal = fwdNormal; // this should not matter
	raycastResult.m_rayFwdNormal = fwdNormal;
	raycastResult.m_rayStartPos = startPos;
	raycastResult.m_rayMaxLength = maxDist;

	Vec3 endPos = startPos + fwdNormal * maxDist;
	float t = 1.0f;
	float SEinverse = 1.0f / (endPos.z - startPos.z);
	Vec3 potentialHit = Vec3::ZERO;

	// test ceiling from below
	if (startPos.z < 1.0f && fwdNormal.z > 0.0f)
	{
		t = (1.0f - startPos.z) * SEinverse;
		potentialHit = startPos + fwdNormal * maxDist * t;
		if (OutOfBounds(potentialHit))
		{
			return raycastResult;
		}
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = maxDist * t;
		raycastResult.m_impactPos = potentialHit;
		raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, -1.0f); // this should not matter
		return raycastResult;
	}

	// test floor from above
	if (startPos.z > 0.0f && fwdNormal.z < 0.0f)
	{
		t = -startPos.z * SEinverse;
		potentialHit = startPos + fwdNormal * maxDist * t;
		if (OutOfBounds(potentialHit))
		{
			return raycastResult;
		}
		raycastResult.m_didImpact = true;
		raycastResult.m_impactDist = maxDist * t;
		raycastResult.m_impactPos = potentialHit;
		raycastResult.m_impactNormal = Vec3(0.0f, 0.0f, 1.0f); // this should not matter
		return raycastResult;
	}

	// hit test failed, return default no hit
	return raycastResult;
}

RaycastResult3D Map::RaycastWorldActors(const Vec3& start, const Vec3& direction, float distance, Actor** actorHit, RaycastFilter filter) const
{
	RaycastResult3D closest;
	RaycastResult3D test;
	closest.m_impactDist = 999999.0f;
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (!m_actors[index] || !m_actors[index]->m_definition)
		{
			continue;
		}
		if (filter.m_ignoreActor && m_actors[index] == filter.m_ignoreActor)
		{
			continue;
		}
		if (filter.m_ignoreActor && filter.m_ignoreActor->m_definition->m_faction != MARINE && m_actors[index]->m_definition->m_faction == filter.m_ignoreActor->m_definition->m_faction)
		{
			continue;
		}
		Vec3 center = m_actors[index]->m_position;
		center.z += m_actors[index]->m_definition->m_physicsHeight * 0.5f;
		test = RaycastVsZCylinder(start, direction, distance, center, m_actors[index]->m_definition->m_physicsHeight, m_actors[index]->m_definition->m_physicsRadius);
		if (test.m_didImpact && test.m_impactDist < closest.m_impactDist)
		{
			closest = test;
			if (actorHit)
			{
				*actorHit = m_actors[index]; // save the actor we hit for reference
			}
		}
	}
	return closest;
}

void Map::ToggleRaycastDebug()
{
	// TODO
}

void Map::DebugPossessNext()
{
	// TODO
}

// generic raycast against a tile grid //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RaycastResult2D Map::RaycastVsTiles2D(Vec3 startPos, Vec3 fwdNormal, float maxDist, int flags) const
{
	// Create 2D variables for testing in XY plane
	Vec2 normal2D = Vec2(fwdNormal.x, fwdNormal.y).GetNormalized();
	float maxDist2D = Vec2(fwdNormal.x, fwdNormal.y).GetLength() * maxDist; // adjust length to be projection on XY plane
	Vec2 startPos2D(startPos.x, startPos.y);

	struct RaycastResult2D raycastResult;

	// set up default return for no intersection
	raycastResult.m_didImpact = false;
	raycastResult.m_impactDist = maxDist2D;
	raycastResult.m_fraction = 1.0f;
	raycastResult.m_impactPos = startPos2D + normal2D * maxDist2D;
	raycastResult.m_impactNormal = normal2D; // this should not matter
	raycastResult.m_rayFwdNormal = normal2D;
	raycastResult.m_rayStartPos = startPos2D;
	raycastResult.m_rayMaxLength = maxDist2D;
	raycastResult.tileCoords = IntVec2(RoundDownToInt(raycastResult.m_impactPos.x), RoundDownToInt(raycastResult.m_impactPos.y)); // matches other default parameters

	int tileX = RoundDownToInt(startPos2D.x);
	int tileY = RoundDownToInt(startPos2D.y);
	float fwdDistPerXCrossing = 1.0f / fabsf(normal2D.x);
	float fwdDistPerYCrossing = 1.0f / fabsf(normal2D.y);
	int tileStepDirectionX = normal2D.x < 0.0f ? -1 : 1;
	int tileStepDirectionY = normal2D.y < 0.0f ? -1 : 1;

	float xAtFirstXCrossing = static_cast<float>(tileX) + static_cast<float>(tileStepDirectionX + 1) / 2.0f;
	float xDistToFirstXCrossing = xAtFirstXCrossing - startPos2D.x;
	float fwdDistAtNextXCrossing = fabsf(xDistToFirstXCrossing) * fwdDistPerXCrossing;

	float yAtFirstYCrossing = static_cast<float>(tileY) + static_cast<float>(tileStepDirectionY + 1) / 2.0f;
	float yDistToFirstYCrossing = yAtFirstYCrossing - startPos2D.y;
	float fwdDistAtNextYCrossing = fabsf(yDistToFirstYCrossing) * fwdDistPerYCrossing;

	// if starting tile is solid, return immediate hit
	if (InSolidTile(tileX, tileY, flags) && InBounds(startPos))
	{
		raycastResult.m_impactDist = 0.0f;
		raycastResult.m_fraction = 0.0f;
		raycastResult.m_didImpact = true;
		raycastResult.m_impactPos = startPos2D;
		// determine impact surface normal according to what it would be if we followed the trajectory to the next tile
		raycastResult.m_impactNormal = (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing) ? Vec2(static_cast<float>(-tileStepDirectionX), 0.0f) : Vec2(0.0f, static_cast<float>(-tileStepDirectionY));
		raycastResult.tileCoords = IntVec2(tileX, tileY);
		return raycastResult;
	}

	Vec3 impact3D;
	Vec2 impact2D;
	for (;;)
	{
		if (fwdDistAtNextXCrossing < fwdDistAtNextYCrossing)
		{
			if (fwdDistAtNextXCrossing > maxDist2D)
			{
				// raycastResult.tileCoords = IntVec2(tileX, tileY);
				return raycastResult; // no hit
			}
			tileX += tileStepDirectionX;
			impact3D = startPos + fwdNormal * fwdDistAtNextXCrossing * maxDist / maxDist2D;
			impact2D = startPos2D + normal2D * fwdDistAtNextXCrossing;
			if (InSolidTile(tileX, tileY, flags) && InBounds(impact3D))
			{
				raycastResult.m_impactDist = fwdDistAtNextXCrossing;
				raycastResult.m_fraction = fwdDistAtNextXCrossing / maxDist2D;
				raycastResult.m_didImpact = true;
				raycastResult.m_impactPos = startPos2D + normal2D * fwdDistAtNextXCrossing;
				raycastResult.m_impactNormal = Vec2(static_cast<float>(-tileStepDirectionX), 0.0f);
				raycastResult.tileCoords = IntVec2(tileX, tileY);
				return raycastResult; // hit
			}
			fwdDistAtNextXCrossing += fwdDistPerXCrossing;
		}
		else
		{
			if (fwdDistAtNextYCrossing > maxDist2D)
			{
				// raycastResult.tileCoords = IntVec2(tileX, tileY);
				return raycastResult; // no hit
			}
			tileY += tileStepDirectionY;
			impact3D = startPos + fwdNormal * fwdDistAtNextYCrossing * maxDist / maxDist2D;
			impact2D = startPos2D + normal2D * fwdDistAtNextYCrossing;
			if (InSolidTile(tileX, tileY, flags) && InBounds(impact3D))
			{
				raycastResult.m_impactDist = fwdDistAtNextYCrossing;
				raycastResult.m_fraction = fwdDistAtNextYCrossing / maxDist2D;
				raycastResult.m_didImpact = true;
				raycastResult.m_impactPos = startPos2D + normal2D * fwdDistAtNextYCrossing;
				raycastResult.m_impactNormal = Vec2(0.0f, static_cast<float>(-tileStepDirectionY));
				raycastResult.tileCoords = IntVec2(tileX, tileY);
				return raycastResult; // hit
			}
			fwdDistAtNextYCrossing += fwdDistPerYCrossing;
		}
	}
}

bool Map::InSolidTile(int tileXcoord, int tileYcoord, int flags) const
{
	if (tileXcoord < 0 || tileYcoord < 0)
	{
		return false;
	}
	if (tileXcoord >= m_dimensions.x || tileYcoord >= m_dimensions.y)
	{
		return false;
	}

	// tile is inside map bounds, so test it based on flags
	if (flags & TESTXY_NOTZ)
	{
		return GetTileXY(tileXcoord, tileYcoord).IsSolid(); // be sure bounds are always valid when making this call
	}

	// if flags are not valid for the tile, return false by default
	return false;
}

// this function test whether a proposed impact point is outside the map since outside is never valid
bool Map::OutOfBounds(Vec3 point) const
{
	constexpr float d = 0.0001f;
	if (point.z < 0.0f - d || point.z > 1.0f + d)
		return true;
	if (point.x < 0.0f - d || point.y < 0.0f - d)
		return true;
	if (point.x < static_cast<float>(m_dimensions.x) + d && point.y < static_cast<float>(m_dimensions.y) + d)
		return false;
	return true;
}

bool Map::InBounds(Vec3 point) const
{
	return !OutOfBounds(point);
}

const Tile& Map::GetTileXY(int x, int y) const
{
	return m_tiles[m_dimensions.x * y + x];
}

Image const& Map::GetMapImage() const
{
	return m_definition->m_image;
}

void Map::SpawnPlayer(int index)
{
	double timeRand = GetCurrentTimeSeconds();
	timeRand = fmod(timeRand, 3.0);
	int odds = static_cast<int>(timeRand);

 	Actor* marine = SpawnActor(ActorDefinition::GetByName("Marine"));
// 	for (int index = 0; index < static_cast<int>(m_definition->m_spawnInfos.size()); index++)
	{
		if (m_definition->m_spawnInfos[odds].m_definition && m_definition->m_spawnInfos[odds].m_definition->m_name.compare("SpawnPoint") == 0)
		{	
			marine->m_position = m_definition->m_spawnInfos[odds].m_position;
			marine->m_orientation = m_definition->m_spawnInfos[odds].m_orientation;
		}
	}

	if (g_theGame->m_player[index] == nullptr)
	{
		ERROR_AND_DIE("Player is null");
	}
	else
	{
		g_theGame->m_player[index]->m_position = marine->m_position;
		g_theGame->m_player[index]->m_orientation = marine->m_orientation;
	}
	marine->OnPossessed(g_theGame->m_player[index]);
}

void Map::SpawnRandomPlayer(int index)
{
	Actor* marine = SpawnActor(ActorDefinition::GetByName("Marine"));
	marine->m_position = FindOpenTile(IntVec2(25, 25), IntVec2(24, 24));
	marine->m_orientation = EulerAngles(random.RollRandomFloatInRange(0.0f, 360.0f), 0.0f, 0.0f);

	if (g_theGame->m_player[index] == nullptr)
	{
		ERROR_AND_DIE("Player is null");
	}
	else
	{
		g_theGame->m_player[index]->m_position = marine->m_position;
		g_theGame->m_player[index]->m_orientation = marine->m_orientation;
	}
	marine->OnPossessed(g_theGame->m_player[index]);
}

void Map::SpawnDemon(SpawnInfo const spawnInfo)
{
	Actor* demon = SpawnActor(ActorDefinition::GetByName(spawnInfo.m_definition->m_name));
	demon->m_position = spawnInfo.m_position;
	demon->m_orientation = spawnInfo.m_orientation;
	demon->m_aiController = new AI(this, demon->m_uid);
	m_enemies.push_back(demon);
}

void Map::SpawnRandomDemon(std::string name, IntVec2 offset, IntVec2 area)
{
	Actor* demon = SpawnActor(ActorDefinition::GetByName(name));
	demon->m_position = FindOpenTile(offset, area);
	demon->m_orientation = EulerAngles(random.RollRandomFloatInRange(0.0f, 360.0f), 0.0f, 0.0f);
	demon->m_aiController = new AI(this, demon->m_uid);
	m_enemies.push_back(demon);
}

Vec3 Map::FindOpenTile(IntVec2 offset, IntVec2 area)
{
	Vec3 position = Vec3::ZERO;
	int x = 0;
	int y = 0;
	do 
	{
		do 
		{
			x = offset.x + random.RollRandomIntInRange(0, area.x);
			y = offset.y + random.RollRandomIntInRange(0, area.y);
		} 
		while (!InBounds(IntVec2(x,y)));
	}
	while (m_definition->m_image.GetTexelColor(IntVec2(x, y)) == Rgba8::WHITE);
	position.x = (float)x + 0.5f;
	position.y = (float)y + 0.5f;
	return position;
}

Actor* Map::SpawnProjectile(const std::string& name)
{
	Actor* projectile = SpawnActor(ActorDefinition::GetByName(name));
	if (projectile->m_definition->m_dieOnSpawn)
	{
		projectile->Die();
	}
	return projectile;
}

void Map::UpdateActors(float deltaSeconds)
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index])
		{
			if (m_actors[index]->m_controller)
			{
				continue;
			}
			if (m_actors[index]->m_aiController)
			{
				continue;
			}
			m_actors[index]->Update(deltaSeconds);
		}
	}
}

void Map::UpdatePlayers(float deltaSeconds)
{
	if (g_theGame->m_player[0])
	{
		g_theGame->m_player[0]->Update(deltaSeconds);
	}
	if (g_theGame->m_player[1])
	{
		g_theGame->m_player[1]->Update(deltaSeconds);
	}
}

void Map::UpdateAI(float deltaSeconds)
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index] && m_actors[index]->m_aiController)
		{
			m_actors[index]->m_aiController->Update(deltaSeconds);
		}
	}
}

void Map::UpdatePhysics(float deltaSeconds)
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index] && m_actors[index]->m_definition->m_simulated && !m_actors[index]->m_isDead)
		{
			m_actors[index]->UpdatePhysics(deltaSeconds);
		}
	}
}

void Map::UpdateCameras(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	for (int index = 0; index < g_theGame->m_numPlayers; index++)
	{
		if (g_theGame->m_player[index])
		{
			g_theGame->m_player[index]->UpdateCameras();
		}
	}
}

void Map::MarkDeadActors()
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		if (m_actors[index] && m_actors[index]->m_health <= 0.0f && m_actors[index]->m_isDead == false)
		{
			m_actors[index]->Die();
			if (m_actors[index]->m_definition->m_faction == DEMON)
			{
				CreateGoalHeatMap(*nearestBody);
			}
		}

		if (m_actors[index] && m_actors[index]->m_isDead && m_actors[index]->m_definition->m_faction != DEMON)
		{
			if (m_actors[index]->m_lifetimeStopwatch.HasDurationElapsed())
			{
				m_actors[index]->m_isDestroyed = true;
				m_actors[index]->m_lifetimeStopwatch.Stop();
			}
		}
	}
}

void Map::DeleteDestroyedActors()
{
	for (int index = 0; index < static_cast<int>(m_actors.size()); index++)
	{
		Actor* actor = m_actors[index];
		if (actor && actor->m_isDestroyed)
		{
			if (actor->m_definition->m_faction == Faction::MARINE)
			{
				// revive marine and move to a spawn point (not killed and not player-dependent)
				int player = dynamic_cast<Player*>(actor->m_controller)->m_playerIndex;
				delete actor;
				m_actors[index] = nullptr;
				SpawnRandomPlayer(player);
			}
			else
			{
				delete actor;
				m_actors[index] = nullptr;
			}
		}
	}
}

void Map::CreateMaskMap(TileHeatMap& out_maskMap)
{
	// set based on tile map
	for (int y = 1; y < m_dimensions.y - 1; y++)
	{
		for (int x = 1; x < m_dimensions.x - 1; x++)
		{
			if (IsTileSolid(x, y))
			{
				out_maskMap.Set(1.0f, x, y);
			}
			else
			{
				out_maskMap.Set(0.0f, x, y);
			}
		}
	}
}

// if there are no dead targets, then the heat map will be map cost
void Map::PopulateDistanceFieldMask(TileHeatMap& out_distanceField, std::vector<Actor*> const& targets, float maxCost, TileHeatMap& maskHeatMap)
{
	TileHeatMap& heatMap = out_distanceField;
	heatMap.SetAllValues(maxCost); // assumes all tiles can have this cost
	for (Actor const* a : targets)
	{
		if (a->m_isDead)
		{
			heatMap.Set(0.0f, a->GetTileCoords()); // assumes this is start tile
		}
	}
	bool changed = false; // flag to determine stopping condition

	do
	{
		changed = false; // assume nothing changes until it does
		for (int y = 1; y < m_dimensions.y - 1; y++)
		{
			for (int x = 1; x < m_dimensions.x - 1; x++)
			{
				if (maskHeatMap.Get(x, y) != 0.0f)
					continue;
				if (heatMap.Get(x, y) > heatMap.Get(x + 1, y) + 1.0f)
				{
					heatMap.Set(heatMap.Get(x + 1, y) + 1.0f, x, y);
					changed = true;
				}

				if (heatMap.Get(x, y) > heatMap.Get(x - 1, y) + 1.0f)
				{
					heatMap.Set(heatMap.Get(x - 1, y) + 1.0f, x, y);
					changed = true;
				}

				if (heatMap.Get(x, y) > heatMap.Get(x, y + 1) + 1.0f)
				{
					heatMap.Set(heatMap.Get(x, y + 1) + 1.0f, x, y);
					changed = true;
				}

				if (heatMap.Get(x, y) > heatMap.Get(x, y - 1) + 1.0f)
				{
					heatMap.Set(heatMap.Get(x, y - 1) + 1.0f, x, y);
					changed = true;
				}
			}
		}
	} while (changed);
}

void Map::RandomizeMap()
{
	IntVec2 dims = m_definition->m_image.GetDimensions();

	for (int y = 1; y < dims.y - 1; y++)
	{
		for (int x = 1; x < dims.x - 1; x++)
		{
			m_definition->m_image.SetTexelColor(IntVec2(x, y), Rgba8::WHITE);
		}
	}

	for (int y = 1; y < dims.y - 1; y++)
	{
		for (int x = 1; x < dims.x - 1; x++)
		{
			if (random.RollRandomFloatZeroToOne() > 0.5f)
			{
				m_definition->m_image.SetTexelColor(IntVec2(x, y), Rgba8(0, 0, 0));
			}
		}
	}

	Image image(m_definition->m_image.GetDimensions(), Rgba8::WHITE, "copy" );

	for (int i = 0; i < 2; i++)
	{
		// create next image
		for (int y = 1; y < dims.y - 1; y++)
		{
			for (int x = 1; x < dims.x - 1; x++)
			{
				if (CountNeighbors(x, y, m_definition->m_image) > 4)
				{
					image.SetTexelColor(IntVec2(x, y), Rgba8::WHITE);
				}
				else
				{
					image.SetTexelColor(IntVec2(x, y), Rgba8(0, 0, 0));
				}
			}
		}
		// copy next image back to original
		for (int y = 1; y < dims.y - 1; y++)
		{
			for (int x = 1; x < dims.x - 1; x++)
			{
				m_definition->m_image.SetTexelColor(IntVec2(x, y), image.GetTexelColor(IntVec2(x, y)));
			}
		}
	}
}

void Map::MakeReachable()
{
	IntVec2 dims = m_definition->m_image.GetDimensions();
	TileHeatMap reachable(dims);
	IntVec2 target = IntVec2::ZERO;
	bool unreachable = false;

	do 
	{
		// set up initial open map
		unreachable = false;
		for (int y = 1; y < dims.y - 1; y++)
		{
			for (int x = 1; x < dims.x - 1; x++)
			{
				if (m_definition->m_image.GetTexelColor(IntVec2(x, y)) != Rgba8::WHITE)
				{
					reachable.Set(HEAT_MAX, x, y);
				}
				else
				{
					reachable.Set(0.0f, x, y);
				}
			}
		}
		// find an open tile and flood fill
		for (int y = 1; y < dims.y - 1; y++)
		{
			for (int x = 1; x < dims.x - 1; x++)
			{
				if (reachable.Get(x, y) == 0.0f)
				{
					target = IntVec2(x, y);
					reachable.FloodFill(x, y, HEAT_MAX);
				}
			}
		}
		// look for any unreachable tiles and dig path
		for (int y = 1; y < dims.y - 1; y++)
		{
			for (int x = 1; x < dims.x - 1; x++)
			{
				if (reachable.Get(x, y) == 0.0f)
				{
					unreachable = true;
					DigPath(&m_definition->m_image, IntVec2(x, y), target, false);
				}
			}
		}
	} while (unreachable);
}

bool Map::InBounds(IntVec2 candidate)
{
	if (candidate.x < 1 || candidate.y < 1)
	{
		return false;
	}
	if (candidate.x > m_dimensions.x - 2 || candidate.y > m_dimensions.y - 2)
	{
		return false;
	}
	return true;
}

void Map::DigPath(Image* image, IntVec2 start, IntVec2 finish, bool stopOnOpen)
{
	IntVec2 current = start;
	IntVec2 newTile = start;
	float odds = 0.5f;
	float distance = (start - finish).GetLength();
	std::vector<IntVec2> directions;
	directions.push_back(IntVec2(1, 0));
	directions.push_back(IntVec2(-1, 0));
	directions.push_back(IntVec2(0, 1));
	directions.push_back(IntVec2(0, -1));

	while (current != finish)
	{
		bool notMoved = true;
		while (notMoved)
		{
			int test = random.RollRandomIntInRange(0, 3);
			if (InBounds(current + directions[test]))
			{
				newTile = current + directions[test];
				if (distance >= (newTile - finish).GetLength()) // || odds > random.Roll(0.0f, 1.0f))
				{
					if (image->GetTexelColor(newTile) != Rgba8::WHITE && stopOnOpen)
					{
						return; // stop if flagged to do partial connection only
					}
					image->SetTexelColor(newTile, Rgba8(0, 0, 0));
					notMoved = false;
					break;
				}

			}
		}
		odds = Clamp(odds, 0.0f, 1.0f);
		distance = (newTile - finish).GetLength();
		current = newTile;
	}
}

int Map::CountNeighbors(int x, int y, Image& image)
{
	int count = 0;
	for (int i = -1; i < 2; i++)
	{
		for (int j = -1; j < 2; j++)
		{
			if (i == 0 && j == 0)
			{
				continue;
			}
			if (image.GetTexelColor(IntVec2(x + i, y + j)) == Rgba8::WHITE)
			{
				count++;
			}
		}
	}
	return count;
}

void Map::CreateGoalHeatMap(TileHeatMap& reachableMap)
{
	float maxCost = HEAT_MAX;

	TileHeatMap maskMap(m_dimensions);
	CreateMaskMap(maskMap);
	PopulateDistanceFieldMask(reachableMap, m_enemies, maxCost, maskMap);
}

Vec3 Map::PickTarget(TileHeatMap& heatMap, IntVec2 tile)
{
	Vec3 target = Vec3::ZERO;
	int x = tile.x;
	int y = tile.y;
	IntVec2 low(x, y); // safe
	float lowest = HEAT_MAX;

	if (lowest > heatMap.Get(x + 1, y))
	{
		lowest = heatMap.Get(x + 1, y);
		low.x = x + 1;
		low.y = y;
	}

	if (lowest > heatMap.Get(x - 1, y))
	{
		lowest = heatMap.Get(x - 1, y);
		low.x = x - 1;
		low.y = y;
	}

	if (lowest > heatMap.Get(x, y + 1))
	{
		lowest = heatMap.Get(x, y + 1);
		low.x = x;
		low.y = y + 1;
	}

	if (lowest > heatMap.Get(x, y - 1))
	{
		lowest = heatMap.Get(x, y - 1);
		low.x = x;
		low.y = y - 1;
	}

	target.x = (float)(low.x + 0.5f);
	target.y = (float)(low.y + 0.5f);
	return target;
}
