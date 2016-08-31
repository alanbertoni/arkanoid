
#pragma once

#include "EventManager.h"

#include <map>

namespace event
{
	template < typename T >
	class EventPublisher
	{
	public:
		EventPublisher() = default;
		~EventPublisher() = default;
		EventPublisher(const EventPublisher&) = delete;
		EventPublisher& operator=(const EventPublisher&) = delete;
		EventPublisher(EventPublisher&&) = delete;
		EventPublisher& operator=(EventPublisher&&) = delete;

		void SetPath(const std::string& szEventPath)
		{
			m_szEventPath = szEventPath;
		}

		const std::string& GetPath() const
		{
			return m_szEventPath;
		}

		void Raise(const T& i_pParam)
		{
			auto it = EventManager::GetSingleton().EditRegistry().find(GetPath());
			if (it != EventManager::GetSingleton().GetRegistry().end())
			{
				auto& l_oHandlersList = it->second;
				for (auto l_pHandler : l_oHandlersList)
				{
					l_pHandler->Invoke(static_cast<void*>(&const_cast<T&>(i_pParam)));
				}
			}
		}

	private:
		std::string	m_szEventPath;
	};

	template<>
	class EventPublisher<void>
	{
	public:
		EventPublisher() = default;
		~EventPublisher() = default;
		EventPublisher(const EventPublisher&) = delete;
		EventPublisher& operator=(const EventPublisher&) = delete;
		EventPublisher(EventPublisher&&) = delete;
		EventPublisher& operator=(EventPublisher&&) = delete;

		void SetPath(const std::string& szEventPath)
		{
			m_szEventPath = szEventPath;
		}

		const std::string& GetPath() const
		{
			return m_szEventPath;
		}

		void Raise()
		{
			auto it = EventManager::GetSingleton().EditRegistry().find(GetPath());
			if (it != EventManager::GetSingleton().GetRegistry().end())
			{
				auto& l_oHandlersList = it->second;
				for (auto l_pHandler : l_oHandlersList)
				{
					l_pHandler->Invoke(nullptr);
				}
			}
		}

	private:
		std::string	m_szEventPath;
	};

	template<>
	class EventPublisher<const char*>
	{
	public:
		EventPublisher() = default;
		~EventPublisher() = default;
		EventPublisher(const EventPublisher&) = delete;
		EventPublisher& operator=(const EventPublisher&) = delete;
		EventPublisher(EventPublisher&&) = delete;
		EventPublisher& operator=(EventPublisher&&) = delete;

		void SetPath(const std::string& szEventPath)
		{
			m_szEventPath = szEventPath;
		}

		const std::string& GetPath() const
		{
			return m_szEventPath;
		}

		void Raise(const char* i_pParam)
		{
			auto it = EventManager::GetSingleton().EditRegistry().find(GetPath());
			if (it != EventManager::GetSingleton().GetRegistry().end())
			{
				auto& l_oHandlersList = it->second;
				for (auto l_pHandler : l_oHandlersList)
				{
					l_pHandler->Invoke(static_cast<void*>(const_cast<char*>(i_pParam)));
				}
			}
		}

	private:
		std::string	m_szEventPath;
	};
}
