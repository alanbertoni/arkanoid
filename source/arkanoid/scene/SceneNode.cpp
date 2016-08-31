
#include "SceneNode.h"
#include "Scene.h"
#include "../actor/TransformComponent.h"

#include "../actor/RendererComponent.h"

scene::SceneNode::SceneNode(actor::Actor * pActor, ERenderPass eRenderPass)
	: m_pParent{ nullptr }
	, m_eRenderPass{ eRenderPass }
	, m_pActor{ pActor }
	, m_oChildren{}
	, m_pTransformComponent{ nullptr }
{
}

bool scene::SceneNode::PreRender(Scene& oScene)
{
	if (m_pTransformComponent)
	{
		oScene.PushAndSetMatrix(m_pTransformComponent->GetToWorldMatrix());
		return true;
	}

	return false;
}

bool scene::SceneNode::Init(Scene& oScene)
{
	if (m_pActor)
	{
		m_pTransformComponent = m_pActor->GetComponent<actor::TransformComponent>(actor::TransformComponent::sk_szTypeName);

		actor::RendererComponent* l_pRenderer = m_pActor->GetComponent<actor::RendererComponent>(actor::RendererComponent::sk_szTypeName);
		if (l_pRenderer)
		{
			l_pRenderer->InitForRendering(oScene, *this);
		}
	}

	return true;
}

bool scene::SceneNode::Render(Scene& oScene)
{
	if (m_pActor)
	{
		actor::RendererComponent* l_pRenderer = m_pActor->GetComponent<actor::RendererComponent>(actor::RendererComponent::sk_szTypeName);
		if (l_pRenderer)
		{
			l_pRenderer->Render(oScene, *this);
		}
	}

	return true;
}

bool scene::SceneNode::RenderChildren(Scene& oScene)
{
	SceneNodeList::iterator l_oIt = m_oChildren.begin();
	while (l_oIt != m_oChildren.end())
	{
		SceneNode* l_pNode = *l_oIt;
		if (l_pNode->PreRender(oScene))
		{
			if (l_pNode->IsVisible(oScene))
			{
				l_pNode->Render(oScene);
				l_pNode->RenderChildren(oScene);
			}
			l_pNode->PostRender(oScene);
		}

		++l_oIt;
	}

	return true;
}

bool scene::SceneNode::PostRender(Scene& oScene)
{
	oScene.PopMatrix();
	return true;
}

bool scene::SceneNode::AddChild(SceneNode* pChildNode)
{
	m_oChildren.push_back(pChildNode);
	pChildNode->m_pParent = this;
	return true;
}

bool scene::SceneNode::RemoveChild(SceneNode* pChildNode)
{
	SceneNodeList::iterator l_oIt = m_oChildren.begin();
	while (l_oIt != m_oChildren.end())
	{
		actor::Actor* l_pActor = (*l_oIt)->GetActor();
		if (l_pActor && l_pActor->GetId() == pChildNode->GetActor()->GetId())
		{
			m_oChildren.erase(l_oIt);
			return true;
		}
		++l_oIt;
	}

	return false;
}

void scene::SceneNode::Destroy(const Scene & oScene)
{
	if (m_pActor)
	{
		actor::RendererComponent* l_pRenderer = m_pActor->GetComponent<actor::RendererComponent>(actor::RendererComponent::sk_szTypeName);
		if (l_pRenderer)
		{
			l_pRenderer->ReleaseRenderingResources(oScene, *this);
		}
	}
}
