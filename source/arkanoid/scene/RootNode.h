#pragma once

#include "SceneNode.h"

namespace scene
{
	class RootNode : public SceneNode
	{
	public:
		RootNode(actor::Actor* pActor, ERenderPass eRenderPass = ERenderPass::RenderPass_Static);
		virtual ~RootNode();

		virtual bool Render(Scene& oScene) override;
		virtual bool RenderChildren(Scene& oScene) override;
		virtual bool AddChild(SceneNode* pChildNode) override;
		virtual bool RemoveChild(SceneNode* pChildNode) override;

	protected:
		RootNode(const RootNode&) = delete;
		RootNode(RootNode&&) = delete;
		RootNode& operator=(const RootNode&) = delete;
		RootNode& operator=(RootNode&&) = delete;

		virtual bool IsVisible(Scene& oScene) const override;
	};

	inline bool RootNode::IsVisible(Scene& oScene) const
	{
		return true;
	}
}