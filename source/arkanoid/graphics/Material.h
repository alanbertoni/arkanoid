#pragma once

namespace actor
{
	class RendererComponent;
}

namespace graphics
{
	class IRenderer;
}

namespace scene
{
	class Scene;
	class SceneNode;
}

namespace graphics
{
	class Material
	{
	public:
		Material() = default;
		virtual ~Material() = default;
		Material(const Material&) = default;
		Material(Material&&) = default;
		Material& operator=(const Material&) = default;
		Material& operator=(Material&&) = default;
		virtual void SetupForRendering(IRenderer& oRenderer, const actor::RendererComponent& oRendererComponent) = 0;
		virtual void UpdateMaterial(const scene::Scene& oScene, const scene::SceneNode& oSceneNode, const actor::RendererComponent& oRendererComponent) = 0;
		virtual void BindMaterial(IRenderer& oRenderer) = 0;
		virtual void ReleaseMaterial(IRenderer& oRenderer) = 0;
	};
}
