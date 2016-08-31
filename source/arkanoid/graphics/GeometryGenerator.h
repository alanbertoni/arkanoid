#pragma once

#include "Geometry.h"
#include "../core/Common.h"
#include "../core/Singleton.h"

namespace graphics
{
	class GeometryGenerator : public core::Singleton<GeometryGenerator>
	{
	public:
		static void CreateInstance();

		~GeometryGenerator() = default;

		MeshData& CreateMeshData(EMeshShape eShape);
		void DestroyMeshData(MeshId oId);

	private:
		GeometryGenerator();
		GeometryGenerator(const GeometryGenerator&) = delete;
		GeometryGenerator& operator=(const GeometryGenerator&) = delete;
		GeometryGenerator(GeometryGenerator&&) = delete;
		GeometryGenerator& operator=(GeometryGenerator&&) = delete;

		MeshData& CreateScreenQuad(float fWidth, float fHeight, MeshId oId);

		typedef std::map<MeshId, std::pair<size_t, MeshData>> MeshDataMap;
		MeshDataMap m_oMeshDataMap;
	};
}