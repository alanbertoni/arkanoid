#pragma once

#include "Common.h"
#include "Singleton.h"
#include "../actor/Actor.h"
#include "../collision/CollisionManager.h"
#include "../event/EventListener.h"
#include "../event/EventPublisher.h"
#include "../input/KeyboardHandler.h"
#include "../script/LuaManager.h"
#include "../scene/Scene.h"

namespace core
{
	class GameLogic : public Singleton<GameLogic>
	{
	public:
		enum class EGameState
		{
			INIT,
			RUNNING,
			RESET,
		};

		static void CreateInstance();
		
		~GameLogic();

		bool Init(std::string szConfigFilename);
		void Update(float fDeltaTime);
		void Render();

		bool ProcessAppMessage(const AppMessage& oEvent);
		static bool IsKey(const char* pChar);
		static bool IsKeyDown(const char* pChar);
		static bool IsKeyUp(const char* pChar);

		actor::Actor& CreateActor(const ActorType& szType, const std::string& szName, const std::string& szParentName);
		actor::Actor& GetActorFromName(std::string szName) const;
		actor::Actor& GetLastActorFromType(std::string szType) const;
		void DestroyActor(ActorId oId);

	private:
		GameLogic();
		GameLogic(const GameLogic&) = delete;
		GameLogic(GameLogic&&) = delete;
		GameLogic& operator=(const GameLogic&) = delete;
		GameLogic& operator=(GameLogic&&) = delete;

		void CreateLuaObject();

		static void RegisterScriptFunction();
		static LuaPlus::LuaObject GetGameLogicFromScript();
		static LuaPlus::LuaObject CreateActorFromScript(const char* szType, const char* szName, const char* szParentName);
		static LuaPlus::LuaObject CreateActorWithoutParentFromScript(const char* szType, const char* szName);
		static LuaPlus::LuaObject GetActorFromScript(const char* szName);
		static LuaPlus::LuaObject GetLastActorFromTypeFromScript(const char* szType);
		static void DestroyActorFromScript(const char* szName);
		void RegisterGlobalScriptFunction();

		bool SetupFromXML(const std::string& szConfigFilename);
		void UpdateActors(float fDeltaTime);
		void DeleteActors();
		void CreateSceneNode(actor::Actor* pActor, const std::string& szParentName);
		void AddToActorTypeMap(const ActorType& szType, actor::Actor* pActor);
		void RemoveFromActorTypeMap(const ActorType& szType, ActorId oId);

		void Reset();

		void OnInit();
		void OnReset();
		void OnRunning(float fDeltaTime);

		LuaPlus::LuaObject GetLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		typedef std::map<ActorId, actor::Actor*> ActorsMap;
		typedef std::map<ActorType, std::vector<actor::Actor*>> ActorTypeToActorsMap;
		event::EventListener<GameLogic> m_oOnResetEvent;
		ActorTypeToActorsMap m_oActorsTypeToActorsMap;
		ActorsMap m_oActorsMap;
		scene::Scene m_oScene;
		input::KeyboardHandler m_oKeyboardHandler;
		std::string m_szActorsFilename;
		std::string m_szScriptFilename;
		EGameState m_eState;
	};

	inline LuaPlus::LuaObject GameLogic::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline void GameLogic::UpdateActors(float fDeltaTime)
	{
		ActorsMap::iterator l_oIt = m_oActorsMap.begin();
		while (l_oIt != m_oActorsMap.end())
		{
			l_oIt->second->Update(fDeltaTime);
			++l_oIt;
		}
	}

	inline void GameLogic::Reset()
	{
		m_eState = EGameState::RESET;
		event::EventPublisher<void> oEventPublisher;
		oEventPublisher.SetPath("GameLogic/OnReset");
		oEventPublisher.Raise();
	}
}