#pragma once

#include "Common.h"
#include "Singleton.h"
#include "../event/EventPublisher.h"
#include "../graphics/IRenderer.h"
#include "../utilities/Time.h"

#include <string>
#include <Windows.h>

namespace core
{
	class GameApp : public Singleton<GameApp>
	{
	public:
		static void CreateInstance();

		~GameApp();

		bool Init();
		void Run();
		void Close();

		bool IsPaused() const;
		graphics::IRenderer& GetRenderer() const;
		
		bool ProcessAppMessage(const AppMessage& oEvent);

	private:
		GameApp();
		GameApp(const GameApp&) = delete;
		GameApp(GameApp&&) = delete;
		GameApp& operator=(const GameApp&) = delete;
		GameApp& operator=(GameApp&&) = delete;
		
		void SetupFromXML(const char* szConfigFilename);
		bool InitWindow();
		
		void Update(float fDeltaTime) const;
		void Render() const;

		void OnPause();
		void OnUnPause();
		void OnExit();
		void OnResize() const;
		
		void ComputeFrameStats() const;

		utilities::Time m_oTime;
		WindowHandle m_oHWnd;
		graphics::IRenderer* m_pRenderer;
		std::string m_szWindowTitle;
		int m_iWindowWidth;
		int m_iWindowHeight;
		int m_iTargetFPS;
		bool m_bExit;
		bool m_bIsPaused;
		bool m_bIsResizing;
	};

	inline bool GameApp::IsPaused() const
	{
		return m_bIsPaused;
	}

	inline graphics::IRenderer& GameApp::GetRenderer() const
	{
		assert(m_pRenderer);
		return *m_pRenderer;
	}
}
