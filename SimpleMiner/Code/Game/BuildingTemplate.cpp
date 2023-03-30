#include "Game/BuildingTemplate.hpp"
#include <string>
#include "Engine/Core/FileUtils.hpp"
#include "Chunk.hpp"

std::vector<BuildingTemplate*> BuildingTemplate::s_buildingTemplates;

BuildingTemplate::~BuildingTemplate()
{

}

BuildingTemplate::BuildingTemplate(const char* filename)
{
	std::string outString;
	FileReadToString(outString, filename);
	Strings rawBlocks = SplitStringOnDelimiter(outString, '[');
	Strings nameField = SplitStringOnDelimiter(rawBlocks[0], ':');
	// save the name of the building
	int pos = 1;
	while (nameField[1][pos] != '\"')
	{
		m_name.push_back(nameField[1][pos++]);
	}
	// read the block information and determine footprint size
	for (int index = 3; index < int(rawBlocks.size()); index++)
	{
		// convert the coordinates to our frame of reference
		Strings rawValues = SplitStringOnDelimiter(rawBlocks[index], ',');
		BlockPosition block;
		block.position.x = std::stoi(rawValues[0]);
		if (abs(block.position.x) > m_xoffset)
		{
			m_xoffset = abs(block.position.x);
		}
		block.position.z = std::stoi(rawValues[1]) + 7;
		block.position.y = -std::stoi(rawValues[2]);
		if (abs(block.position.y) > m_yoffset)
		{
			m_yoffset = abs(block.position.y);
		}
		rawValues[3][0] = '0';
		rawValues[3][1] = 'x';
		// convert the color code to a block type
		int colorCode = std::stoi(rawValues[3],nullptr, 16);
		switch (colorCode)
		{
		case 0x00FF0000:
			block.type = BuildingBlock::WALL;
			break;
		case 0x00FFFF00:
			block.type = BuildingBlock::BEAM;
			break;
		case 0x00FFFFFF:
			block.type = BuildingBlock::FLOOR;
			break;
		case 0x00FF00FF:
			block.type = BuildingBlock::ROOF;
			break;
		case 0x00FF8000:
			block.type = BuildingBlock::LIGHT;
			break;
		case 0x0000FFFF:
			block.type = BuildingBlock::BRICK;
			break;
		case 0x000000FF:
			block.type = BuildingBlock::STONE;
			break;
		case 0x00A52A2A:
			block.type = BuildingBlock::LOG;
			break;
		case 0x0000FF00:
			block.type = BuildingBlock::LEAVES;
			break;
		case 0x00FF80FF:
			block.type = BuildingBlock::WATER;
			break;
		default:
			block.type = BuildingBlock::EMPTY;
			break;
		}
		// save the block information
		m_blocks.push_back(block);
	}
	// add one to the offsets for clearance around the building
	 m_xoffset++;
	 m_yoffset++;
}

void BuildingTemplate::CopyBuildingToChunk(Chunk* chunk, int terrainHeight, int dx, int dy, int rotation)
{
	int temp;
	for (int index = 0; index < (int)m_blocks.size(); index++)
	{
		terrainHeight; // move up to base position
		IntVec3 position = m_blocks[index].position;
		switch (rotation)
		{
		case 0:
			// unchanged
			break;
		case 1:
			position.x *= -1;
			break;
		case 2:
			temp = position.y;
			position.y = position.x;
			position.x = temp;
 			position.x *= -1;
//			position.y *= -1;
			break;
		case 3:
			temp = position.y;
			position.y = position.x;
			position.x = temp;
			break;
		}
		position.x += dx;
		position.y += dy;
		position.z += terrainHeight;
		if (position.x < 0 || position.x > MASK_X)
		{
			continue;
		}
		if (position.y < 0 || position.y > MASK_Y)
		{
			continue;
		}
		if (position.z < 0 || position.z > MASK_Z)
		{
			continue;
		}

		uint8_t block = chunk->ConvertToBlock(m_blocks[index].type);
		chunk->SetBlock(position, block);
	}
}

void BuildingTemplate::Initialize(const char* source)
{
	// parse configuration file
	tinyxml2::XMLDocument doc;
	doc.LoadFile(source);
	XmlElement* element = doc.RootElement();

	NamedStrings templateNames;
	while (element)
	{
		tinyxml2::XMLAttribute const* attribute = element->FirstAttribute();
		while (attribute)
		{
			BuildingTemplate* building = new BuildingTemplate(attribute->Value());
			s_buildingTemplates.push_back(building);
			attribute = attribute->Next();
		}
		element = element->NextSiblingElement();
	}

}

void BuildingTemplate::Destroy()
{
	for (BuildingTemplate* obj : s_buildingTemplates)
	{
		delete obj;
	}
}

BuildingTemplate* BuildingTemplate::GetByName(const std::string& name)
{
	for (BuildingTemplate* building : s_buildingTemplates)
	{
		if (building->m_name == name)
		{
			return building;
		}
	}
	return nullptr;
}
