#pragma once

#include "../actor/Actor.h"
#include "../utilities/Types.h"

namespace actor
{
	class TransformComponent;
}

namespace scene
{
	class Scene;
}

namespace scene
{
	class SceneNode
	{
	public:
		SceneNode(actor::Actor* pActor, ERenderPass eRenderPass = ERenderPass::RenderPass_Static);
		virtual ~SceneNode() = default;

		virtual bool Init(Scene& oScene);
		virtual bool Render(Scene& oScene);
		virtual bool RenderChildren(Scene& oScene);
		virtual bool AddChild(SceneNode* pChildNode);
		virtual bool RemoveChild(SceneNode* pChildNode);
		virtual void Destroy(const Scene& oScene);

		ERenderPass GetRenderPass() const;
		actor::Actor* GetActor();
		actor::TransformComponent* GetActorsTransformComponent();
		SceneNode* GetParent();

	protected:
		SceneNode(const SceneNode&) = delete;
		SceneNode(SceneNode&&) = delete;
		SceneNode& operator=(const SceneNode&) = delete;
		SceneNode& operator=(SceneNode&&) = delete;

		virtual bool IsVisible(Scene& oScene) const;
		virtual bool PreRender(Scene& oScene);
		virtual bool PostRender(Scene& oScene);
		
		typedef std::vector<SceneNode*> SceneNodeList;
		SceneNodeList m_oChildren;
	
	private:
		SceneNode* m_pParent;
		actor::Actor* m_pActor;
		actor::TransformComponent* m_pTransformComponent;
		ERenderPass m_eRenderPass;
	};

	inline bool SceneNode::IsVisible(Scene& oScene) const
	{
		if (m_pActor)
		{
			return m_pActor->IsEnable();
		}

		return true;
	}

	inline ERenderPass SceneNode::GetRenderPass() const
	{
		return m_eRenderPass;
	}

	inline actor::Actor* SceneNode::GetActor()
	{
		return m_pActor;
	}

	inline actor::TransformComponent* SceneNode::GetActorsTransformComponent()
	{
		return m_pTransformComponent;
	}

	inline SceneNode* SceneNode::GetParent()
	{
		return m_pParent;
	}
}