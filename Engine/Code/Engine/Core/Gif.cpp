#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Core/Gif.hpp"
#include "StringUtils.hpp"
#include "ThirdParty/stb/stb_image.h"
#include "Image.hpp"
#include "Engine/Renderer/Renderer.hpp"

Gif::~Gif()
{
	if (m_delays)
	{
		delete[] m_delays;
	}
}

Gif::Gif(char const* imageFilePath, Renderer* renderer)
{
	m_renderer = renderer;
	int bytesPerTexel = 4; // This normally would be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int* delays;

	// Load (and decompress) the Gif RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	unsigned char* texelData = Image::stbi_load_gif_file(imageFilePath, &m_dimensions.x, &m_dimensions.y, &m_frames, &delays);
	m_delays = new float[m_frames];
	for (int index = 0; index < m_frames; index++)
	{
		m_delays[index] = float(delays[index]) / 1000.0f;
	}
	delete[] delays;

	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load Gif \"%s\"", imageFilePath));

	// create a texture for each frame
	Rgba8 texel;
	int frameSize = m_dimensions.x * m_dimensions.y * bytesPerTexel;
	for (int index = 0; index <  m_frames; index ++)
	{
		std::string frameName(imageFilePath);
		frameName += std::to_string(index); // create a unique identifier for the frame texture
		Texture* texture = renderer->CreateTextureFromData(frameName.c_str(), m_dimensions, bytesPerTexel, texelData + (index * frameSize));
		UNUSED(texture); // the texture is stored in the renderer (but could reverse for GIFs)
	}

	stbi_image_free(texelData);
	m_imageFilePath = imageFilePath;
}

Gif::Gif(IntVec2 size, int frames, int* delays, Rgba8 color, char const* imageFilePath, Renderer* renderer)
{
	m_renderer = renderer;
	m_dimensions = size;
	m_frames = frames;
	m_delays = new float[m_frames];
	for (int index = 0; index < m_frames; index++)
	{
		m_delays[index] = float(delays[index]) / 1000.0f;
	}
	for (int index = 0; index < size.x * size.y * frames; index++)
	{
//		m_rgbaTexels.push_back(color);
	}
	m_imageFilePath = imageFilePath;
}

void Gif::Play(AABB2 window, int mode, bool wait)
{
	if (wait && m_playing)
	{
		return; // wait for current play to end
	}
	m_window = window;
	m_currentFrame = 0;
	m_frameTime = 0.0f;
	m_playing = true;
	m_mode = mode;
}

void Gif::Stop()
{
	m_playing = false;
	m_currentFrame = 0;
	m_frameTime = 0.0f;
}

void Gif::TogglePause()
{
	m_paused = !m_paused;
}

void Gif::Update(float deltaSeconds)
{
	// could handle pause by holding current frame when paused
	if (!m_playing || m_paused)
	{
		return; // do nothing if not playing or paused
	}
	m_frameTime += deltaSeconds;
	if (m_frameTime > m_delays[m_currentFrame])
	{
		m_frameTime -= m_delays[m_currentFrame];
		m_currentFrame++;
		if (m_currentFrame == m_frames)
		{
			switch (m_mode)
			{
			case 0: // once and done
				m_playing = false;
				m_currentFrame = 0;
				m_frameTime = 0.0f;
				break;
			case 1: // once and hold
				m_currentFrame--;
				break;
			case 2: // looping
				m_currentFrame = 0;
				break;
			}
		}
	}
}

void Gif::Render() const
{
	if (!m_playing || !m_renderer)
	{
		return;
	}
	std::vector<Vertex_PCU> GifVerts;
	std::string frameName(m_imageFilePath);
	frameName += std::to_string(m_currentFrame); // create a unique identifier for the frame texture
	m_renderer->BindTexture(m_renderer->CreateOrGetTextureFromFile(frameName.c_str()));
	m_renderer->SetBlendMode(BlendMode::ADDITIVE);
	AddVertsForAABB2D(GifVerts, m_window, Rgba8::WHITE, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f));
	m_renderer->DrawVertexArray(int(GifVerts.size()), GifVerts.data());
}

std::string const& Gif::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Gif::GetDimensions() const
{
	return m_dimensions;
}

int Gif::GetFrameCount() const
{
	return m_frames;
}

float Gif::GetDelayForFrame(int index) const
{
	if (index < 0 || index >= m_frames)
	{
		return m_delays[0]; // error check with safe return since delays are generally uniform
	}
	return m_delays[index];
}

bool Gif::IsPlaying()
{
	return m_playing;
}

bool Gif::IsPaused()
{
	return m_paused;
}

