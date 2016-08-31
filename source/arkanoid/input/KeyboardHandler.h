#pragma once

#include <cctype>
#include <vector>

namespace input
{
	class KeyboardHandler
	{
	public:
		KeyboardHandler();
		~KeyboardHandler() = default;

		KeyboardHandler(const KeyboardHandler&) = delete;
		KeyboardHandler& operator=(const KeyboardHandler&) = delete;
		KeyboardHandler(KeyboardHandler&&) = delete;
		KeyboardHandler& operator=(KeyboardHandler&&) = delete;

		bool IsKey(char cChar) const;
		bool IsKeyDown(char cChar) const;
		bool IsKeyUp(char cChar) const;

		bool OnKeyDown(unsigned char ucChar);
		bool OnKeyUp(unsigned char ucChar);
		void Update();

	private:
		enum class EKeyState
		{
			DOWN,
			UP,
			UNDEFINED,
		};

		static const size_t s_uiNKeys = 256;
		EKeyState m_aeKeysState[s_uiNKeys];
		EKeyState m_aeLastKeysState[s_uiNKeys];
		std::vector<unsigned char> m_oChangedKeys;
	};

	inline bool KeyboardHandler::OnKeyDown(unsigned char ucChar)
	{
		m_oChangedKeys.push_back(ucChar);
		m_aeKeysState[ucChar] = EKeyState::DOWN;
		return true;
	}

	inline bool KeyboardHandler::OnKeyUp(unsigned char ucChar)
	{
		m_oChangedKeys.push_back(ucChar);
		m_aeKeysState[ucChar] = EKeyState::UP;
		return true;
	}

	inline bool KeyboardHandler::IsKeyDown(char cChar) const
	{
		const int l_iChar = toupper(static_cast<int>(cChar));
		return !(m_aeLastKeysState[static_cast<unsigned char>(l_iChar)] == EKeyState::DOWN) && (m_aeKeysState[static_cast<unsigned char>(l_iChar)] == EKeyState::DOWN);
	}

	inline bool KeyboardHandler::IsKey(char cChar) const
	{
		const int l_iChar = toupper(static_cast<int>(cChar));
		return (m_aeKeysState[static_cast<unsigned char>(l_iChar)] == EKeyState::DOWN) || (m_aeLastKeysState[static_cast<unsigned char>(l_iChar)] == EKeyState::DOWN);
	}

	inline bool KeyboardHandler::IsKeyUp(char cChar) const
	{
		const int l_iChar = toupper(static_cast<int>(cChar));
		return (m_aeLastKeysState[static_cast<unsigned char>(l_iChar)] == EKeyState::DOWN) && (m_aeKeysState[static_cast<unsigned char>(l_iChar)]  == EKeyState::UP);
	}
}