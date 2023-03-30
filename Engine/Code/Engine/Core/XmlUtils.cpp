#include "ThirdParty/TinyXML2/tinyxml2.h"
#include "Engine/Core/XmlUtils.hpp"
#include "Rgba8.hpp"
#include <string>
#include <vector>
#include "../Math/EulerAngles.hpp"

int ParseXmlAttribute(XmlElement const& element, char const* attributeName, int defaultValue)
{
	return element.IntAttribute(attributeName, defaultValue);
}

char ParseXmlAttribute(XmlElement const& element, char const* attributeName, char defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		return temp[0];
	}
	return defaultValue;
}

bool ParseXmlAttribute(XmlElement const& element, char const* attributeName, bool defaultValue)
{
	bool temp = defaultValue;
	element.QueryBoolAttribute(attributeName, &temp);
	return temp;
}

float ParseXmlAttribute(XmlElement const& element, char const* attributeName, float defaultValue)
{
	return element.FloatAttribute(attributeName, defaultValue);
}

Rgba8 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Rgba8 const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 3)
		{
			// TODO: test that the strings are numeric and in range
			return Rgba8(static_cast<unsigned char>(atoi(strings[0].c_str())), static_cast<unsigned char>(atoi(strings[1].c_str())), 
				static_cast<unsigned char>(atoi(strings[2].c_str())), 255);
		}
		if (strings.size() == 4)
		{
			// TODO: test that the strings are numeric and in range
			return Rgba8(static_cast<unsigned char>(atoi(strings[0].c_str())), static_cast<unsigned char>(atoi(strings[1].c_str())), 
				static_cast<unsigned char>(atoi(strings[2].c_str())), static_cast<unsigned char>(atoi(strings[3].c_str())));
		}
	}
	return defaultValue;
}

Vec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec2 const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 2)
		{
			// TODO: test that the strings are numeric
			return Vec2(static_cast<float>(atof(strings[0].c_str())), static_cast<float>(atof(strings[1].c_str())));
		}
	}	return defaultValue;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		return std::string(temp);
	}
	return defaultValue;
}

Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues, char delimiter)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, delimiter);
		return strings;
	}
	return defaultValues;
}

std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, char const* defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		return std::string(temp);
	}
	return defaultValue;
}

IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 2)
		{
			// TODO: test that the strings are numeric
			return IntVec2(atoi(strings[0].c_str()), atoi(strings[1].c_str()));
		}
		return defaultValue;
	}
	return defaultValue;
}

IntVec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec3 const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 3)
		{
			// TODO: test that the strings are numeric
			return IntVec3(atoi(strings[0].c_str()), atoi(strings[1].c_str()), atoi(strings[2].c_str()));
		}
		return defaultValue;
	}
	return defaultValue;
}

Vec3 ParseXmlAttribute(XmlElement const& element, char const* attributeName, Vec3 const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 3)
		{
			// TODO: test that the strings are numeric
			return Vec3(static_cast<float>(atof(strings[0].c_str())), static_cast<float>(atof(strings[1].c_str())), static_cast<float>(atof(strings[2].c_str())));
		}
	}	return defaultValue;
}

EulerAngles  ParseXmlAttribute(XmlElement const& element, char const* attributeName, EulerAngles const& defaultValue)
{
	const char* temp = element.Attribute(attributeName);
	if (temp)
	{
		Strings strings = SplitStringOnDelimiter(temp, ',');
		if (strings.size() == 3)
		{
			// TODO: test that the strings are numeric
			return EulerAngles(static_cast<float>(atof(strings[0].c_str())), static_cast<float>(atof(strings[1].c_str())), static_cast<float>(atof(strings[2].c_str())));
		}
	}	return defaultValue;
}

void ParseXmlFile(const char* fileName)
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile(fileName);
}

