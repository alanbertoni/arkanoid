
#include "KeyboardHandler.h"

input::KeyboardHandler::KeyboardHandler()
{
	// Init the state of the Keyboard's keys to UNDEFINED (neither UP nor DOWN)
	for (size_t l_uiIndex = 0; l_uiIndex < s_uiNKeys; ++l_uiIndex)
	{
		m_aeKeysState[l_uiIndex] = EKeyState::UNDEFINED;
		m_aeLastKeysState[l_uiIndex] = EKeyState::UNDEFINED;
	}
}

void input::KeyboardHandler::Update()
{
	//Reset the state of the Keyboard's keys for the next frame 
	while (!m_oChangedKeys.empty())
	{
		unsigned char l_ucChar = m_oChangedKeys.back();
		m_aeLastKeysState[l_ucChar] = m_aeKeysState[l_ucChar];
		m_aeKeysState[l_ucChar] = EKeyState::UNDEFINED;
		m_oChangedKeys.pop_back();
	}
}