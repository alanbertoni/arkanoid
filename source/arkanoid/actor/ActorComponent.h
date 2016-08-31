#pragma once
#include "../utilities/String.h"
#include "../utilities/Types.h"
#include "../script/LuaManager.h"

#include <LuaPlus.h>
#include <tinyxml2.h>

namespace actor
{
	class Actor;

	class ActorComponent
	{
	public:
		static ComponentTypeId GetTypeIdFromName(std::string szComponentTypeStr);
		
		ActorComponent(Actor* pOwner);
		virtual ~ActorComponent() = default;

		virtual bool Init(const tinyxml2::XMLElement* pData) = 0;
		virtual void PostInit();
		virtual void Update(float fDeltaTime);
		
		virtual ComponentTypeId GetTypeId() const;
		virtual LuaPlus::LuaObject GetLuaObject();

		ComponentId GetId() const;
		Actor* GetOwner();
		void SetOwner(Actor* pOwner);
		bool IsEnable() const;
		void SetEnable(bool bEnable);

		static void RegisterScriptFunction();

	protected:
		virtual std::string GetTypeName() const = 0;

		static const char* sk_szMetatableName;

	private:
		ActorComponent(const ActorComponent&) = delete;
		ActorComponent(ActorComponent&&) = delete;
		ActorComponent& operator=(const ActorComponent&) = delete;
		ActorComponent& operator=(ActorComponent&&) = delete;

		void CreateLuaObject();
		LuaPlus::LuaObject GetOwnerFromScript();

		LuaPlus::LuaObject m_oLuaObject;
		Actor* m_pOwner;
		ComponentId m_uiId;
		bool m_bIsEnable;
	};

	inline ComponentId ActorComponent::GetId() const
	{
		return m_uiId;
	}

	inline Actor* ActorComponent::GetOwner()
	{
		return m_pOwner;
	}

	inline void ActorComponent::SetOwner(Actor* pOwner)
	{
		m_pOwner = pOwner;
	}

	inline ComponentTypeId ActorComponent::GetTypeId() const
	{
		return GetTypeIdFromName(GetTypeName());
	}

	inline ComponentTypeId ActorComponent::GetTypeIdFromName(std::string szComponentTypeStr)
	{
		return utilities::HashString(szComponentTypeStr);
	}

	inline bool ActorComponent::IsEnable() const
	{
		return m_bIsEnable;
	}

	inline void ActorComponent::SetEnable(bool bEnable)
	{
		m_bIsEnable = bEnable;
	}

	inline LuaPlus::LuaObject ActorComponent::GetLuaObject()
	{
		return m_oLuaObject;
	}

	inline void ActorComponent::PostInit()
	{
	}

	inline void ActorComponent::Update(float fDeltaTime)
	{
	}
}