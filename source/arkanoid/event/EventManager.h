
#pragma once
#include "../core/Singleton.h"

#include <map>
#include <string>
#include <vector>

namespace event
{
	class EventManager : public core::Singleton<EventManager>
	{
	public:
		class EventHandler
		{
		public:
			EventHandler() = default;

			virtual ~EventHandler()
			{
				UnsubscribeEventHandler();
			}

			EventHandler(const EventHandler&) = delete;
			EventHandler& operator=(const EventHandler&) = delete;
			EventHandler(EventHandler&&) = delete;
			EventHandler& operator=(EventHandler&&) = delete;

			const std::string& GetPath() const
			{
				return m_oEventPath;
			}

			void SubscribeEventHandler(const std::string& szEventPath);
			void UnsubscribeEventHandler();

			virtual void Invoke(void* pParam) = 0;

		private:
			std::string	m_oEventPath;
		};

	private:
		typedef std::vector<EventHandler*> EventHandlersList;
		typedef std::map<std::string, EventHandlersList> EventsMap;
		typedef EventsMap::iterator	EventsMapIt;

	public:
		static void CreateInstance();

		~EventManager() = default;

		const EventsMap& GetRegistry() const
		{
			return m_oRegistryMap;
		}

		EventsMap& EditRegistry()
		{
			return m_oRegistryMap;
		}

	private:
		EventManager();
		EventManager(const EventManager&) = delete;
		EventManager& operator=(const EventManager&) = delete;
		EventManager(EventManager&&) = delete;
		EventManager& operator=(EventManager&&) = delete;

		static void RegisterGlobalLuaFunction();
		static void RaiseEventFromScript(const char* szEventPath, const char* szParam);

		EventsMap m_oRegistryMap;
	};
}
