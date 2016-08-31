#pragma once
// Adapted from "Game Coding Complete 4th Edition"

#include "../core/Singleton.h"

#include <LuaPlus.h>

namespace script
{
	class LuaManager : public core::Singleton<LuaManager>
	{
	public:
		static void CreateInstance();
		~LuaManager();

		void Init();
		void ExecuteFile(const char* i_pFilename);
		void ExecuteString(const char* i_pString);

		LuaPlus::LuaObject GetGlobalVars(void);
		LuaPlus::LuaState* GetLuaState(void) const;

	private:
		static void Print(const char* i_szString);

		void RegisterGlobalLuaFunction();
		void PrintError(int errorNum);
		void ClearStack();

		LuaManager();
		LuaManager(const LuaManager&) = delete;
		LuaManager(LuaManager&&) = delete;
		LuaManager& operator=(const LuaManager&) = delete;
		LuaManager& operator=(LuaManager&&) = delete;

		LuaPlus::LuaState* m_pLuaState;
	};
}
