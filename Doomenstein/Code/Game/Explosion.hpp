#pragma once
#include "Game/GameCommon.hpp"
#include "Engine/Renderer/SpriteSheet.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Entity.hpp"

class Explosion : public Entity
{
public:
	SpriteSheet const* m_spriteSheet;
	SpriteAnimDefinition* m_explosionInstance = nullptr;
	double m_startTime = 0.0;
	SpriteDefinition const* m_sprite = nullptr;

	~Explosion();
	Explosion(Vec2 position, float orientation);
	void RunAnimation(SpriteSheet const& spriteSheet, float scale, float durationSeconds, SpriteAnimPlaybackType playbackType = SpriteAnimPlaybackType::ONCE);
	void StopAnimation();
	void Update(float deltaSeconds);
	void Render() const;

private:
	float m_durationSeconds;
	SpriteAnimPlaybackType m_playbackType = SpriteAnimPlaybackType::ONCE;
};