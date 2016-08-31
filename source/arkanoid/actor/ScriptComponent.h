#pragma once

#include "ActorComponent.h"
#include "ColliderComponent.h"
#include "../event/EventListener.h"

namespace actor
{
	class ScriptComponent : public ActorComponent
	{
	public:
		enum class EScriptState
		{
			UNINITILIZE,
			RUNNING,
			SUCCEED,
			FAIL,
			PAUSE,
			NONE
		};

		ScriptComponent(Actor* pOwner);
		~ScriptComponent();

		virtual bool Init(const tinyxml2::XMLElement* pData) override;
		virtual void PostInit() override;
		virtual void Update(float fDeltaTime) override;
		virtual LuaPlus::LuaObject GetLuaObject() override;

		void Succeed();
		void Fail();
		void Pause();
		void UnPause();

		bool IsRunning() const;
		bool IsPaused() const;
		bool IsEnded() const;

		EScriptState GetState() const;
		const std::string& GetFilename() const;

		static void RegisterScriptFunction();
		static const char* sk_szTypeName;

	protected:
		virtual std::string GetTypeName() const override;

	private:
		ScriptComponent(const ScriptComponent&) = delete;
		ScriptComponent(ScriptComponent&&) = delete;
		ScriptComponent& operator=(const ScriptComponent&) = delete;
		ScriptComponent& operator=(ScriptComponent&&) = delete;

		void OnEnter();
		void OnUpdate(float fDeltaTime);
		void OnExit();
		void OnReset();
		void OnCollision(const ColliderComponent::CollisionData& oCollisionData);

		void SetupFromScript(LuaPlus::LuaObject oConstructionData);
		void CreateLuaObject();

		static const char* sk_szMetatableName;
		LuaPlus::LuaObject m_oLuaObject;

		event::EventListener<ScriptComponent> m_oOnResetEvent;
		event::EventListener<ScriptComponent, ColliderComponent::CollisionData> m_oOnCollisionEvent;
		std::string m_szScriptFilename;
		float m_fTime;
		float m_fFrequency;
		EScriptState m_eState;
		EScriptState m_ePreviousState;
	};

	inline std::string ScriptComponent::GetTypeName() const
	{
		return sk_szTypeName;
	}

	inline LuaPlus::LuaObject ScriptComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline const std::string& ScriptComponent::GetFilename() const
	{
		return m_szScriptFilename;
	}

	inline void ScriptComponent::PostInit()
	{
		if (!m_szScriptFilename.empty())
		{
			script::LuaManager::GetSingleton().ExecuteFile(m_szScriptFilename.c_str());
		}
	}

	inline void ScriptComponent::Succeed()
	{
		m_eState = EScriptState::SUCCEED;
	}

	inline void ScriptComponent::Fail()
	{
		m_eState = EScriptState::FAIL;
	}

	inline void ScriptComponent::Pause()
	{
		m_ePreviousState = m_eState;
		m_eState = EScriptState::PAUSE;
	}

	inline void ScriptComponent::UnPause()
	{
		assert(m_ePreviousState != EScriptState::NONE);
		m_eState = m_ePreviousState;
		m_ePreviousState = EScriptState::NONE;
	}

	inline bool ScriptComponent::IsRunning() const
	{
		return m_eState == EScriptState::RUNNING;
	}

	inline bool ScriptComponent::IsPaused() const
	{
		return m_eState == EScriptState::PAUSE;
	}

	inline bool ScriptComponent::IsEnded() const
	{
		return m_eState == EScriptState::SUCCEED || m_eState == EScriptState::FAIL;
	}

	inline ScriptComponent::EScriptState ScriptComponent::GetState() const
	{
		return m_eState;
	}
}
