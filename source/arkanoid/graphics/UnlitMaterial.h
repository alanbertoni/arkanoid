#pragma once

#include "Material.h"
#include "../utilities/Math.h"
#include "../utilities/Types.h"

namespace graphics
{
	class UnlitMaterial : public Material
	{
	public:
		UnlitMaterial();
		virtual ~UnlitMaterial() = default;
		UnlitMaterial(const UnlitMaterial&) = default;
		UnlitMaterial(UnlitMaterial&&) = default;
		UnlitMaterial& operator=(const UnlitMaterial&) = default;
		UnlitMaterial& operator=(UnlitMaterial&&) = default;

		virtual void SetupForRendering(IRenderer& oRenderer, const actor::RendererComponent& oRendererComponent) override;
		virtual void UpdateMaterial(const scene::Scene& oScene, const scene::SceneNode& oSceneNode, const actor::RendererComponent& oRendererComponent) override;
		virtual void BindMaterial(IRenderer& oRenderer) override;
		virtual void ReleaseMaterial(IRenderer& oRenderer) override;

	private:
		static const char* s_szShaderFilePath;

		void CreateConstantBuffers(IRenderer& oRenderer);

		ShaderId m_oVertexShaderId;
		ShaderId m_oPixelShaderId;
		std::vector<CBufferId> m_oVSConstantBuffers;
		std::vector<CBufferId> m_oPSConstantBuffers;
		TextureId m_oTextureId;
	};
}