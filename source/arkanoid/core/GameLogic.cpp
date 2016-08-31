
#include "GameLogic.h"
#include "GameApp.h"
#include "../actor/ActorFactory.h"
#include "../graphics/GeometryGenerator.h"
#include "../scene/SceneNode.h"

const char* core::GameLogic::sk_szMetatableName = "GameLogicMetatable";

void core::GameLogic::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		GameLogic::RegisterScriptFunction();
		GameLogic* l_oGameLogic = new GameLogic{};
		l_oGameLogic->RegisterGlobalScriptFunction();
	}
}

core::GameLogic::GameLogic()
	: Singleton{}
	, m_oActorsMap{}
	, m_oActorsTypeToActorsMap{}
	, m_szActorsFilename{}
	, m_szScriptFilename{}
	, m_eState{ EGameState::INIT }
	, m_oScene{ GameApp::GetSingleton().GetRenderer() }
	, m_oKeyboardHandler{}
	, m_oOnResetEvent{}
{
	CreateLuaObject();
	m_oOnResetEvent.Subscribe(this, &GameLogic::Reset, "ResetGame");
}

core::GameLogic::~GameLogic()
{
	DeleteActors();
	actor::ActorFactory::Release();
	graphics::GeometryGenerator::Release();
	collision::CollisionManager::Release();
}

void core::GameLogic::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<GameLogic*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

void core::GameLogic::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
}

void core::GameLogic::RegisterGlobalScriptFunction()
{
	LuaPlus::LuaObject pGlobalVars = script::LuaManager::GetSingleton().GetGlobalVars();
	pGlobalVars.RegisterDirect("CreateActor", &GameLogic::CreateActorFromScript);
	pGlobalVars.RegisterDirect("CreateActorWithoutParent", &GameLogic::CreateActorWithoutParentFromScript);
	pGlobalVars.RegisterDirect("DestroyActorFromName", &GameLogic::DestroyActorFromScript);
	pGlobalVars.RegisterDirect("GetActorFromName", &GameLogic::GetActorFromScript);
	pGlobalVars.RegisterDirect("GetLastActorFromType", &GameLogic::GetLastActorFromTypeFromScript);
	pGlobalVars.RegisterDirect("GetGameLogic", &GameLogic::GetGameLogicFromScript);
	pGlobalVars.RegisterDirect("IsKey", &GameLogic::IsKey);
	pGlobalVars.RegisterDirect("IsKeyDown", &GameLogic::IsKeyDown);
	pGlobalVars.RegisterDirect("IsKeyUp", &GameLogic::IsKeyUp);
}

bool core::GameLogic::Init(std::string szConfigFilename)
{
	if (!SetupFromXML(szConfigFilename))
	{
		return false;
	}

	if (!m_szScriptFilename.empty())
	{
		script::LuaManager::GetSingleton().ExecuteFile(m_szScriptFilename.c_str());
	}

	collision::CollisionManager::CreateInstance();
	graphics::GeometryGenerator::CreateInstance();
	actor::ActorFactory::CreateInstance();

	if (!actor::ActorFactory::GetSingleton().Init(m_szActorsFilename))
	{
		LOG_ERROR("Failed to init the ActorFactory");
		return false;
	}

	return true;
}

void core::GameLogic::Update(float fDeltaTime)
{
	switch (m_eState)
	{
	case core::GameLogic::EGameState::INIT:
	{
		OnInit();
		break;
	}
	case core::GameLogic::EGameState::RUNNING:
	{
		OnRunning(fDeltaTime);
		break;
	}
	case core::GameLogic::EGameState::RESET:
	{
		OnReset();
		break;
	}
	}

	UpdateActors(fDeltaTime);
	collision::CollisionManager::GetSingleton().Update();
	m_oKeyboardHandler.Update();
}

void core::GameLogic::Render()
{
	m_oScene.Render();
}

bool core::GameLogic::ProcessAppMessage(const AppMessage & oEvent)
{
	switch (oEvent.m_oMessageId)
	{
	case WM_KEYDOWN:
	{
		return m_oKeyboardHandler.OnKeyDown(static_cast<unsigned char>(oEvent.m_oParam));
	}
	case WM_KEYUP:
	{
		return m_oKeyboardHandler.OnKeyUp(static_cast<unsigned char>(oEvent.m_oParam));
	}
	}
	return false;
}

void core::GameLogic::OnInit()
{
	if (m_oLuaObject.GetByName("OnInit").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnInitFunction(m_oLuaObject.GetByName("OnInit"));
		oOnInitFunction(m_oLuaObject);
	}

	m_eState = EGameState::RESET;

	OnReset();
}

void core::GameLogic::OnReset()
{
	if (m_oLuaObject.GetByName("OnReset").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnResetFunction(m_oLuaObject.GetByName("OnReset"));
		oOnResetFunction(m_oLuaObject);
	}

	m_eState = EGameState::RUNNING;
}

void core::GameLogic::OnRunning(float fDeltaTime)
{
	if (m_oLuaObject.GetByName("OnRunning").IsFunction())
	{
		LuaPlus::LuaFunction<void> oOnRunningFunction(m_oLuaObject.GetByName("OnRunning"));
		oOnRunningFunction(m_oLuaObject, fDeltaTime);
	}
}

actor::Actor& core::GameLogic::CreateActor(const ActorType& szType, const std::string& szName, const std::string& szParentName)
{
	actor::Actor* l_pActor = actor::ActorFactory::GetSingleton().CreateActor(szType, szName);
	if (l_pActor)
	{
		m_oActorsMap[l_pActor->GetId()] = l_pActor;

		CreateSceneNode(l_pActor, szParentName);

		AddToActorTypeMap(szType, l_pActor);
		l_pActor->PostInit();

		return *l_pActor;
	}
	else
	{
		assert(0 && "Failed to createActor");
		return *(new actor::Actor{ "INVALID ACTOR" });
	}
}

actor::Actor& core::GameLogic::GetActorFromName(std::string szName) const
{
	auto l_oIt = m_oActorsMap.find(utilities::HashString(szName));
	if (l_oIt != m_oActorsMap.end())
	{
		return *(l_oIt->second);
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "There is no Actor with name " << szName;
		LOG_ERROR(l_oSS.str());
		assert(0 && l_oSS.str().c_str());
		return *(new actor::Actor{ "INVALID ACTOR" });
	}
}

actor::Actor& core::GameLogic::GetLastActorFromType(std::string szType) const
{
	auto l_oIt = m_oActorsTypeToActorsMap.find(szType);
	if (l_oIt != m_oActorsTypeToActorsMap.end())
	{
		auto it = l_oIt->second.rbegin();
		if (it != l_oIt->second.rend())
		{
			return *(*it);
		}
	}

	std::stringstream l_oSS;
	l_oSS << "There is no Actor of type " << szType;
	LOG_ERROR(l_oSS.str());
	assert(0 && l_oSS.str().c_str());
	return *(new actor::Actor{ "INVALID ACTOR" });
}

void core::GameLogic::DestroyActor(ActorId oId)
{
	m_oScene.RemoveChild(oId);

	auto l_oIt = m_oActorsMap.find(oId);
	if (l_oIt != m_oActorsMap.end())
	{
		auto l_pActor = l_oIt->second;
		RemoveFromActorTypeMap(l_pActor->GetType(), oId);
		delete l_pActor;
		m_oActorsMap.erase(l_oIt);
	}
}

void core::GameLogic::DeleteActors()
{
	ActorsMap::iterator l_oIt = m_oActorsMap.begin();
	while (l_oIt != m_oActorsMap.end())
	{
		m_oScene.RemoveChild(l_oIt->first);
		delete l_oIt->second;
		++l_oIt;
	}

	m_oActorsMap.clear();
	m_oActorsTypeToActorsMap.clear();
}

void core::GameLogic::CreateSceneNode(actor::Actor * pActor, const std::string& szParentName)
{
	ActorId l_oId = utilities::HashString(szParentName);
	if (szParentName.empty() || l_oId == INVALID_ACTOR_ID)
	{
		m_oScene.AddChild(new scene::SceneNode{ pActor });
	}
	else
	{
		m_oScene.AddChild(l_oId, new scene::SceneNode{ pActor });
	}
}

void core::GameLogic::AddToActorTypeMap(const ActorType& szType, actor::Actor* pActor)
{
	auto l_oIt = m_oActorsTypeToActorsMap.find(szType);
	if (l_oIt != m_oActorsTypeToActorsMap.end())
	{
		l_oIt->second.push_back(pActor);
	}
	else
	{
		std::vector<actor::Actor*> l_oActorList;
		l_oActorList.push_back(pActor);
		m_oActorsTypeToActorsMap.emplace(szType, std::move(l_oActorList));
	}
}

void core::GameLogic::RemoveFromActorTypeMap(const ActorType& szType, ActorId oId)
{
	auto l_oIt = m_oActorsTypeToActorsMap.find(szType);
	if (l_oIt != m_oActorsTypeToActorsMap.end())
	{
		auto& l_oActorList = l_oIt->second;
		for (auto it = l_oActorList.begin(); it != l_oActorList.end(); ++it)
		{
			if ((*it)->GetId() == oId)
			{
				l_oActorList.erase(it);
				break;
			}
		}
	}
}

LuaPlus::LuaObject core::GameLogic::GetGameLogicFromScript()
{
	return GetSingleton().GetLuaObject();
}

LuaPlus::LuaObject core::GameLogic::CreateActorFromScript(const char* szType, const char* szName, const char* szParentName)
{
	return GetSingleton().CreateActor(szType, szName, szParentName).GetLuaObject();
}

LuaPlus::LuaObject core::GameLogic::CreateActorWithoutParentFromScript(const char* szType, const char* szName)
{
	return GetSingleton().CreateActor(szType, szName, "").GetLuaObject();
}

LuaPlus::LuaObject core::GameLogic::GetActorFromScript(const char* szName)
{
	return GetSingleton().GetActorFromName(szName).GetLuaObject();
}

LuaPlus::LuaObject core::GameLogic::GetLastActorFromTypeFromScript(const char* szType)
{
	return GetSingleton().GetLastActorFromType(szType).GetLuaObject();
}

void core::GameLogic::DestroyActorFromScript(const char* szName)
{
	GetSingleton().DestroyActor(utilities::HashString(szName));
}

bool core::GameLogic::IsKey(const char* pChar)
{
	if (!pChar)
	{
		return false;
	}

	return GetSingleton().m_oKeyboardHandler.IsKey(*pChar);
}

bool core::GameLogic::IsKeyDown(const char* pChar)
{
	if (!pChar)
	{
		return false;
	}

	return GetSingleton().m_oKeyboardHandler.IsKeyDown(*pChar);
}

bool core::GameLogic::IsKeyUp(const char* pChar)
{
	if (!pChar)
	{
		return false;
	}

	return GetSingleton().m_oKeyboardHandler.IsKeyUp(*pChar);
}

bool core::GameLogic::SetupFromXML(const std::string& szConfigFilename)
{
	if (szConfigFilename.empty())
	{
		LOG("LOGIC", "Failed to init GameLogic due to missing config filename!!!");
		return false;
	}

	tinyxml2::XMLDocument l_oConfigXMLDoc;
	l_oConfigXMLDoc.LoadFile(szConfigFilename.c_str());

	if (l_oConfigXMLDoc.Error())
	{
		LOG("LOGIC", "Error while loading config filename");
		return false;
	}

	const tinyxml2::XMLElement* l_pRootElem = l_oConfigXMLDoc.RootElement();
	if (!l_pRootElem)
	{
		LOG("LOGIC", "Config file does not have a Root element");
		return false;
	}

	const tinyxml2::XMLElement* l_pGameLogicElem = l_pRootElem->FirstChildElement("GameLogic");
	if (l_pGameLogicElem)
	{
		m_szScriptFilename = l_pGameLogicElem->Attribute("script");

		const tinyxml2::XMLElement* l_pActorsElem = l_pGameLogicElem->FirstChildElement("Actors");
		if (!l_pActorsElem)
		{
			LOG("LOGIC", "Missing Actors element in config file");
			return false;
		}

		m_szActorsFilename = l_pActorsElem->Attribute("filename");

		LOG("LOGIC", "GameLogic successfully config");
		return true;
	}

	LOG("LOGIC", "Missing GameLogic element in config file");
	return false;
}
