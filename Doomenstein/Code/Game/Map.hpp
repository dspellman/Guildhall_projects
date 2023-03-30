#pragma once
#include "Game/GameCommon.hpp"
#include "Game/Tile.hpp"
#include "Engine/Math/AABB3.hpp"
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Math/Vec3.hpp"
#include <vector>
#include "Game/Actor.hpp"
#include "Engine/Math/RaycastUtils.hpp"
#include "Game/SpawnInfo.hpp"
#include "Game/ActorUID.hpp"
#include "Engine/Core/TileHeatMap.hpp"

//------------------------------------------------------------------------------------------------
class Game;
class MapDefinition;
class Shader;
class Texture;
struct AABB2;
struct Vertex_PNCU;
class IndexBuffer;
class VertexBuffer;
class Player;
class TileHeatMap;

enum ISSOLID_FLAGS
{
	TESTXY_NOTZ = 1,
};

struct RaycastFilter
{
	RaycastFilter(Actor const* type = nullptr)
		: m_ignoreActor(type) {};
	Actor const* m_ignoreActor = nullptr;
};

//------------------------------------------------------------------------------------------------
class Map
{
public:
	Map( Game* game, const MapDefinition* definition );
	~Map();

	Actor* SpawnActor(const SpawnInfo& spawnInfo);
	int NextSalt();
	void DestroyActor(const ActorUID uid);
	Actor* FindActorByUID(const ActorUID uid) const;
	Actor* GetClosestVisibleEnemy(Actor const& actor) const;

	void Update( float deltaSeconds );
	void Render();

	bool IsTileSolid(int x, int y) const;
	AABB2 GetAABB2ForTile2D(int x, int y) const;
	bool DoesActorCollide(Actor* actor);
	void PushActorsOutOfEachOther(Actor* a, Actor* b);
	void CollideActors();
	void CollideActors(Actor* actorA, Actor* actorB);
	void CollideActorsWithMap();
	void CollideActorWithMap(Actor* actor);
	void DeleteDestroyedActors();

	void CreateMaskMap(TileHeatMap& out_maskMap);
	void PopulateDistanceFieldMask(TileHeatMap& out_distanceField, std::vector<Actor*> const& targets, float maxCost, TileHeatMap& maskHeatMap);
	Player* GetPlayer();
	Game* GetGame();

	void CreateTiles();
	void CreateGeometry(); // the vertices and indices
	void CreateBuffers();

	RaycastResult3D RaycastAll(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, Actor** actorHit  = nullptr, RaycastFilter filter = RaycastFilter()) const;
	RaycastResult3D RaycastWorldXY(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, RaycastFilter filter = RaycastFilter()) const;
	RaycastResult3D RaycastWorldZ(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, RaycastFilter filter = RaycastFilter()) const;
	RaycastResult3D RaycastWorldActors(const Vec3& startPos, const Vec3& fwdNormal, float maxDist, Actor** actorHit, RaycastFilter filter = RaycastFilter()) const;
	void ToggleRaycastDebug();
	void DebugPossessNext();

	RaycastResult2D RaycastVsTiles2D(Vec3 startPosition, Vec3 forwardNormal, float maxDist, int flags) const;
	bool InSolidTile(int tileXcoord, int tileYcoord, int flags) const;
	bool OutOfBounds(Vec3 point) const;
	bool InBounds(Vec3 point) const;
	bool InBounds(IntVec2 candidate);
	void DigPath(Image* image, IntVec2 start, IntVec2 finish, bool stopOnOpen);
	const Tile& GetTileXY(int x, int y) const;

	Image const& GetMapImage() const;

	// Info
// 	Player* m_player[2];
//	int m_up = 0;
	Game* m_game = nullptr;

	// Map
	const MapDefinition* m_definition = nullptr;
	std::vector<Tile> m_tiles;
	IntVec2 m_dimensions;

	// Rendering
	std::vector<Vertex_PNCU> m_vertexes;
	std::vector<unsigned int> m_indexes;
	const Texture* m_texture = nullptr;
	Shader* m_shader = nullptr;
	VertexBuffer* m_immediateVBO_PCU = nullptr;
	VertexBuffer* m_immediateVBO_PNCU = nullptr;
	IndexBuffer* m_indexBuffer = nullptr;
	unsigned int m_indexCount = 0;

	bool m_debugRaycasts = false;
	float m_debugRadius = 0.01f;
	float m_debugDuration = 10.0f;

public:
	//game objects
	std::vector<Actor*> m_actors;
	int m_actorSalt = 0x0000FFFE;
	std::vector<Actor*> m_enemies;
	TileHeatMap* nearestBody = nullptr;

	void SpawnPlayer(int index);
	void SpawnRandomPlayer(int index);
	void SpawnDemon(SpawnInfo const spawnInfo);
	void SpawnRandomDemon(std::string name, IntVec2 offset, IntVec2 area);
	Vec3 FindOpenTile(IntVec2 offset, IntVec2 area);
	Actor* SpawnProjectile(const std::string& name);
	void CreateGoalHeatMap(TileHeatMap& reachableMap);
	Vec3 PickTarget(TileHeatMap& nearestBody, IntVec2 tile);

private:
	void UpdateActors(float deltaSeconds);
	void UpdatePlayers(float deltaSeconds);
	void UpdateAI(float deltaSeconds);
	void UpdatePhysics(float deltaSeconds);
	void UpdateCameras(float deltaSeconds);
	void MarkDeadActors();

	void RandomizeMap();
	void MakeReachable();
	int CountNeighbors(int x, int y, Image& image);
};
