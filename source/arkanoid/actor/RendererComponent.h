#pragma once

#include "ActorComponent.h"
#include "../graphics/Geometry.h"

namespace graphics
{
	class Material;
}

namespace scene
{
	class Scene;
	class SceneNode;
}

namespace actor
{
	class RendererComponent : public ActorComponent
	{
	public:
		RendererComponent(Actor* pOwner);
		~RendererComponent();

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		const Color& GetDiffuseColor() const;
		void SetDiffuseColor(const Color& oColor);
		const Vec4& GetTextureScaleAndOffset() const;
		const std::string& GetTextureFilename() const;

		void InitForRendering(scene::Scene& oScene, const scene::SceneNode& oSceneNode);
		void Render(scene::Scene& oScene, scene::SceneNode& oSceneNode);
		void ReleaseRenderingResources(const scene::Scene& oScene, const scene::SceneNode& oSceneNode);

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		RendererComponent(const RendererComponent&) = delete;
		RendererComponent(RendererComponent&&) = delete;
		RendererComponent& operator=(const RendererComponent&) = delete;
		RendererComponent& operator=(RendererComponent&&) = delete;

		bool InitMeshInfo(const tinyxml2::XMLElement* pData);
		bool InitMaterialInfo(const tinyxml2::XMLElement* pData);
		bool InitColorInfo(const tinyxml2::XMLElement* pData);
		bool InitTextureInfo(const tinyxml2::XMLElement* pData);

		void SetDiffuseColorFromScript(LuaPlus::LuaObject oColorLuaObject);
		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		Color m_oDiffuseColor;
		Vec4 m_vTextureScaleAndOffset;
		std::string m_szTextureFile;
		graphics::Material* m_pMaterial;
		graphics::EMeshShape m_eMeshShape;
		MeshId m_uiMeshId;
	};

	inline std::string RendererComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject RendererComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline const Color& RendererComponent::GetDiffuseColor() const
	{
		return m_oDiffuseColor;
	}

	inline const Vec4& RendererComponent::GetTextureScaleAndOffset() const
	{
		return m_vTextureScaleAndOffset;
	}

	inline const std::string& RendererComponent::GetTextureFilename() const
	{
		return m_szTextureFile;
	}
}