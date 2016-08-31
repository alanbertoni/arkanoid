#pragma once

#include "../utilities/Math.h"
#include "../utilities/Types.h"

namespace graphics
{
	enum class EMeshShape
	{
		QUAD,
		FULLSCREENQUAD,
		QUAD2W1H,
	};

	enum class EVertexType
	{
		POSITION_TEXCOORD
	};

	struct Vertex_UnlitTextured
	{
		Vec3 m_vPos;
		Vec2 m_vUV;
	};

	struct MeshData
	{
		std::vector<Vertex_UnlitTextured> m_oVertices;
		std::vector<UINT16> m_oIndices;
		UINT m_uiVertexStride;
		UINT m_uiVerticesCount;
		UINT m_uiIndecesCount;
		MeshId m_uiId;
	};
}