
#include "EventManager.h"
#include "EventPublisher.h"
#include "../script/LuaManager.h"

void event::EventManager::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
		new EventManager();
}

event::EventManager::EventManager()
	: Singleton{}
{
	RegisterGlobalLuaFunction();
}

void event::EventManager::RegisterGlobalLuaFunction()
{
	LuaPlus::LuaObject oLuaObject = script::LuaManager::GetSingleton().GetGlobalVars();

	oLuaObject.RegisterDirect("SendEvent", &EventManager::RaiseEventFromScript);
}

void event::EventManager::RaiseEventFromScript(const char* szEventPath, const char* szParam)
{
	if (szParam && strlen(szParam) > 0)
	{
		event::EventPublisher<const char*> oEventPublisher;
		oEventPublisher.SetPath(szEventPath);
		oEventPublisher.Raise(szParam);
	}
	else
	{
		event::EventPublisher<void> oEventPublisher;
		oEventPublisher.SetPath(szEventPath);
		oEventPublisher.Raise();
	}
}

void event::EventManager::EventHandler::SubscribeEventHandler(const std::string& szEventPath)
{
	EventsMap& vRegistry{ EventManager::GetSingleton().EditRegistry() };
	auto l_oIt = vRegistry.find(szEventPath);
	if (l_oIt != vRegistry.end())
	{
		l_oIt->second.push_back(this);
	}
	else
	{
		EventHandlersList l_oList;
		l_oList.push_back(this);
		vRegistry.emplace(szEventPath, l_oList);
	}

	m_oEventPath = szEventPath;
}

void event::EventManager::EventHandler::UnsubscribeEventHandler()
{
	EventsMap& vRegistry{ EventManager::GetSingleton().EditRegistry() };
	auto l_oIt = vRegistry.find(GetPath());
	assert(l_oIt != vRegistry.end());

	auto it = l_oIt->second.begin();
	while (it != l_oIt->second.end())
	{
		auto l_oTempIt = it;
		if ((*l_oTempIt) == this)
		{
			l_oIt->second.erase(l_oTempIt);
			break;
		}
		++it;
	}

	if (l_oIt->second.empty())
	{
		vRegistry.erase(l_oIt);
	}
}
