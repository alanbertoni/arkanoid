
#include "ActorFactory.h"
#include "Actor.h"
#include "AudioListenerComponent.h"
#include "AudioSourceComponent.h"
#include "CameraComponent.h"
#include "ColliderComponent.h"
#include "RendererComponent.h"
#include "ScriptComponent.h"
#include "TransformComponent.h"

#include <sstream>

void actor::ActorFactory::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		new ActorFactory{};
	}
}

actor::ActorFactory::ActorFactory()
	: Singleton{}
	, m_oActorsXML{}
	, m_oPrefabMap{}
	, m_oFactoryMap{}
{
	RegisterScriptFunction();
	RegisterActorComponents();
}

void actor::ActorFactory::RegisterScriptFunction()
{
	Actor::RegisterScriptFunction();
	ActorComponent::RegisterScriptFunction();
	AudioListenerComponent::RegisterScriptFunction();
	AudioSourceComponent::RegisterScriptFunction();
	CameraComponent::RegisterScriptFunction();
	ColliderComponent::RegisterScriptFunction();
	RendererComponent::RegisterScriptFunction();
	ScriptComponent::RegisterScriptFunction();
	TransformComponent::RegisterScriptFunction();
}

void actor::ActorFactory::RegisterActorComponents()
{
	m_oFactoryMap.Register<AudioListenerComponent>(ActorComponent::GetTypeIdFromName(AudioListenerComponent::sk_szTypeName));
	m_oFactoryMap.Register<AudioSourceComponent>(ActorComponent::GetTypeIdFromName(AudioSourceComponent::sk_szTypeName));
	m_oFactoryMap.Register<CameraComponent>(ActorComponent::GetTypeIdFromName(CameraComponent::sk_szTypeName));
	m_oFactoryMap.Register<ColliderComponent>(ActorComponent::GetTypeIdFromName(ColliderComponent::sk_szTypeName));
	m_oFactoryMap.Register<RendererComponent>(ActorComponent::GetTypeIdFromName(RendererComponent::sk_szTypeName));
	m_oFactoryMap.Register<ScriptComponent>(ActorComponent::GetTypeIdFromName(ScriptComponent::sk_szTypeName));
	m_oFactoryMap.Register<TransformComponent>(ActorComponent::GetTypeIdFromName(TransformComponent::sk_szTypeName));
}

actor::ActorFactory::~ActorFactory()
{
}

bool actor::ActorFactory::Init(std::string szActorsFilename)
{
	return SetupFromXML(szActorsFilename);
}

actor::Actor* actor::ActorFactory::CreateActor(ActorType szType, std::string szName)
{
	PrefabMap::const_iterator l_oIt = m_oPrefabMap.find(szType);
	if (l_oIt == m_oPrefabMap.end())
	{
		std::stringstream l_oSs;
		l_oSs << "Failed to create Actor " << szName << "due to missing Actor Type " << szType << " in the PrefabMap";
		LOG("ACTOR", l_oSs.str());
		return nullptr;
	}
	else
	{
		const tinyxml2::XMLElement* pActorElement = l_oIt->second;
		
		Actor* pActor = new Actor{ szName };

		if (!pActor)
		{
			std::stringstream l_oSs;
			l_oSs << "Failed to create Actor " << szName << " with Actor Type " << szType;
			LOG("ACTOR", l_oSs.str());
			return nullptr;
		}

		pActor->Init(pActorElement);

		const tinyxml2::XMLElement* pComponentElement = pActorElement->FirstChildElement();
		for (; pComponentElement; pComponentElement = pComponentElement->NextSiblingElement())
		{
			ActorComponent* pComponent = CreateActorComponent(pActor, pComponentElement);

			assert(pComponent);
			if (pComponent)
			{
				pActor->AddComponent(pComponent);
			}
		}

		return pActor;
	}
}

actor::ActorComponent* actor::ActorFactory::CreateActorComponent(Actor * pOwner, const tinyxml2::XMLElement* pComponentElement)
{
	assert(pOwner && pComponentElement);

	const char* pComponentTypeName = pComponentElement->Value();
	ActorComponent* pComponent = m_oFactoryMap.Create(ActorComponent::GetTypeIdFromName(pComponentTypeName), pOwner);

	if (pComponent)
	{
		pComponent->Init(pComponentElement);
		return pComponent;
	}

	std::stringstream l_oSs;
	l_oSs << "Failed to create ActorComponent " << pComponentTypeName << " belonging to " << pOwner->GetName();
	LOG("ACTOR", l_oSs.str());

	delete pComponent;
	return nullptr;
}

bool actor::ActorFactory::SetupFromXML(std::string szActorsFilename)
{
	if (szActorsFilename.empty())
	{
		LOG("ACTOR", "Failed to init ActorFactory due to missing actors filename!!!");
		return false;
	}

	m_oActorsXML.LoadFile(szActorsFilename.c_str());

	if (m_oActorsXML.Error())
	{
		LOG("ACTOR", "Error while loading actors filename");
		return false;
	}

	const tinyxml2::XMLElement* l_pRootElem = m_oActorsXML.RootElement();
	if (!l_pRootElem)
	{
		LOG("ACTOR", "Actors file does not have a Root element");
		return false;
	}

	const tinyxml2::XMLElement* l_pActorElem = nullptr;
	for (l_pActorElem = l_pRootElem->FirstChildElement("Actor"); l_pActorElem; l_pActorElem = l_pActorElem->NextSiblingElement())
	{
		ActorType l_szType = l_pActorElem->Attribute("type");
		if (!l_szType.empty())
		{
			m_oPrefabMap[l_szType] = l_pActorElem;
		}
	}

	if (m_oPrefabMap.empty())
	{
		LOG("ACTOR", "Failed to init the Actors prefab");
		return false;
	}

	LOG("ACTOR", "Actors prefab initialized successfully");
	return true;
}
