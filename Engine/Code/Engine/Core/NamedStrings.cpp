#include "Engine/Core/NamedStrings.hpp"

NamedStrings::NamedStrings()
{

}

void NamedStrings::PopulateFromXmlElementAttributes(XmlElement const& element)
{
	tinyxml2::XMLAttribute const* attribute = element.FirstAttribute();
	while (attribute)
	{
		m_keyValuePairs[attribute->Name()] = attribute->Value();
		attribute = attribute->Next();
	}
}

void NamedStrings::SetValue(std::string const& keyName, std::string const& newValue)
{
	m_keyValuePairs[keyName] = newValue;
}

std::string NamedStrings::GetValue(std::string const& keyName, std::string const& defaultValue) const
{
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return iter->second;
}

bool NamedStrings::GetValue(std::string const& keyName, bool defaultValue) const
{
	std::string value = GetValue(keyName, defaultValue ? "true" : "false");
	if (!value.compare("True") || !value.compare("true") || !value.compare("TRUE"))
		return true;
	if (!value.compare("False") || !value.compare("false") || !value.compare("FALSE"))
		return false;
	return defaultValue;
}

int NamedStrings::GetValue(std::string const& keyName, int defaultValue) const
{
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return atoi(iter->second.c_str()); // TODO test numeric
}

float NamedStrings::GetValue(std::string const& keyName, float defaultValue) const
{
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return static_cast<float>(atof(iter->second.c_str())); // TODO test numeric
}

std::string NamedStrings::GetValue(std::string const& keyName, char const* defaultValue) const
{
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	return iter->second;
}

Rgba8 NamedStrings::GetValue(std::string const& keyName, Rgba8 const& defaultValue) const
{
	Rgba8 value;
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	value.SetFromText(iter->second.c_str()); // TODO test numeric
	return value;
}

Vec2 NamedStrings::GetValue(std::string const& keyName, Vec2 const& defaultValue) const
{
	Vec2 value;
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	value.SetFromText(iter->second.c_str()); // TODO test numeric
	return value;
}

IntVec2 NamedStrings::GetValue(std::string const& keyName, IntVec2 const& defaultValue) const
{
	IntVec2 value;
	Blackboard::const_iterator iter = m_keyValuePairs.find(keyName);
	if (iter == m_keyValuePairs.end())
	{
		return defaultValue;
	}
	value.SetFromText(iter->second.c_str()); // TODO test numeric
	return value;
}

