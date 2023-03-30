#pragma once
#include <d3d11.h>

class ConstantBuffer
{
	friend class Renderer;

public:
	ConstantBuffer(size_t size);
	ConstantBuffer(const ConstantBuffer& copy) = delete;
	virtual ~ConstantBuffer();

	ID3D11Buffer* m_buffer = nullptr;
	size_t m_size = 0;
};
