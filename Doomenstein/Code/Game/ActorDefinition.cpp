#include "Game/ActorDefinition.hpp"
#include "Game/WeaponDefinition.hpp"
#include "Engine/Core/EngineCommon.hpp"

std::vector<ActorDefinition*> ActorDefinition::s_definitions;

XmlElement const* ActorDefinition::ParseAppearance(XmlElement const* SubElement)
{
	// appearance tag
	m_spriteSize = ParseXmlAttribute(*SubElement, "size", Vec2::ONE);
	m_spritePivot = ParseXmlAttribute(*SubElement, "pivot", Vec2(0.5f, 0.0f));
	std::string billboardType = ParseXmlAttribute(*SubElement, "billboardType", "junk");
	if (billboardType == "Facing")
	{
		m_billboardType = BillboardType::FACING;
	}
	else if (billboardType == "Aligned")
	{
		m_billboardType = BillboardType::ALIGNED;
	}
	else
	{
		m_billboardType = BillboardType::NONE;
	}
	m_renderDepth = ParseXmlAttribute(*SubElement, "renderDepth", true);
	m_renderLit = ParseXmlAttribute(*SubElement, "renderLit", true);
	m_renderRounded = ParseXmlAttribute(*SubElement, "renderRoundedNormals", false);

	XmlElement const* loopElement = SubElement->FirstChildElement();
	while (loopElement != nullptr)
	{
		SpriteAnimationGroupDefinition animationGroup;
		animationGroup.LoadFromXmlElement(*loopElement);
		m_spriteAnimationGroupDefinitions.push_back(animationGroup);
		loopElement = loopElement->NextSiblingElement();
	}

	return SubElement;
}

int ActorDefinition::GetGroupIndexByName(const std::string& name) const
{
	for (int index = 0; index < static_cast<int>(m_spriteAnimationGroupDefinitions.size()); index++)
	{
		if (m_spriteAnimationGroupDefinitions[index].m_name == name)
		{
			return index;
		}
	}
	return 0;
}

bool ActorDefinition::LoadFromXmlElement(const XmlElement& element)
{
	XmlElement const* SubElement = nullptr;

	m_name = ParseXmlAttribute(element, "name", "UNKNOWN");
	if (m_name.compare("UNKNOWN") == 0)
	{
		// log error
		return false;
	}

	if (m_name.compare("PlasmaProjectile") == 0)
	{
		m_faction = Faction::NEUTRAL;
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.01f);

		SubElement = element.FirstChildElement();
		// collision tag
		m_physicsRadius = ParseXmlAttribute(*SubElement, "radius", 0.3f);
		m_physicsHeight = ParseXmlAttribute(*SubElement, "height", 0.75f);
		m_collidesWithWorld = ParseXmlAttribute(*SubElement, "collidesWithWorld", false);
		m_collidesWithActors = ParseXmlAttribute(*SubElement, "collidesWithActors", false);
		std::string damageRange = ParseXmlAttribute(*SubElement, "damageOnCollide", "0.0~0.0");
		Strings rangeValues = SplitStringOnDelimiter(damageRange, '~');
		m_damageOnCollide = FloatRange(std::stof(rangeValues[0]), std::stof(rangeValues[1]));
		m_impulseOnCollide = ParseXmlAttribute(*SubElement, "impulseOnCollide", 0.0f);
		m_dieOnCollide = ParseXmlAttribute(*SubElement, "dieOnCollide", true);

		SubElement = SubElement->NextSiblingElement();
		// physics tag
		m_simulated = ParseXmlAttribute(*SubElement, "simulated", true);
		m_turnSpeed = ParseXmlAttribute(*SubElement, "turnSpeed", 180.0f);
		m_flying = ParseXmlAttribute(*SubElement, "flying", false);
		m_drag = ParseXmlAttribute(*SubElement, "drag", 9.0f);

		SubElement = SubElement->NextSiblingElement();
		SubElement = ParseAppearance(SubElement);
		return true;
	}

	if (m_name.compare("PlasmaGrenadeProjectile") == 0)
	{
		m_faction = Faction::NEUTRAL;
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.01f);

		SubElement = element.FirstChildElement();
		// collision tag
		m_physicsRadius = ParseXmlAttribute(*SubElement, "radius", 0.3f);
		m_physicsHeight = ParseXmlAttribute(*SubElement, "height", 0.75f);
		m_collidesWithWorld = ParseXmlAttribute(*SubElement, "collidesWithWorld", false);
		m_collidesWithActors = ParseXmlAttribute(*SubElement, "collidesWithActors", false);
		std::string damageRange = ParseXmlAttribute(*SubElement, "damageOnCollide", "0.0~0.0");
		Strings rangeValues = SplitStringOnDelimiter(damageRange, '~');
		m_damageOnCollide = FloatRange(std::stof(rangeValues[0]), std::stof(rangeValues[1]));
		m_impulseOnCollide = ParseXmlAttribute(*SubElement, "impulseOnCollide", 0.0f);
		m_dieOnCollide = ParseXmlAttribute(*SubElement, "dieOnCollide", true);

		SubElement = SubElement->NextSiblingElement();
		// physics tag
		m_simulated = ParseXmlAttribute(*SubElement, "simulated", true);
		m_turnSpeed = ParseXmlAttribute(*SubElement, "turnSpeed", 180.0f);
		m_flying = ParseXmlAttribute(*SubElement, "flying", false);
		m_drag = ParseXmlAttribute(*SubElement, "drag", 9.0f);

		SubElement = SubElement->NextSiblingElement();
		SubElement = ParseAppearance(SubElement);
		return true;
	}

	if (m_name == "ShotgunShells")
	{
		m_faction = Faction::NEUTRAL;
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.01f);

		SubElement = element.FirstChildElement();
		// collision tag
		m_physicsRadius = ParseXmlAttribute(*SubElement, "radius", 0.3f);
		m_physicsHeight = ParseXmlAttribute(*SubElement, "height", 0.75f);
		m_collidesWithWorld = ParseXmlAttribute(*SubElement, "collidesWithWorld", false);
		m_collidesWithActors = ParseXmlAttribute(*SubElement, "collidesWithActors", false);
		std::string damageRange = ParseXmlAttribute(*SubElement, "damageOnCollide", "0.0~0.0");
		Strings rangeValues = SplitStringOnDelimiter(damageRange, '~');
		m_damageOnCollide = FloatRange(std::stof(rangeValues[0]), std::stof(rangeValues[1]));
		m_impulseOnCollide = ParseXmlAttribute(*SubElement, "impulseOnCollide", 0.0f);
		m_dieOnCollide = ParseXmlAttribute(*SubElement, "dieOnCollide", true);

		SubElement = SubElement->NextSiblingElement();
		// physics tag
		m_simulated = ParseXmlAttribute(*SubElement, "simulated", true);
		m_turnSpeed = ParseXmlAttribute(*SubElement, "turnSpeed", 180.0f);
		m_flying = ParseXmlAttribute(*SubElement, "flying", false);
		m_drag = ParseXmlAttribute(*SubElement, "drag", 9.0f);

		SubElement = SubElement->NextSiblingElement();
		SubElement = ParseAppearance(SubElement);
		return true;
	}

	if (m_name.compare("BulletHit") == 0)
	{
		m_faction = Faction::NEUTRAL;
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_dieOnSpawn = ParseXmlAttribute(element, "dieOnSpawn", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.01f);

		SubElement = element.FirstChildElement();
		SubElement = ParseAppearance(SubElement);
		return true;
	}

	if (m_name.compare("BloodSplatter") == 0)
	{
		m_faction = Faction::NEUTRAL;
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_dieOnSpawn = ParseXmlAttribute(element, "dieOnSpawn", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 0.01f);

		SubElement = element.FirstChildElement();
		SubElement = ParseAppearance(SubElement);
		return true;
	}

	if (m_name.compare("SpawnPoint") == 0)
	{
		return true;
	}

	if ((m_name.compare("Marine") == 0) || (m_name.compare("Demon") == 0) || (m_name.compare("Boss") == 0))
	{
		std::string faction = ParseXmlAttribute(element, "faction", "Neutral");
		if (faction.compare("Marine") == 0)
		{
			m_faction = Faction::MARINE;
		}
		else
		{
			m_faction = Faction::DEMON;
		}
		m_health = ParseXmlAttribute(element, "health", 100.0f);
		m_canBePossessed = ParseXmlAttribute(element, "canBePossessed", false);
		m_corpseLifetime = ParseXmlAttribute(element, "corpseLifetime", 1.0f);

		SubElement = element.FirstChildElement();
		// collision tag
		m_physicsRadius = ParseXmlAttribute(*SubElement, "radius", 0.3f);
		m_physicsHeight = ParseXmlAttribute(*SubElement, "height", 0.75f);
		m_collidesWithWorld = ParseXmlAttribute(*SubElement, "collidesWithWorld", true);
		m_collidesWithActors = ParseXmlAttribute(*SubElement, "collidesWithActors", true);

		SubElement = SubElement->NextSiblingElement();
		// physics tag
		m_simulated = ParseXmlAttribute(*SubElement, "simulated", true);
		m_walkSpeed = ParseXmlAttribute(*SubElement, "walkSpeed", 1.75f);
		m_runSpeed = ParseXmlAttribute(*SubElement, "runSpeed", 5.5f);
		m_turnSpeed = ParseXmlAttribute(*SubElement, "turnSpeed", 180.0f);
		m_drag = ParseXmlAttribute(*SubElement, "drag", 9.0f);

		SubElement = SubElement->NextSiblingElement();
		// camera tag
		m_eyeHeight = ParseXmlAttribute(*SubElement, "eyeHeight", 0.5f);
		m_cameraFOVDegrees = ParseXmlAttribute(*SubElement, "cameraFOV", 60.0f);
	}

	if (m_name.compare("Marine") == 0)
	{
		SubElement = SubElement->NextSiblingElement();
 		SubElement = ParseAppearance(SubElement);

		SubElement = SubElement->NextSiblingElement();
		// Sounds tag
		XmlElement const* loopElement = SubElement->FirstChildElement();
		std::string sound;
//		while (loopElement != nullptr)
		{
			//Sound tag
			sound = ParseXmlAttribute(*loopElement, "sound", "junk");
			m_hurtSoundName = ParseXmlAttribute(*loopElement, "name", "junk");
			loopElement = loopElement->NextSiblingElement();
			sound = ParseXmlAttribute(*loopElement, "sound", "junk");
			m_deathSoundName = ParseXmlAttribute(*loopElement, "name", "junk");
			loopElement = loopElement->NextSiblingElement();
		}

		SubElement = SubElement->NextSiblingElement();
		// inventory tag
		SubElement = SubElement->FirstChildElement();
		// weapon tag
		while (SubElement)
		{
			std::string weapon = ParseXmlAttribute(*SubElement, "name", "NONE");
			if (weapon.compare("NONE") == 0)
			{
				ERROR_AND_DIE("bad weapon");
			}
			else
			{
				WeaponDefinition const* weaponDefinition = WeaponDefinition::GetByName(weapon);
				m_weaponDefinitions.push_back(weaponDefinition);
			}
			SubElement = SubElement->NextSiblingElement();
		}
	}

	if (m_name.compare("Demon") == 0 || (m_name.compare("Boss") == 0))
	{
		SubElement = SubElement->NextSiblingElement();
		// AI tag
		m_aiEnabled = ParseXmlAttribute(*SubElement, "aiEnabled", false);
		m_sightRadius = ParseXmlAttribute(*SubElement, "sightRadius", 60.0f);
		m_sightAngle = ParseXmlAttribute(*SubElement, "sightAngle", 100.0f);
		std::string damageRange = ParseXmlAttribute(*SubElement, "meleeDamage", "0.0~0.0");
		Strings rangeValues = SplitStringOnDelimiter(damageRange, '~');
		m_meleeDamage = FloatRange(std::stof(rangeValues[0]), std::stof(rangeValues[1]));
		m_meleeDelay = ParseXmlAttribute(*SubElement, "meleeDelay", 2.0f);
		m_meleeRange = ParseXmlAttribute(*SubElement, "meleeRange", 1.0f);

		SubElement = SubElement->NextSiblingElement();
 		SubElement = ParseAppearance(SubElement);

		SubElement = SubElement->NextSiblingElement();
		// Sounds tag
		XmlElement const* loopElement = SubElement->FirstChildElement();
		std::string sound;
//		while (loopElement != nullptr)
		{
			//Sound tag
			sound = ParseXmlAttribute(*loopElement, "sound", "junk");
			m_attackSoundName = ParseXmlAttribute(*loopElement, "name", "junk");
			loopElement = loopElement->NextSiblingElement();
			sound = ParseXmlAttribute(*loopElement, "sound", "junk");
			m_hurtSoundName = ParseXmlAttribute(*loopElement, "name", "junk");
			loopElement = loopElement->NextSiblingElement();
			sound = ParseXmlAttribute(*loopElement, "sound", "junk");
			m_deathSoundName = ParseXmlAttribute(*loopElement, "name", "junk");
			loopElement = loopElement->NextSiblingElement();
		}
	}
	return true;
}

void ActorDefinition::InitializeDefinitions(const char* path)
{
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile(path);
	XmlElement* root = doc.RootElement();
	if (!root)
	{
		ERROR_AND_DIE("ActorDefinitions.xml problem reading file!");
	}

	XmlElement* element = root->FirstChildElement();
	while (element)
	{
		ActorDefinition* pDefinition = new ActorDefinition();
		pDefinition->LoadFromXmlElement(*element);
		ActorDefinition::s_definitions.push_back(pDefinition);
		element = element->NextSiblingElement();
	}
}

void ActorDefinition::ClearDefinitions()
{
	destroy<ActorDefinition>(s_definitions);
}

const ActorDefinition* ActorDefinition::GetByName(const std::string& name)
{
	for (int index = 0; index < static_cast<int>(s_definitions.size()); index++)
	{
		if (s_definitions[index] && s_definitions[index]->m_name == name)
		{
			return s_definitions[index];
		}
	}
	return nullptr;
}
