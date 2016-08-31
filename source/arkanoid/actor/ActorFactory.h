#pragma once

#include "ActorComponent.h"
#include "../core/Common.h"
#include "../core/Singleton.h"

#include "tinyxml2.h"

namespace
{
	template <class BaseType, class SubType>
	BaseType* GenericObjectCreationFunction(actor::Actor* pOwner) { return new SubType{ pOwner }; }

	template <class BaseClass, class IdType>
	class GenericObjectFactory
	{
		typedef BaseClass* (*ObjectCreationFunction)(actor::Actor* i_pParentId);
		std::map<IdType, ObjectCreationFunction> m_ConstructorCallback;

	public:
		template <class SubClass>
		bool Register(IdType i_oId)
		{
			auto findIt = m_ConstructorCallback.find(i_oId);
			if (findIt == m_ConstructorCallback.end())
			{
				m_ConstructorCallback[i_oId] = &GenericObjectCreationFunction<BaseClass, SubClass>;
				return true;
			}

			return false;
		}

		BaseClass* Create(IdType i_oId, actor::Actor* pOwner)
		{
			std::map<IdType, ObjectCreationFunction>::iterator findIt = m_ConstructorCallback.find(i_oId);
			if (findIt != m_ConstructorCallback.end())
			{
				ObjectCreationFunction pFunc = findIt->second;
				return pFunc(pOwner);
			}

			return nullptr;
		}
	};
}

namespace actor
{
	class ActorFactory : public core::Singleton<ActorFactory>
	{
	public:
		typedef std::map<ActorType, const tinyxml2::XMLElement*> PrefabMap;
		static void CreateInstance();

		~ActorFactory();
		bool Init(std::string szActorsFilename);

		Actor* CreateActor(ActorType szType, std::string szName);
		ActorComponent* CreateActorComponent(Actor* pOwner, const tinyxml2::XMLElement* pComponentElement);
	private:
		ActorFactory();
		ActorFactory(const ActorFactory&) = delete;
		ActorFactory(ActorFactory&&) = delete;
		ActorFactory& operator=(const ActorFactory&) = delete;
		ActorFactory& operator=(ActorFactory&&) = delete;

		void RegisterScriptFunction();
		void RegisterActorComponents();

		bool SetupFromXML(std::string szActorsFilename);

		tinyxml2::XMLDocument m_oActorsXML;
		PrefabMap m_oPrefabMap;
		GenericObjectFactory<ActorComponent, ComponentTypeId> m_oFactoryMap;
	};
}