#include "Game/Block.hpp"

Block::Block()
{
	m_lighting = 0;
}

uint8_t Block::GetBlockDefinition() const
{
	return m_definition;
}

uint8_t Block::GetLightEmitted() const
{
	return (uint8_t)BlockDefinition::s_definitions[m_definition].m_light;
}

uint8_t Block::GetIndoorLight() const
{
	return m_lighting & 0x0F;
}

uint8_t Block::GetOutdoorLight() const
{
	return (m_lighting >> 4) & 0x0F;
}

void Block::SetBlockDefinition(uint8_t definition)
{
	m_definition = definition;
}

void Block::SetIndoorLight(uint8_t indoor)
{
	m_lighting &= 0xF0;
	m_lighting |= (indoor & 0x0F);
}

void Block::SetOutdoorLight(uint8_t outdoor)
{
	m_lighting &= 0x0F;
	m_lighting |= ((outdoor & 0x0F) << 4);
}

bool Block::IsSky() const
{
	return m_flags & (0x01 << BLOCK_BIT_IS_SKY);
}

void Block::SetSky(bool state)
{
	if (state)
		m_flags |= (0x01 << BLOCK_BIT_IS_SKY);
	else
		m_flags &= ~(0x01 << BLOCK_BIT_IS_SKY);
}

bool Block::IsLightDirty() const
{
	return m_flags & (0x01 << BLOCK_BIT_IS_LIGHT_DIRTY);
}

void Block::SetLightDirty(bool state)
{
	if (state)
		m_flags |= (0x01 << BLOCK_BIT_IS_LIGHT_DIRTY);
	else
		m_flags &= ~(0x01 << BLOCK_BIT_IS_LIGHT_DIRTY);
}

bool Block::IsOpaque() const
{
	return m_flags & (0x01 << BLOCK_BIT_IS_FULL_OPAQUE);
}

void Block::SetOpaque(bool state)
{
	if (state)
		m_flags |= (0x01 << BLOCK_BIT_IS_FULL_OPAQUE);
	else
		m_flags &= ~(0x01 << BLOCK_BIT_IS_FULL_OPAQUE);
}

bool Block::IsSolid() const
{
	return m_flags & (0x01 << BLOCK_BIT_IS_SOLID);
}

void Block::SetSolid(bool state)
{
	if (state)
		m_flags |= (0x01 << BLOCK_BIT_IS_SOLID);
	else
		m_flags &= ~(0x01 << BLOCK_BIT_IS_SOLID);
}

bool Block::IsVisible() const
{
	return m_flags & (0x01 << BLOCK_BIT_IS_VISIBLE);
}

void Block::SetVisible(bool state)
{
	if (state)
		m_flags |= (0x01 << BLOCK_BIT_IS_VISIBLE);
	else
		m_flags &= ~(0x01 << BLOCK_BIT_IS_VISIBLE);
}

void Block::InitializeFlags()
{
	SetSolid(BlockDefinition::s_definitions[m_definition].m_solid);
	SetVisible(BlockDefinition::s_definitions[m_definition].m_visible);
	SetOpaque(BlockDefinition::s_definitions[m_definition].m_opaque);
	SetLightDirty(false);
	SetSky(false);
}
