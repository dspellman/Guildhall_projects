#pragma once
#include "Engine/Core/XmlUtils.hpp"
#include "ActorDefinition.hpp"
#include "Engine/Math/Vec3.hpp"
#include "Engine/Math/EulerAngles.hpp"

//------------------------------------------------------------------------------------------------
class SpawnInfo
{
public:
	SpawnInfo();
	SpawnInfo( const ActorDefinition* definition, const Vec3& position = Vec3::ZERO, const EulerAngles& orientation = EulerAngles::ZERO, const Vec3& velocity = Vec3::ZERO );
	SpawnInfo( const char* definitionName, const Vec3& position = Vec3::ZERO, const EulerAngles& orientation = EulerAngles::ZERO, const Vec3& velocity = Vec3::ZERO );

	bool LoadFromXmlElement( const XmlElement& element );

	const ActorDefinition* m_definition = nullptr;
	Vec3 m_position = Vec3::ZERO;
	EulerAngles m_orientation = EulerAngles::ZERO;
	Vec3 m_velocity = Vec3::ZERO;
};




