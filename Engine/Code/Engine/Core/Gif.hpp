#pragma once
//#include "ThirdParty/stb/stb_image.h"
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Math/AABB2.hpp"

class Texture;

class Gif
{
	friend class Renderer;
	friend class Image;
public:
	~Gif();
	Gif() = default;
	Gif(char const* imageFilePath, Renderer* renderer);
	Gif( IntVec2 size, int frames, int* delays, Rgba8 color, char const* imageFilePath, Renderer* renderer );
	void Play(AABB2 window, int mode, bool wait = false);
	void Stop();
	void TogglePause();
	void Update(float deltaSeconds);
	void Render() const;
	std::string const& GetImageFilePath() const;
	IntVec2	GetDimensions() const;
	int GetFrameCount() const;
	float GetDelayForFrame(int index) const;
	bool IsPlaying();
	bool IsPaused();

private:
	std::string				m_imageFilePath;
	IntVec2					m_dimensions = IntVec2(0, 0);
	int						m_frames = 0;
	float*					m_delays = nullptr;
//	std::vector< Texture* >	m_frameTextures;
	AABB2 m_window = AABB2::ZERO_TO_ONE;
	int m_currentFrame = 0;
	float m_frameTime = 0.0f;
	bool m_playing = false;
	bool m_paused = false;
	int m_mode = 0; // 0 = once and done, 1 = once and hold, 2 = looping
	Renderer* m_renderer = nullptr;
};