#include "Engine/Renderer/ConstantBuffer.hpp"
#include "Renderer.hpp"

ConstantBuffer::ConstantBuffer(size_t size)
{
	m_size = size;
	// assumes buffer created externally and m_buffer set directly?
}

ConstantBuffer::~ConstantBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}
