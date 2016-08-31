
#include "ActorComponent.h"
#include "Actor.h"

const char* actor::ActorComponent::sk_szMetatableName = "ActorComponentMetatable";

actor::ActorComponent::ActorComponent(Actor* pOwner)
	: m_pOwner{ pOwner }
	, m_bIsEnable{ true }
{
	static ComponentId s_uiId = 0;
	m_uiId = ++s_uiId;
	CreateLuaObject();
}

void actor::ActorComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);

	l_oMetaTable.RegisterObjectDirect("SetEnable", (ActorComponent*)0, &ActorComponent::SetEnable);
	l_oMetaTable.RegisterObjectDirect("IsEnable", (ActorComponent*)0, &ActorComponent::IsEnable);
	l_oMetaTable.RegisterObjectDirect("GetOwner", (ActorComponent*)0, &ActorComponent::GetOwnerFromScript);
}

void actor::ActorComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<ActorComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

LuaPlus::LuaObject actor::ActorComponent::GetOwnerFromScript()
{
	return GetOwner()->GetLuaObject();
}