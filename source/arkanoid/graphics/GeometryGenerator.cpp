
#include "GeometryGenerator.h"
#include "../utilities/String.h"

void graphics::GeometryGenerator::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		new GeometryGenerator{};
	}
}

graphics::GeometryGenerator::GeometryGenerator()
	: Singleton{}
	, m_oMeshDataMap{}
{
}

graphics::MeshData& graphics::GeometryGenerator::CreateScreenQuad(float fWidth, float fHeight, MeshId oId)
{
	MeshData l_oMeshData;

	Vertex_UnlitTextured l_aoVerts[4];

	float l_fW2 = 0.5f * fWidth;
	float l_fH2 = 0.5f * fHeight;

	l_aoVerts[0] = Vertex_UnlitTextured{ Vec3{ -l_fW2, -l_fH2, 0.0f }, Vec2{ 0.0f , 1.0f } };
	l_aoVerts[1] = Vertex_UnlitTextured{ Vec3{ -l_fW2, +l_fH2, 0.0f }, Vec2{ 0.0f , 0.0f } };
	l_aoVerts[2] = Vertex_UnlitTextured{ Vec3{ +l_fW2, +l_fH2, 0.0f }, Vec2{ 1.0f , 0.0f } };
	l_aoVerts[3] = Vertex_UnlitTextured{ Vec3{ +l_fW2, -l_fH2, 0.0f }, Vec2{ 1.0f , 1.0f } };

	l_oMeshData.m_oVertices.assign(&l_aoVerts[0], &l_aoVerts[4]);
	l_oMeshData.m_uiVertexStride = sizeof(Vertex_UnlitTextured);
	l_oMeshData.m_uiVerticesCount = l_oMeshData.m_oVertices.size();

	UINT16 l_auiIndices[6];

	l_auiIndices[0] = 0; l_auiIndices[1] = 1; l_auiIndices[2] = 2;
	l_auiIndices[3] = 0; l_auiIndices[4] = 2; l_auiIndices[5] = 3;

	l_oMeshData.m_oIndices.assign(&l_auiIndices[0], &l_auiIndices[6]);
	l_oMeshData.m_uiIndecesCount = l_oMeshData.m_oIndices.size();

	l_oMeshData.m_uiId = oId;

	m_oMeshDataMap.emplace(l_oMeshData.m_uiId, std::make_pair(1, l_oMeshData));

	MeshDataMap::iterator l_oIt = m_oMeshDataMap.find(l_oMeshData.m_uiId);
	return l_oIt->second.second;
}

graphics::MeshData& graphics::GeometryGenerator::CreateMeshData(EMeshShape eShape)
{
	switch (eShape)
	{
	case graphics::EMeshShape::FULLSCREENQUAD:
	{
		MeshId l_oId = utilities::HashString("FULLSCREENQUAD");
		MeshDataMap::iterator l_oIt = m_oMeshDataMap.find(l_oId);
		if (l_oIt == m_oMeshDataMap.end())
		{
			return CreateScreenQuad(2.0f, 2.0f, l_oId);
		}
		else
		{
			++(l_oIt->second.first);
			return l_oIt->second.second;
		}
	}
	case graphics::EMeshShape::QUAD2W1H:
	{
		MeshId l_oId = utilities::HashString("QUAD2W1H");
		MeshDataMap::iterator l_oIt = m_oMeshDataMap.find(l_oId);
		if (l_oIt == m_oMeshDataMap.end())
		{
			return CreateScreenQuad(1.0f, 0.5f, l_oId);
		}
		else
		{
			++(l_oIt->second.first);
			return l_oIt->second.second;
		}
	}
	case graphics::EMeshShape::QUAD:
	{
		MeshId l_oId = utilities::HashString("QUAD");
		MeshDataMap::iterator l_oIt = m_oMeshDataMap.find(l_oId);
		if (l_oIt == m_oMeshDataMap.end())
		{
			return CreateScreenQuad(1.0f, 1.0f, l_oId);
		}
		else
		{
			++(l_oIt->second.first);
			return l_oIt->second.second;
		}
	}
	default:
	{
		LOG_ERROR("Unknown MeshShape type");
		assert(false);
		return *(new MeshData{});
	}
	}
}

void graphics::GeometryGenerator::DestroyMeshData(MeshId oId)
{
	auto l_oIt = m_oMeshDataMap.find(oId);
	assert(l_oIt != m_oMeshDataMap.end());
	auto& l_oPair = l_oIt->second;
	if (--l_oPair.first < 1)
	{
		m_oMeshDataMap.erase(l_oIt);
	}
}
