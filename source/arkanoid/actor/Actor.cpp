
#include "Actor.h"
#include "ActorComponent.h"
#include "TransformComponent.h"
#include "../log/LogManager.h"
#include "../script/LuaManager.h"

#include <sstream>

actor::Actor::Actor(std::string szName)
	: m_oComponentsMap{}
	, m_szType{ "UNKNOWN" }
	, m_szName{ szName }
	, m_uiId{ 0 }
	, m_bIsEnable{ true }
{
	m_uiId = utilities::HashString(szName);
	CreateLuaObject();
}

actor::Actor::~Actor()
{
	auto l_oIt = m_oComponentsMap.begin();
	while (l_oIt != m_oComponentsMap.end())
	{
		delete l_oIt->second;
		++l_oIt;
	}
	m_oComponentsMap.clear();
}

void actor::Actor::Init(const tinyxml2::XMLElement* pData)
{
	m_szType = pData->Attribute("type");
	
	std::stringstream l_oMessage;
	l_oMessage << "Initializing Actor: " << m_szName << "(" << m_uiId << ") Type: " << m_szType;
	LOG("ACTOR", l_oMessage.str().c_str());
}

void actor::Actor::PostInit()
{
	TransformComponent* l_pTransformComponent = GetComponent<TransformComponent>(TransformComponent::sk_szTypeName);
	if (!l_pTransformComponent)
	{
		l_pTransformComponent = new TransformComponent{ this };
		AddComponent(l_pTransformComponent);
	}

	for (ActorComponents::iterator it = m_oComponentsMap.begin(); it != m_oComponentsMap.end(); ++it)
	{
		it->second->PostInit();
	}
	std::stringstream l_oMessage;
	l_oMessage << "Post Init Actor: " << m_szName << "(" << m_uiId << ") Type: " << m_szType << " complete";
	LOG("ACTOR", l_oMessage.str().c_str());
}

void actor::Actor::Update(float fDeltaTime)
{
	if (!IsEnable())
	{
		return;
	}

	for (ActorComponents::iterator it = m_oComponentsMap.begin(); it != m_oComponentsMap.end(); ++it)
	{
		it->second->Update(fDeltaTime);
	}
}

void actor::Actor::AddComponent(ActorComponent* pComponent)
{
	std::pair<ActorComponents::iterator, bool> l_oSuccess = m_oComponentsMap.insert(std::make_pair(pComponent->GetTypeId(), pComponent));
	assert(l_oSuccess.second);
}

void actor::Actor::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable("ActorMetatable");
	l_oMetaTable.SetObject("__index", l_oMetaTable);

	l_oMetaTable.RegisterObjectDirect("SetEnable", (Actor*)0, &Actor::SetEnable);
	l_oMetaTable.RegisterObjectDirect("IsEnable", (Actor*)0, &Actor::IsEnable);
	l_oMetaTable.RegisterObjectDirect("GetComponent", (Actor*)0, &Actor::GetComponentFromScript);
}

void actor::Actor::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName("ActorMetatable");

	m_oLuaObject.SetLightUserData("__object", const_cast<Actor*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

LuaPlus::LuaObject actor::Actor::GetComponentFromScript(const char* szComponentTypeName)
{
	ComponentTypeId l_uiId = ActorComponent::GetTypeIdFromName(szComponentTypeName);
	ActorComponents::iterator l_oFindIt = m_oComponentsMap.find(l_uiId);
	if (l_oFindIt != m_oComponentsMap.end())
	{
		ActorComponent* pComponent = l_oFindIt->second;
		if (pComponent)
		{
			return pComponent->GetLuaObject();
		}
	}

	std::stringstream l_oSS;
	l_oSS << "There is no ActorComponent of type " << szComponentTypeName << " for Actor with name " << m_szName;
	LOG_ERROR(l_oSS.str());
	return LuaPlus::LuaObject();
}
