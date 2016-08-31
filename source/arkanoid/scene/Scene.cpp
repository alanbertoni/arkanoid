
#include "Scene.h"
#include "RootNode.h"
#include "../actor/CameraComponent.h"

scene::Scene::Scene(graphics::IRenderer& oRenderer)
	: m_oRenderer{ oRenderer }
	, m_ActorMap{}
	, m_oMatrixStack{}
	, m_oOnCameraInitEvent{}
{
	m_pRootNode = new RootNode{ nullptr };
	m_ActorMap[INVALID_ACTOR_ID] = m_pRootNode;

	Mat4x4 l_oIdentityMatrix;
	utilities::MathHelper::SetIdentityMatrix(l_oIdentityMatrix);
	m_oMatrixStack.emplace(std::move(l_oIdentityMatrix));

	m_oOnCameraInitEvent.Subscribe(this, &Scene::OnCameraInit, "Init/ActorComponent/CameraComponent");
}

scene::Scene::~Scene()
{
	while (!m_oMatrixStack.empty())
	{
		m_oMatrixStack.pop();
	}

	delete m_pRootNode;
}

bool scene::Scene::Render()
{
	if (m_pRootNode)
	{
		return m_pRootNode->Render(*this);
	}

	return false;
}

scene::SceneNode* scene::Scene::FindSceneNode(ActorId oId)
{
	auto l_oIt = m_ActorMap.find(oId);
	if (l_oIt != m_ActorMap.end())
	{
		return l_oIt->second;
	}

	std::stringstream l_oSS;
	l_oSS << "There is no SceneNode associated to Actor with id " << oId;
	LOG_ERROR(l_oSS.str());
	assert(0 && l_oSS.str().c_str());
	return nullptr;
}

bool scene::Scene::AddChild(SceneNode* pChildNode)
{
	return AddChild(INVALID_ACTOR_ID, pChildNode);
}

bool scene::Scene::AddChild(ActorId oId, SceneNode* pChildNode)
{
	assert(pChildNode);
	SceneNode* l_pNode = FindSceneNode(oId);
	assert(l_pNode);

	m_ActorMap[pChildNode->GetActor()->GetId()] = pChildNode;
	bool l_bResult = l_pNode->AddChild(pChildNode);
	pChildNode->Init(*this);
	return l_bResult;
}

bool scene::Scene::RemoveChild(ActorId oId)
{
	if (oId == INVALID_ACTOR_ID)
	{
		return false;
	}

	SceneNode* l_pNode = FindSceneNode(oId);
	assert(l_pNode);

	SceneNode* l_pParentNode = l_pNode->GetParent();
	assert(l_pParentNode);

	l_pParentNode->RemoveChild(l_pNode);

	l_pNode->Destroy(*this);
	delete l_pNode;

	auto l_oIt = m_ActorMap.find(oId);
	if (l_oIt != m_ActorMap.end())
	{
		m_ActorMap.erase(l_oIt);
	}

	return true;
}

void scene::Scene::OnCameraInit(const ActorId& oCameraId)
{
	m_pCamera = FindSceneNode(oCameraId);
	assert(m_pCamera);
	actor::CameraComponent* l_pCameraComponent = m_pCamera->GetActor()->GetComponent<actor::CameraComponent>(actor::CameraComponent::sk_szTypeName);
	assert(l_pCameraComponent);

	Dimension l_oResizeDim{ m_oRenderer.GetScreenWidth(), m_oRenderer.GetScreenHeight() };
	l_pCameraComponent->OnResize(l_oResizeDim);
}
