#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

//struct IntVec2;
typedef std::vector< std::string > Strings;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );

Strings SplitStringOnDelimiter(const std::string& originalString, char delimiterToSplitOn);
Strings SplitLinesOnCRLFCombo(const std::string& originalString);
// IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue);
// std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue);
// Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues);



