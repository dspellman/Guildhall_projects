#include "Game/Actor.hpp"
#include "Engine/Math/RandomNumberGenerator.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Game/GameCommon.hpp"
#include "Controller.hpp"
#include "SpawnInfo.hpp"
#include "Game/Player.hpp"
#include "Game/AI.hpp"
#include "Game.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Map.hpp"
#include "Engine/Renderer/DebugRenderMode.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Math/Easing.hpp"

Actor::~Actor()
{
	if (m_aiController)
	{
		delete m_aiController;
		m_aiController = nullptr;
	}
	if (m_actorClock)
	{
		delete m_actorClock;
		m_actorClock = nullptr;
	}
}

Actor::Actor(Map* map, const SpawnInfo& spawnInfo)
	: m_map(map)
{
	m_controller = nullptr;
	m_position = spawnInfo.m_position;
	m_orientation = spawnInfo.m_orientation;
	m_velocity = spawnInfo.m_velocity;
	m_definition = spawnInfo.m_definition;
	m_health = m_definition->m_health;

	for (int index = 0; index < static_cast<int>(m_definition->m_weaponDefinitions.size()); index++)
	{
		if (m_definition->m_weaponDefinitions[index])
		{
			push(new Weapon(m_definition->m_weaponDefinitions[index]), m_weapons);
		}
	}

	if (m_weapons.size())
	{
		m_equippedWeaponIndex = 0;
	}
	m_actorClock = new Clock(g_systemClock);
}

void Actor::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	if (m_definition->m_name == "PlasmaGrenadeProjectile")
	{
		AddForce(Vec3(0.0f, 0.0f, -1.0f) * GRAVITY);
		m_lifeTime -= deltaSeconds;
		if (m_lifeTime < 0.0f)
		{
			Die();
		}
	}
}

void Actor::UpdatePhysics(float deltaSeconds)
{
// 	AddForce(-m_velocity * m_definition->m_drag);

	if (m_controller)
	{
		Player* player = dynamic_cast<Player*>(m_controller);
		if (player && player->m_freeFlyCameraMode)
		{
			return;
		}
	}

	AddForce(-m_velocity * m_definition->m_drag);
	m_velocity += m_acceleration * deltaSeconds;
	float length = m_velocity.GetLength();
	if (m_definition->m_flying == false)
	{
		m_velocity.z = 0.0f;
	}
	m_velocity = m_velocity.GetNormalized() * length;
	m_position += m_velocity * deltaSeconds;

	m_acceleration = Vec3::ZERO;
}

Mat44 Actor::GetModelMatrix() const
{
	Mat44 orient = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp();
	orient.Orthonormalize_XFwd_YLeft_ZUp();
	Mat44 trans;
	trans.SetTranslation3D(m_position);
	trans.Append(orient);
	return trans;
}

Vec3 Actor::GetForward() const
{
	return m_orientation.GetForwardNormal();
}

Vec3 Actor::GetLeft() const
{
	Vec3 i;
	Vec3 j;
	Vec3 k;
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);
	return j;
}

Vec3 Actor::BuildNoneBasis(Vec3& i, Vec3& j, Vec3& k) const
{
	m_orientation.GetAsVectors_XFwd_YLeft_ZUp(i, j, k);

	Vec3 pivot(m_position.x, m_position.y, 0.0f); // we should calculate centered image then move to actual pivot here)
	Vec3 CP = pivot - m_map->GetPlayer()->m_position; // camera is player location?
	CP.z = 0.0f;
	CP.Normalize();
	return CP;
}

Vec3 Actor::BuildFacingBasis(Vec3& i, Vec3& j, Vec3& k) const
{
	Vec3 pivot(m_position.x, m_position.y, 0.0f); // we should calculate centered image then move to actual pivot here, this assume (0.5, 0.0)
	Vec3 PC = m_map->GetPlayer()->m_position - pivot; // camera is player location?
	PC.z = 0.0f;
	PC.Normalize();
	i = PC;
	j = Vec3(-i.y, i.x, 0.0f);
	k = Vec3(0.0f, 0.0f, 1.0f);

	Vec3 CP = pivot - m_map->GetPlayer()->m_position; // camera is player location?
	CP.z = 0.0f;
	CP.Normalize();
	return CP;
}

Vec3 Actor::BuildAlignedBasis(Vec3& i, Vec3& j, Vec3& k) const
{
	i = -1.0f * m_map->GetPlayer()->m_orientation.GetForwardNormal();
	i.z = 0.0f;
	i.Normalize();
	j = Vec3(-i.y, i.x, 0.0f);
	k = Vec3(0.0f, 0.0f, 1.0f);

	Vec3 pivot(m_position.x, m_position.y, 0.0f); // we should calculate centered image then move to actual pivot here, this assume (0.5, 0.0)
	Vec3 CP = pivot - m_map->GetPlayer()->m_position; // camera is player location?
	CP.z = 0.0f;
	CP.Normalize();
	return CP;
}

bool Actor::Damage(float damage)
{
	if (damage == 0.0f)
	{
		return false;
	}
	m_health -= damage;
	if (m_health <= 0.0f)
	{
			//Die();
		return true;
	}
	SoundID painSound = g_theAudio->CreateOrGetSound(m_definition->m_hurtSoundName);
	g_theAudio->StartSoundAt(painSound, m_position);
	return false;
}

void Actor::Die()
{
	if (m_isDead)
	{
		return; // just in case we die again
	}
	m_isDead = true;
	m_health = 0.0f; // for display
	// if kill, then increment kills for other player, increment deaths
	// handle timer for corpse showing then destroy?
	m_lifetimeStopwatch.Start(m_definition->m_corpseLifetime);
	SetAnimation("Death");
	SoundID deathSound = g_theAudio->CreateOrGetSound(m_definition->m_deathSoundName);
	g_theAudio->StartSoundAt(deathSound, m_position);

	if (m_definition->m_name == "PlasmaGrenadeProjectile")
	{
		// spawn explosion animation and sound
		for (Actor* a : m_map->m_enemies)
		{
			if (a->m_isDead == false)
			{
				a->Damage(GrenadeDamage((m_position - a->m_position).GetLength(), 10.0f, 120.0f));
			}
		}
	}
}

float Actor::GrenadeDamage(float distance, float range, float maxDamage)
{
	float damage = SmoothStop2(ClampZeroToOne(RangeMap(distance, 0.0f, range, 1.0f, 0.0f)));
	damage *= maxDamage;
	return damage;
}

void Actor::AddForce(const Vec3& force)
{
	m_acceleration += force;
}

// called when hit with something, multiply by reverse incident normal
void Actor::AddImpulse(const Vec3& impulse)
{
	m_velocity += impulse;
}

void Actor::OnCollide(Actor* other)
{
	other->Damage(random.RollRandomFloatInRange(m_definition->m_damageOnCollide));
	if (other->m_health < 0.0f)
	{
		other->Die();
	}
}

void Actor::OnPossessed(Controller* controller)
{
	if (!controller)
	{
		ERROR_AND_DIE("Bad controller");
	}
	m_controller = controller;
	m_controller->Possess(this);
}

void Actor::OnUnpossessed(Controller* controller)
{
	// how do we destroy AI controllers? (we don't)
	if (controller)
	{
		controller->Possess(nullptr);
	}
	m_controller = nullptr;
}

void Actor::MoveInDirection(Vec3 direction, float speed)
{
	AddForce(direction * speed * m_definition->m_drag); // what about delta seconds or force or impulse?
	SetAnimation("Walk");
}

Weapon* Actor::GetEquippedWeapon()
{
	return m_equippedWeaponIndex != -1 ? m_weapons[m_equippedWeaponIndex] : nullptr; // which weapon?
}


Weapon* Actor::GetAllternateWeapon()
{
	if ((int)m_weapons.size() < 5)
	{
		m_alternateWeapon = -1; // should not matter
		return nullptr;
	}
	m_alternateWeapon = m_grenadeCount == 0 ? 3 : 4;
	return m_weapons[m_alternateWeapon]; // grenade if available, otherwise kick default
}

void Actor::EquipWeapon(int weaponIndex)
{
	if (weaponIndex < static_cast<int>(m_weapons.size()) && weaponIndex > -1)
	{
		m_equippedWeaponIndex = weaponIndex;
	}
}

void Actor::EquipNextWeapon()
{
	if (static_cast<int>(m_weapons.size()) == 0)
	{
		return;
	}
	if (m_equippedWeaponIndex == -1)
	{
		m_equippedWeaponIndex = 0;
	}
	m_equippedWeaponIndex++;
	if (m_equippedWeaponIndex == static_cast<int>(m_weapons.size()))
	{
		m_equippedWeaponIndex = 0;
	}
}

void Actor::EquipPreviousWeapon()
{
	if (static_cast<int>(m_weapons.size()) == 0)
	{
		return;
	}
	if (m_equippedWeaponIndex == -1)
	{
		m_equippedWeaponIndex = 0;
	}
	m_equippedWeaponIndex--;
	if (m_equippedWeaponIndex < 0)
	{
		m_equippedWeaponIndex = static_cast<int>(m_weapons.size()) - 1;
	}
}

void Actor::Attack()
{
	m_weaponStart = m_actorClock->GetTotalTime();
}

void Actor::SetAnimation(std::string const& animation)
{
	if (m_animation == "Death")
	{
		return;
	}
	if ((m_animation == animation || animation == "Idle") && (m_animationDuration > static_cast<float>(m_actorClock->GetTotalTime() - m_animationStart)))
	{
		return;
	}

	m_animation = animation;
	m_animationStart = m_actorClock->GetTotalTime();
}

void Actor::Render() const
{
	if (m_definition->m_visible == false)
	{
		return; // don't render invisible objects at all
	}

	if (m_controller)
	{
		Player* player = dynamic_cast<Player*>(m_controller);
		if (player && player == g_theGame->m_renderingPlayer && !player->m_freeFlyCameraMode)
		{
			return;
		}
		else
		{
			//DebugAddMessage(std::to_string(player->m_playerIndex), 1.0f, Rgba8::WHITE, Rgba8::WHITE);
		}
	}

	Rgba8 tint = Rgba8::BLUE;
	std::vector<Vertex_PNCU> vertices;

//	if (m_definition->m_faction == Faction::DEMON || m_definition->m_faction == Faction::MARINE || m_definition->m_name == "PlasmaProjectile")
	if (m_definition->m_name == "PlasmaProjectile" && ((m_position - m_owner->m_position).GetLength() < 0.5f) && (g_theGame->m_renderingPlayer->GetActor() == m_owner))
	{
//		return;
	}

		{
		tint = Rgba8::WHITE; //m_isDead ? Rgba8::RED.dim(0.5) : Rgba8::RED;
 		Vec3 i, j, k;
 		Vec3 lookAt;
		if (m_definition->m_billboardType == NONE)
		{
			lookAt = BuildNoneBasis(i, j, k);
		}
		if (m_definition->m_billboardType == FACING)
		{
			lookAt = BuildFacingBasis(i, j, k);
		}
		if (m_definition->m_billboardType == ALIGNED)
		{
			lookAt = BuildAlignedBasis(i, j, k);
		}

		Vec3 zeroed(m_position);
		if (m_definition->m_name == "PlasmaProjectile")
		{
			zeroed.z -= 0.15f;
		}
		if (m_definition->m_name == "BulletHit")
		{
			zeroed += i * 0.01f;
		}
		Mat44 billboardMatrix(i, j, k, zeroed);

		Mat44 localXform = m_orientation.GetAsMatrix_XFwd_YLeft_ZUp().GetOrthonormalInverse();
 		Vec3 animationFacing = localXform.TransformVectorQuantity3D(lookAt);

		// choose animation based on direction
		m_groupIndex = (m_animation == "Idle") ? m_definition->GetGroupIndexByName("walk") : m_definition->GetGroupIndexByName(m_animation);
		SpriteAnimationDefinition const& animDef = m_definition->m_spriteAnimationGroupDefinitions[m_groupIndex].GetAnimationForDirection(animationFacing);
		m_animationDuration = animDef.GetDuration();
		// if duration is over, then get animDef[0] instead and present that--but how to set the variables in here?  How to do this in Update?
		SpriteDefinition const& spriteDef = (m_animation == "Idle") ? animDef.GetSpriteDefAtTime(0.0f) : animDef.GetSpriteDefAtTime(static_cast<float>(m_actorClock->GetTotalTime() - m_animationStart));
		std::string str = std::to_string(animDef.m_currentFrame);		

		// from sprite code
		g_theRenderer->SetModelMatrix(billboardMatrix);
		g_theRenderer->SetModelColor(tint);
		g_theRenderer->BindTexture(&spriteDef.GetTexture());
		g_theRenderer->SetBlendMode(BlendMode::OPAQUE);

		// from below code
		g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
		g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
		g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);

		if (animDef.m_shader)
		{
			g_theRenderer->BindShaderByName("Data/Shaders/SpriteLit");
			//g_theRenderer->BindShaderByName(animDef.m_shader->m_config.m_name.c_str());
		}
		else
		{
			g_theRenderer->BindShaderByName("Data/Shaders/SpriteLit");
		}

		Vec2 spriteSize = m_definition->m_spriteSize;
		if (m_definition->m_name == "PlasmaGrenadeProjectile" && m_animation == "Death")
		{
			spriteSize *= 16.0f;
		}

		Vec3 ll(0.0f, -spriteSize.x * 0.5f, 0.0f);
		Vec3 lr(0.0f, spriteSize.x * 0.5f, 0.0f);
		Vec3 ul(ll);
		ul.z = spriteSize.y;
		Vec3 ur(lr);
		ur.z = spriteSize.y;
		if (m_definition->m_name == "BulletHit" || m_definition->m_name == "BloodSplatter" || (m_definition->m_name == "PlasmaGrenadeProjectile" && m_animation == "Death"))
		{
			float adjustment = m_definition->m_spriteSize.y * m_definition->m_spritePivot.y;
			ll.z -= adjustment;
			lr.z -= adjustment;
			ul.z -= adjustment;
			ur.z -= adjustment;
		}
		if (m_definition->m_name == "PlasmaGrenadeProjectile" && m_animation == "Death")
		{
			float adjustment = m_definition->m_spriteSize.y * m_definition->m_spritePivot.y * 8.0f;
			ll.z -= adjustment;
			lr.z -= adjustment;
			ul.z -= adjustment;
			ur.z -= adjustment;
		}

		AddVertsForRoundedQuad3D(vertices, ul, ll, lr, ur, tint, spriteDef.GetUVs());
		g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());

		g_theRenderer->BindShader(nullptr);
	}

// 	g_theRenderer->SetSamplerMode(SamplerMode::POINTCLAMP);
// 	g_theRenderer->SetBlendMode(BlendMode::OPAQUE);
//	g_theRenderer->BindTexture(nullptr);
// 	g_theRenderer->SetDepthStencilState(DepthTest::LESS_EQUAL, true);
// 	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
// 
// 	g_theRenderer->DrawVertexArray(static_cast<int>(vertices.size()), vertices.data());
// 
// 	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::WIREFRAME, WindingOrder::COUNTERCLOCKWISE);
// 
// 	g_theRenderer->DrawVertexArray(static_cast<int>(wireVertices.size()), wireVertices.data());
// 
// 	g_theRenderer->SetRasterizerState(CullMode::BACK, FillMode::SOLID, WindingOrder::COUNTERCLOCKWISE);
}

IntVec2 const Actor::GetTileCoords() const
{
	IntVec2 tile = IntVec2(RoundDownToInt(m_position.x), RoundDownToInt(m_position.y));
	return tile;
}
