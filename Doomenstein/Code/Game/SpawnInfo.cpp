#include "SpawnInfo.hpp"
#include "Engine/Core/EngineCommon.hpp"

SpawnInfo::SpawnInfo()
{

}

SpawnInfo::SpawnInfo(const ActorDefinition* definition, const Vec3& position /*= Vec3::ZERO*/, const EulerAngles& orientation /*= EulerAngles::ZERO*/, const Vec3& velocity /*= Vec3::ZERO */)
	: m_definition(definition), m_position(position), m_orientation(orientation), m_velocity(velocity)
{

}

SpawnInfo::SpawnInfo(const char* definitionName, const Vec3& position /*= Vec3::ZERO*/, const EulerAngles& orientation /*= EulerAngles::ZERO*/, const Vec3& velocity /*= Vec3::ZERO */)
	: m_position(position), m_orientation(orientation), m_velocity(velocity)
{
	m_definition = ActorDefinition::GetByName(definitionName);
}

bool SpawnInfo::LoadFromXmlElement(const XmlElement& element)
{
	std::string actor = ParseXmlAttribute(element, "actor", "UNKNOWN");
	if (actor.compare("UNKNOWN") == 0)
	{
		// log error
	}
	else
	{
		m_definition = ActorDefinition::GetByName(actor);
	}
	m_position = ParseXmlAttribute(element, "position", Vec3::ZERO);
	m_orientation = ParseXmlAttribute(element, "orientation", EulerAngles::ZERO);

	return true;
}

