#include "Engine/Core/ErrorWarningAssert.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "Engine/Core/Image.hpp"
#include "StringUtils.hpp"
#include "Gif.hpp"
#include "EngineCommon.hpp"

Image::Image(char const* imageFilePath)
{
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

									// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &m_dimensions.x, &m_dimensions.y, &bytesPerTexel, numComponentsRequested);

	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));

	Rgba8 texel;
	for (int index = 0; index < m_dimensions.x * m_dimensions.y * bytesPerTexel; index += bytesPerTexel)
	{
		texel.r = texelData[index];
		texel.g = texelData[index + 1];
		texel.b = texelData[index + 2];
		texel.a = bytesPerTexel == 3 ? 255 : texelData[index + 3];
		m_rgbaTexels.push_back(texel);
	}

	stbi_image_free(texelData);
	m_imageFilePath = imageFilePath;
}

Image::Image(IntVec2 size, Rgba8 color, char const* imageFilePath)
{
	m_dimensions = size;
	for (int index = 0; index < size.x * size.y; index++)
	{
		m_rgbaTexels.push_back(color);
	}
	m_imageFilePath = imageFilePath;
}

//---------------------------------------------------------------------------------------------
// constructor for specifically creating a GIF image
Image::Image(char const* imageFilePath, Gif* gif)
{
	UNUSED(gif);
	int x;
	int y;
	int frames = 0;
	int* delays = nullptr;
	unsigned char* result = Image::stbi_load_gif_file(imageFilePath, &x, &y, &frames, &delays);
	UNUSED(result);
}

std::string const& Image::GetImageFilePath() const
{
	return m_imageFilePath;
}

IntVec2 Image::GetDimensions() const
{
	return m_dimensions;
}

const void* Image::GetRawData() const
{
	return (void*)(m_rgbaTexels.data());
}

Rgba8 Image::GetTexelColor(IntVec2 const& texelCoords) const
{
	return m_rgbaTexels[texelCoords.x + m_dimensions.x * texelCoords.y];
}

void Image::SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor)
{
	m_rgbaTexels[texelCoords.x + m_dimensions.x * texelCoords.y] = newColor;
}

//============================================================================================================
// GIF handling functions
unsigned char* stbi_load_gif(stbi__context* s, int* x, int* y, int* frames, int** delays)
{
	int comp;
	unsigned char* result = 0;

	if (stbi__gif_test(s))
	{
		return (unsigned char *)stbi__load_gif_main(s, delays, x, y, frames, &comp, 4);
	}

	stbi__result_info ri;
// no effect	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	result = (unsigned char *)stbi__load_main(s, x, y, &comp, 4, &ri, 8);
	*frames = !!result;

	if (ri.bits_per_channel != 8) 
	{
		STBI_ASSERT(ri.bits_per_channel == 16);
		result = stbi__convert_16_to_8((stbi__uint16*)result, *x, *y, 4);
		ri.bits_per_channel = 8;
	}

	return result;
}

//----------------------------------------------------------------------------------------------------------------
unsigned char* Image::stbi_load_gif_mem(unsigned char* buffer, int len, int* x, int* y, int* frames, int** delays)
{
	stbi__context s;
	stbi__start_mem(&s, buffer, len);
	return stbi_load_gif(&s, x, y, frames, delays);
}

//----------------------------------------------------------------------------------------------------------------
unsigned char* Image::stbi_load_gif_file(char const* filename, int* x, int* y, int* frames, int** delays)
{
	FILE* f;
	stbi__context s;
	unsigned char* result = 0;

	f = stbi__fopen(filename, "rb");
	if (!f)
	{
		return stbi__errpuc("can't fopen", "Unable to open file");
	}

	stbi__start_file(&s, f);
	result = stbi_load_gif(&s, x, y, frames, delays);
	fclose(f);

	return result;
}

