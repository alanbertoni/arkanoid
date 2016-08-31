#pragma once

#include "../log/LogManager.h"
#include "../utilities/String.h"
#include "../utilities/Types.h"

#include <LuaPlus.h>
#include <tinyxml2.h>

namespace actor
{
	class ActorComponent;

	class Actor
	{
	public:
		typedef std::map<ComponentTypeId, ActorComponent*> ActorComponents;

		explicit Actor(std::string szName);
		~Actor();

		Actor(const Actor&) = delete;
		Actor& operator=(const Actor&) = delete;
		Actor(Actor&&) = delete;
		Actor& operator=(Actor&&) = delete;

		void Init(const tinyxml2::XMLElement* pData);
		void PostInit();
		void Update(float fDeltaTime);

		const ActorComponents& GetComponents() const;
		ActorType GetType() const;
		std::string GetName() const;
		ActorId GetId() const;
		bool IsEnable() const;

		void SetEnable(bool bEnable);
		void AddComponent(ActorComponent* pComponent);

		template <class ComponentType>
		ComponentType* GetComponent(ComponentTypeId uiId);

		template <class ComponentType>
		ComponentType* GetComponent(const char* szComponentTypeName);

		static void RegisterScriptFunction();
		LuaPlus::LuaObject GetLuaObject();

	private:
		void CreateLuaObject();
		LuaPlus::LuaObject GetComponentFromScript(const char* szComponentTypeName);
		
		LuaPlus::LuaObject m_oLuaObject;
		ActorComponents m_oComponentsMap;
		ActorType m_szType;
		std::string m_szName;
		ActorId m_uiId;
		bool m_bIsEnable;
	};

	inline ActorId Actor::GetId() const
	{
		return m_uiId;
	}

	inline ActorType Actor::GetType() const
	{
		return m_szType;
	}

	inline std::string Actor::GetName() const
	{
		return m_szName;
	}

	inline const Actor::ActorComponents& Actor::GetComponents() const
	{
		return m_oComponentsMap;
	}

	inline bool Actor::IsEnable() const
	{
		return m_bIsEnable;
	}

	inline void Actor::SetEnable(bool bEnable)
	{
		m_bIsEnable = bEnable;
	}

	inline LuaPlus::LuaObject Actor::GetLuaObject()
	{
		assert(!m_oLuaObject.IsNil());
		return m_oLuaObject;
	}

	template <class ComponentType>
	inline ComponentType* Actor::GetComponent(ComponentTypeId uiId)
	{
		ActorComponents::iterator l_oFindIt = m_oComponentsMap.find(uiId);
		if (l_oFindIt != m_oComponentsMap.end())
		{
			ComponentType* pComponent = static_cast<ComponentType*>(l_oFindIt->second);
			if (pComponent)
				return pComponent;
		}

		std::stringstream l_oSS;
		l_oSS << "There is no ActorComponent with id " << uiId << " for Actor with name " << m_szName;
		LOG_ERROR(l_oSS.str());
		return nullptr;
	}

	template <class ComponentType>
	inline ComponentType* Actor::GetComponent(const char* szComponentTypeName)
	{
		ComponentTypeId l_uiId = ActorComponent::GetTypeIdFromName(szComponentTypeName);
		ActorComponents::iterator l_oFindIt = m_oComponentsMap.find(l_uiId);
		if (l_oFindIt != m_oComponentsMap.end())
		{
			ComponentType* pComponent = static_cast<ComponentType*>(l_oFindIt->second);
			if (pComponent)
				return pComponent;
		}

		std::stringstream l_oSS;
		l_oSS << "There is no ActorComponent of type " << szComponentTypeName << " for Actor with name " << m_szName;
		LOG_ERROR(l_oSS.str());
		return nullptr;
	}
}