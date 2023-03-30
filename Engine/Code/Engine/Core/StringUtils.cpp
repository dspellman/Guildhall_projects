#include "Engine/Core/StringUtils.hpp"
#include <stdarg.h>

//-----------------------------------------------------------------------------------------------
constexpr int STRINGF_STACK_LOCAL_TEMP_LENGTH = 2048;


//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... )
{
	char textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, STRINGF_STACK_LOCAL_TEMP_LENGTH, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ STRINGF_STACK_LOCAL_TEMP_LENGTH - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	return std::string( textLiteral );
}


//-----------------------------------------------------------------------------------------------
const std::string Stringf( int maxLength, char const* format, ... )
{
	char textLiteralSmall[ STRINGF_STACK_LOCAL_TEMP_LENGTH ];
	char* textLiteral = textLiteralSmall;
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		textLiteral = new char[ maxLength ];

	va_list variableArgumentList;
	va_start( variableArgumentList, format );
	vsnprintf_s( textLiteral, maxLength, _TRUNCATE, format, variableArgumentList );	
	va_end( variableArgumentList );
	textLiteral[ maxLength - 1 ] = '\0'; // In case vsnprintf overran (doesn't auto-terminate)

	std::string returnValue( textLiteral );
	if( maxLength > STRINGF_STACK_LOCAL_TEMP_LENGTH )
		delete[] textLiteral;

	return returnValue;
}

Strings SplitStringOnDelimiter(const std::string& originalString, char delimiterToSplitOn)
{
	Strings strings;
	std::string token;
	int index = 0;

	while (index < originalString.size())
	{
		if (originalString[index] == delimiterToSplitOn)
		{
			strings.push_back(token);
			token.clear();
		}
		else
		{
			token.push_back(originalString[index]);
		}
		index++;
	}
	strings.push_back(token); // push last instance
	return strings;
}

Strings SplitLinesOnCRLFCombo(const std::string& originalString)
{
	Strings strings;
	std::string token;
	int index = 0;

	while (index < originalString.size())
	{
		if (originalString[index] == 10) // break on LF
		{
			strings.push_back(token);
			token.clear();
		}
		else
		{
			if (originalString[index] != 13) // ignore CR
			{
				token.push_back(originalString[index]);
			}
		}
		index++;
	}
	strings.push_back(token); // push last instance
	return strings;
}
// IntVec2 ParseXmlAttribute(XmlElement const& element, char const* attributeName, IntVec2 const& defaultValue)
// {
// 
// }
// 
// std::string ParseXmlAttribute(XmlElement const& element, char const* attributeName, std::string const& defaultValue)
// {
// 
// }
// 
// Strings ParseXmlAttribute(XmlElement const& element, char const* attributeName, Strings const& defaultValues)
// {
// 
// }
