#pragma once
#include "Engine/Core/Stopwatch.hpp"

class WeaponDefinition;
struct Vec3;
class Actor;

class Weapon
{
public:
	Weapon( const WeaponDefinition* definition );
	~Weapon();

	void Fire( const Vec3& position, const Vec3& forward, Actor* owner );
	Vec3 GetRandomDirectionInCone( const Vec3& position, const Vec3& forward, float angle ) const;

	WeaponDefinition const* m_definition = nullptr;
	Stopwatch m_refireStopwatch;
	bool m_idle = true;
};
