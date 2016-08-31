
#pragma once

#include "EventManager.h"

#include <string>

namespace event
{
	template < typename Class, typename ParamType = void >
	class EventListener;

	//EventListener with param
	template < typename Class, typename ParamType>
	class EventListener : public EventManager::EventHandler
	{
	public:
		typedef void (Class::*CallbackFunc)(const ParamType&);

		EventListener()
			: EventManager::EventHandler{}
			, m_pTargetObject{ nullptr }
			, m_pMethod{ nullptr }
		{
		}

		~EventListener() = default;
		EventListener(const EventListener&) = delete;
		EventListener& operator=(const EventListener&) = delete;
		EventListener(EventListener&&) = delete;
		EventListener& operator=(EventListener&&) = delete;

		void Subscribe(Class* i_pTargetObject, CallbackFunc i_pMethod, const std::string i_oEventPath)
		{
			m_pTargetObject = i_pTargetObject;
			m_pMethod = i_pMethod;

			SubscribeEventHandler(i_oEventPath);
		}

		void Invoke(void* i_pParam)
		{
			if (m_pTargetObject)
			{
				const ParamType& oParam = *static_cast<const ParamType*>(i_pParam);

				(m_pTargetObject->*m_pMethod)(oParam);
			}
		}

	private:	
		Class* m_pTargetObject;
		CallbackFunc m_pMethod;
	};

	//EventListener without param
	template < typename Class >
	class EventListener<Class, void> : public EventManager::EventHandler
	{
	public:
		typedef void (Class::*CallbackFunc)();

		EventListener()
			: EventManager::EventHandler{}
			, m_pTargetObject{ nullptr }
			, m_pMethod{ nullptr }
		{
		}

		~EventListener() = default;
		EventListener(const EventListener&) = delete;
		EventListener& operator=(const EventListener&) = delete;
		EventListener(EventListener&&) = delete;
		EventListener& operator=(EventListener&&) = delete;

		void Subscribe(Class* i_pTargetObject, CallbackFunc i_pMethod, const std::string i_oEventPath)
		{
			m_pTargetObject = i_pTargetObject;
			m_pMethod = i_pMethod;

			SubscribeEventHandler(i_oEventPath);
		}

		void Invoke(void* i_pParam)
		{
			if (m_pTargetObject)
			{
				(m_pTargetObject->*m_pMethod)();
			}
		}

	private:
		Class* m_pTargetObject;
		CallbackFunc m_pMethod;
	};

	//EventListener with param const char*
	template < typename Class >
	class EventListener<Class, const char*> : public EventManager::EventHandler
	{
	public:
		typedef void (Class::*CallbackFunc)(const char*);

		EventListener()
			: EventManager::EventHandler{}
			, m_pTargetObject{ nullptr }
			, m_pMethod{ nullptr }
		{
		}

		~EventListener() = default;
		EventListener(const EventListener&) = delete;
		EventListener& operator=(const EventListener&) = delete;
		EventListener(EventListener&&) = delete;
		EventListener& operator=(EventListener&&) = delete;

		void Subscribe(Class* i_pTargetObject, CallbackFunc i_pMethod, const std::string i_oEventPath)
		{
			m_pTargetObject = i_pTargetObject;
			m_pMethod = i_pMethod;

			SubscribeEventHandler(i_oEventPath);
		}

		void Invoke(void* i_pParam)
		{
			if (m_pTargetObject)
			{
				const char* pParam = static_cast<const char*>(i_pParam);
				(m_pTargetObject->*m_pMethod)(pParam);
			}
		}

	private:	
		Class* m_pTargetObject;
		CallbackFunc m_pMethod;
	};
}
