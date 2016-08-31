
#include "RootNode.h"
#include "../log/LogManager.h"

scene::RootNode::RootNode(actor::Actor* pActor, ERenderPass eRenderPass)
	: SceneNode{ nullptr, eRenderPass }
{
	m_oChildren.reserve(RenderPass_Last);

	m_oChildren.push_back(new SceneNode{ nullptr, ERenderPass::RenderPass_Static });
	m_oChildren.push_back(new SceneNode{ nullptr, ERenderPass::RenderPass_Actor });
	m_oChildren.push_back(new SceneNode{ nullptr, ERenderPass::RenderPass_Background });
	m_oChildren.push_back(new SceneNode{ nullptr, ERenderPass::RenderPass_NotRendered });
}

scene::RootNode::~RootNode()
{
	SceneNodeList::iterator l_oIt = m_oChildren.begin();
	while (l_oIt != m_oChildren.end())
	{
		delete (*l_oIt);
		++l_oIt;
	}
}

bool scene::RootNode::Render(Scene& oScene)
{
	return RenderChildren(oScene);
}

bool scene::RootNode::RenderChildren(Scene& oScene)
{
	unsigned int l_uiPass = static_cast<unsigned int>(ERenderPass::RenderPass_Static);
	while (l_uiPass < static_cast<unsigned int>(ERenderPass::RenderPass_Last))
	{
		switch (static_cast<ERenderPass>(l_uiPass))
		{
		case ERenderPass::RenderPass_Static:
		case ERenderPass::RenderPass_Actor:
		{
			m_oChildren[l_uiPass]->RenderChildren(oScene);
			break;
		}
		case ERenderPass::RenderPass_Background:
		{
			m_oChildren[l_uiPass]->RenderChildren(oScene);
			break;
		}
		}

		++l_uiPass;
	}

	return true;
}

bool scene::RootNode::AddChild(SceneNode* pChildNode)
{
	ERenderPass l_eRenderPass = pChildNode->GetRenderPass();
	unsigned int l_uiPass = static_cast<unsigned int>(l_eRenderPass);

	if (l_uiPass >= m_oChildren.size() || !m_oChildren[l_uiPass])
	{
		assert(pChildNode->GetActor());
		std::stringstream l_oSs;
		l_oSs << "Error while trying to add child node with render pass " << l_eRenderPass;
		l_oSs << " and Actor name " << pChildNode->GetActor()->GetName();
		l_oSs << " to RootNode";
		LOG_ERROR(l_oSs.str());
		return false;
	}

	return m_oChildren[l_uiPass]->AddChild(pChildNode);
}

bool scene::RootNode::RemoveChild(SceneNode* pChildNode)
{
	ERenderPass l_eRenderPass = pChildNode->GetRenderPass();
	unsigned int l_uiPass = static_cast<unsigned int>(l_eRenderPass);

	if (l_uiPass >= m_oChildren.size() || !m_oChildren[l_uiPass])
	{
		assert(pChildNode->GetActor());
		std::stringstream l_oSs;
		l_oSs << "Error while trying to remove child node with render pass " << l_eRenderPass;
		l_oSs << " and Actor name " << pChildNode->GetActor()->GetName();
		l_oSs << " to RootNode";
		LOG_ERROR(l_oSs.str());
		return false;
	}

	return m_oChildren[l_uiPass]->RemoveChild(pChildNode);
}
