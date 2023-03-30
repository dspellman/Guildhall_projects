#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "d3d11.h"

VertexBuffer::VertexBuffer(size_t size, unsigned int stride)
{
	UNUSED(stride); // how should this be used QUESTION
	m_size = size;
	m_stride = stride;
	// assumes buffer created externally and m_buffer set directly?
}

VertexBuffer::~VertexBuffer()
{
	DX_SAFE_RELEASE(m_buffer);
}

unsigned int VertexBuffer::GetStride() const
{
	return m_stride;
}

