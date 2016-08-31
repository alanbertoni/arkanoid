#include "LuaManager.h"
#include "../log/LogManager.h"

void script::LuaManager::CreateInstance()
{
	if(GetSingletonPtr() == nullptr)
		new LuaManager();
}

script::LuaManager::LuaManager()
	: m_pLuaState{ nullptr }
{
	Init();
}

script::LuaManager::~LuaManager()
{
	if (m_pLuaState)
	{
		LuaPlus::LuaState::Destroy(m_pLuaState);
		m_pLuaState = nullptr;
	}
}

void script::LuaManager::Init()
{
	m_pLuaState = LuaPlus::LuaState::Create(true);
	if (m_pLuaState == nullptr)
	{
		LOG_ERROR("Error creating LuaState!");
		return;
	}

	RegisterGlobalLuaFunction();
}

void script::LuaManager::RegisterGlobalLuaFunction()
{
	GetGlobalVars().RegisterDirect("Print", &LuaManager::Print);
	GetGlobalVars().RegisterDirect("ExecuteFile", (*this), &LuaManager::ExecuteFile);
	GetGlobalVars().RegisterDirect("ExecuteString", (*this), &LuaManager::ExecuteString);
}

void script::LuaManager::ExecuteFile(const char* pFilename)
{
	int result = m_pLuaState->DoFile(pFilename);
	if (result != 0)
	{
		PrintError(result);
		assert(0 && "Script error!");
	}
}

void script::LuaManager::ExecuteString(const char* pString )
{
	if (strlen(pString) > 0)
	{
		int result = m_pLuaState->DoString(pString);
		if (result != 0)
		{
			PrintError(result);
			assert(0 && "Script error!");
		}
	}
}

LuaPlus::LuaObject script::LuaManager::GetGlobalVars(void)
{
	return m_pLuaState->GetGlobals();
}

LuaPlus::LuaState* script::LuaManager::GetLuaState(void) const
{
	return m_pLuaState;
}

void script::LuaManager::PrintError(int errorNum)
{
	LuaPlus::LuaStackObject stackObj(m_pLuaState,-1);
	const char* errStr = stackObj.GetString();
	if (errStr)
	{
		ClearStack();
	}
	else
	{
		errStr = "Unknown Lua parse error";
	}

	LOG_ERROR(errStr);
}

void script::LuaManager::ClearStack()
{
	m_pLuaState->SetTop(0);
}

void script::LuaManager::Print(const char* i_szString)
{
	printf(i_szString);
	LOG_INFO(i_szString);
}
