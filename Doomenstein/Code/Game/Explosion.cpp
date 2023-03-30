#include "Engine/Renderer/SpriteSheet.hpp"
#include "Explosion.hpp"
#include "Engine/Renderer/SpriteAnimDefinition.hpp"
#include "Game.hpp"

Explosion::~Explosion()
{
	delete m_explosionInstance;
	m_explosionInstance = nullptr;
}

Explosion::Explosion( Vec2 position, float orientationDegrees )
	: Entity(position, orientationDegrees)
{
}

void Explosion::RunAnimation(SpriteSheet const& spriteSheet, float scale, float durationSeconds, SpriteAnimPlaybackType playbackType /*= SpriteAnimPlaybackType::ONCE*/)
{
	m_spriteSheet = &spriteSheet;
	m_scalingFactor = scale;
	m_startTime = GetCurrentTimeSeconds();
	m_durationSeconds = durationSeconds;
	m_playbackType = playbackType;
	m_explosionInstance = new SpriteAnimDefinition(spriteSheet, 0, 24, durationSeconds, playbackType);
}

void Explosion::StopAnimation()
{
	Die();
}

void Explosion::Update(float deltaSeconds)
{
	UNUSED(deltaSeconds);
	m_sprite = &m_explosionInstance->GetSpriteDefAtTime(static_cast<float>(GetCurrentTimeSeconds() - m_startTime));

	double elapsedTime = GetCurrentTimeSeconds() - m_startTime;
	if ((m_playbackType == SpriteAnimPlaybackType::ONCE) && (m_durationSeconds < static_cast<float>(elapsedTime)))
	{
		Die(); // garbage collect sprite animations that are no longer playing
	}
}

void Explosion::Render() const
{
	if (m_sprite == nullptr)
	{
		return; // we have not updated the first time
	}
	std::vector<Vertex_PCU> ExplosionVerts;
	AABB2 unitBox = AABB2::UNIT_BOX;
//	float rotation = random.RollRandomFloatInRange(0.0f, 360.0f); // assumes random rotation for now

	g_theRenderer->BindTexture(&m_spriteSheet->GetTexture());
	g_theRenderer->SetBlendMode(BlendMode::ADDITIVE);
	AddVertsForAABB2D(ExplosionVerts, unitBox, Rgba8::WHITE, m_sprite->GetUVs().m_mins, m_sprite->GetUVs().m_maxs);
	TransformVertexArrayXY3D(static_cast<int>(ExplosionVerts.size()), ExplosionVerts.data(), m_scalingFactor, m_orientationDegrees, m_position);
	g_theRenderer->DrawVertexArray(static_cast<int>(ExplosionVerts.size()), ExplosionVerts.data());
}

