#pragma once
#include "Game/GameCommon.hpp"
#include "Game/BlockDefinition.hpp"

class Block
{
public:
//	~Block();
	Block();

private:
	uint8_t m_definition = AIR;
	uint8_t m_lighting = 0; // outdoor bits 4-7, indoor bits 0-3
	uint8_t m_flags = 0;

public:
	uint8_t GetBlockDefinition() const;
	uint8_t GetLightEmitted() const;
	uint8_t GetIndoorLight() const;
	uint8_t GetOutdoorLight() const;
	void SetBlockDefinition(uint8_t definition);
	void SetIndoorLight(uint8_t indoor);
	void SetOutdoorLight(uint8_t outdoor);
	bool IsSky() const;
	void SetSky(bool state);
	bool IsLightDirty() const;
	void SetLightDirty(bool state);
	bool IsOpaque() const;
	void SetOpaque(bool state);
	bool IsSolid() const;
	void SetSolid(bool state);
	bool IsVisible() const;
	void SetVisible(bool state);
	void InitializeFlags();
};