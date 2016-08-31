
#include "ScriptComponent.h"
#include "Actor.h"
#include "../log/LogManager.h"

const char* actor::ScriptComponent::sk_szTypeName = "ScriptComponent";
const char* actor::ScriptComponent::sk_szMetatableName = "ScriptComponentMetatable";

actor::ScriptComponent::ScriptComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_eState{ EScriptState::UNINITILIZE }
	, m_ePreviousState{ EScriptState::UNINITILIZE }
	, m_fFrequency{ 0.0f }
	, m_fTime{ 0.0f }
	, m_szScriptFilename{}
{
	CreateLuaObject();
	m_oOnResetEvent.Subscribe(this, &ScriptComponent::OnReset, "GameLogic/OnReset");
	std::string l_szOnCollisionPath = "OnCollision/";
	m_oOnCollisionEvent.Subscribe(this, &ScriptComponent::OnCollision, l_szOnCollisionPath + pOwner->GetName());
}

actor::ScriptComponent::~ScriptComponent()
{
	OnExit();
}

void actor::ScriptComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("Setup", (ScriptComponent*)0, &ScriptComponent::SetupFromScript);
	l_oMetaTable.RegisterObjectDirect("Succeed", (ScriptComponent*)0, &ScriptComponent::Succeed);
	l_oMetaTable.RegisterObjectDirect("Fail", (ScriptComponent*)0, &ScriptComponent::Fail);
	l_oMetaTable.RegisterObjectDirect("Pause", (ScriptComponent*)0, &ScriptComponent::Pause);
	l_oMetaTable.RegisterObjectDirect("UnPause", (ScriptComponent*)0, &ScriptComponent::UnPause);
	l_oMetaTable.RegisterObjectDirect("IsRunning", (ScriptComponent*)0, &ScriptComponent::IsRunning);
	l_oMetaTable.RegisterObjectDirect("IsEnded", (ScriptComponent*)0, &ScriptComponent::IsEnded);
	l_oMetaTable.RegisterObjectDirect("IsPaused", (ScriptComponent*)0, &ScriptComponent::IsPaused);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::ScriptComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<ScriptComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::ScriptComponent::Init(const tinyxml2::XMLElement* pData)
{
	const char* l_szFilename = pData->Attribute("filename");
	if (l_szFilename)
	{
		m_szScriptFilename = l_szFilename;

		std::stringstream l_oSS;
		l_oSS << "ScriptComponent of Actor " << GetOwner()->GetName() << " successfully initialized with file " << l_szFilename;
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init ScriptComponent of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::ScriptComponent::Update(float fDeltaTime)
{
	if (!IsEnable() || !GetOwner()->IsEnable() || m_szScriptFilename.empty())
	{
		return;
	}

	switch (m_eState)
	{
	case actor::ScriptComponent::EScriptState::UNINITILIZE:
	{
		OnEnter();
		break;
	}
	case actor::ScriptComponent::EScriptState::RUNNING:
	{
		OnUpdate(fDeltaTime);
		break;
	}
	case actor::ScriptComponent::EScriptState::SUCCEED:
	case actor::ScriptComponent::EScriptState::FAIL:
	{
		OnExit();
		break;
	}
	case actor::ScriptComponent::EScriptState::PAUSE:
		break;
	case actor::ScriptComponent::EScriptState::NONE:
		break;
	}
}

void actor::ScriptComponent::OnEnter()
{
	if (m_oLuaObject.GetByName("OnEnter").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnEnterFunction(m_oLuaObject.GetByName("OnEnter"));
		oOnEnterFunction(m_oLuaObject);
	}

	m_eState = EScriptState::RUNNING;
}

void actor::ScriptComponent::OnUpdate(float fDeltaTime)
{
	m_fTime += fDeltaTime;
	if (m_fTime >= m_fFrequency)
	{
		if (m_oLuaObject.GetByName("OnUpdate").IsFunction())
		{
			try
			{
				LuaPlus::LuaFunction<void> oOnUpdateFunc(m_oLuaObject.GetByName("OnUpdate"));
				oOnUpdateFunc(m_oLuaObject, m_fTime);
				m_fTime = 0.0f;
			}
			catch (LuaPlus::LuaException& e)
			{
				std::stringstream l_oSS;
				l_oSS << "An Exception occured while trying to execute the OnUpdate LuaFunction in ScriptComponent of Actor " << GetOwner()->GetName();
				l_oSS << " Error message is " << e.GetErrorMessage();
				LOG("ACTOR", l_oSS.str());
			}
		}
	}
}

void actor::ScriptComponent::OnExit()
{
	if (m_oLuaObject.GetByName("OnExit").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnExitFunction(m_oLuaObject.GetByName("OnExit"));
		oOnExitFunction(m_oLuaObject);
	}
	m_eState = EScriptState::NONE;
}

void actor::ScriptComponent::OnReset()
{
	m_eState = EScriptState::UNINITILIZE;
}

void actor::ScriptComponent::SetupFromScript(LuaPlus::LuaObject oConstructionData)
{
	if (oConstructionData.IsTable())
	{
		for (LuaPlus::LuaTableIterator oConstructionDataIt = oConstructionData;	oConstructionDataIt; oConstructionDataIt.Next())
		{
			const char* szKey = oConstructionDataIt.GetKey().GetString();
			LuaPlus::LuaObject oVal = oConstructionDataIt.GetValue();

			if (strcmp(szKey, "Frequency") == 0 && oVal.IsInteger())
			{
				m_fFrequency = oVal.GetFloat();
			}
			else
			{
				m_oLuaObject.SetObject(szKey, oVal);
			}
		}
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to setup the LuaObject related to the ScriptComponent of Actor " << GetOwner()->GetName();
		l_oSS << " Construction data needs to be a table";
		LOG_ERROR(l_oSS.str());
	}
}

void actor::ScriptComponent::OnCollision(const ColliderComponent::CollisionData& oCollisionData)
{
	if (m_oLuaObject.GetByName("OnCollision").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnCollisionFunction(m_oLuaObject.GetByName("OnCollision"));
		oOnCollisionFunction(m_oLuaObject, const_cast<ColliderComponent::CollisionData&>(oCollisionData).GetLuaObject());
	}
}
