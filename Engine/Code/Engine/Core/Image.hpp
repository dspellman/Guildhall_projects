#pragma once
#include "ThirdParty/stb/stb_image.h"
#include <string>
#include <vector>
#include "Engine/Math/IntVec2.hpp"
#include "Engine/Core/Rgba8.hpp"

class Gif;

class Image
{
	friend class Renderer;
public:
	~Image() {};
	Image() = default;
	Image(char const* imageFilePath);
	Image( IntVec2 size, Rgba8 color, char const* imageFilePath );
	Image(char const* imageFilePath, Gif* gif);
	std::string const& GetImageFilePath() const;
	IntVec2	GetDimensions() const;
	const void* GetRawData() const;
	Rgba8	GetTexelColor(IntVec2 const& texelCoords) const;
	void	SetTexelColor(IntVec2 const& texelCoords, Rgba8 const& newColor);

	static unsigned char* stbi_load_gif_mem(unsigned char* buffer, int len, int* x, int* y, int* frames, int** delays);
	static unsigned char *stbi_load_gif_file(char const *filename, int *x, int *y, int *frames, int **delays);

private:
	std::string				m_imageFilePath;
	IntVec2					m_dimensions = IntVec2(0, 0);
	std::vector< Rgba8 >	m_rgbaTexels;
};