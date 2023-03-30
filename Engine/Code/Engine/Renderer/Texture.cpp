#include "Engine\Renderer\Texture.hpp"
#include "Engine\Renderer\Renderer.hpp"

Texture::Texture(IntVec2 const dimensions, std::string const imageFilePath)
	: m_dimensions(dimensions), m_name(imageFilePath)
{

}

Texture::~Texture()
{
	DX_SAFE_RELEASE(m_texture);
	DX_SAFE_RELEASE(m_shaderResourceView);
}
