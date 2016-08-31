#pragma once

#include "D3D11Includes.h"

namespace graphics
{
	struct Mesh
	{
		VertexBuffer* m_pVertexBuffer;
		IndexBuffer* m_pIndexBuffer;
		UINT m_uiVerticesCount;
		UINT m_uiVertexStride;
		UINT m_uiIndecesCount;
	};
}