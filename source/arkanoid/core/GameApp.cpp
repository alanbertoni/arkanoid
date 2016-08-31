
#include "GameApp.h"
#include "GameLogic.h"
#include "../audio/AudioManager.h"
#include "../graphics/RendererFactory.h"
#include "../log/LogManager.h"
#include "../script/LuaManager.h"
#include "../event/EventManager.h"

#include <iostream>
#include <sstream>
#include <tinyxml2.h>

namespace
{
	LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		core::AppMessage l_oEvent{ hwnd , msg, wParam, lParam };
		if (core::GameApp::GetSingleton().ProcessAppMessage(l_oEvent))
		{
			return 0;
		}
		return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	static const char* CONFIG_FILENAME = "data/config/config.xml";
}

core::GameApp::GameApp()
	: Singleton{}
	, m_oTime{}
	, m_oHWnd{ NULL }
	, m_szWindowTitle{ "GameApp Window" }
	, m_iWindowHeight{ 600 }
	, m_iWindowWidth{ 800 }
	, m_iTargetFPS{ 60 }
	, m_bExit{ false }
	, m_bIsPaused{ false }
	, m_bIsResizing{ false }
	, m_pRenderer{ nullptr }
{
}

core::GameApp::~GameApp()
{
	Close();
}

void core::GameApp::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		new GameApp{};
	}
}

bool core::GameApp::Init()
{
	SetupFromXML(CONFIG_FILENAME);
	logger::LogManager::CreateInstance(CONFIG_FILENAME);
	script::LuaManager::CreateInstance();
	event::EventManager::CreateInstance();
	audio::AudioManager::CreateInstance();
	if (!audio::AudioManager::GetSingleton().Init())
	{
		LOG_ERROR("Failed to initialize the AudioManager!");
		return false;
	}


	if (!InitWindow())
	{
		LOG_ERROR("Failed to initialize the Main Window!");
		return false;
	}

	m_pRenderer = graphics::RendererFactory::CreateRenderer(CONFIG_FILENAME, m_oHWnd);
	if (m_pRenderer == nullptr)
	{
		LOG_ERROR("Failed to create the Renderer!");
		return false;
	}

	GameLogic::CreateInstance();
	if (!GameLogic::GetSingleton().Init(CONFIG_FILENAME))
	{
		LOG_ERROR("Failed to initialize the GameLogic");
		return false;
	}

	return true;
}

void core::GameApp::Run()
{
	m_oTime.Reset();

	float l_fTargetDeltaTime = 1.0f / static_cast<float>(m_iTargetFPS);
	float l_fElapsedTime = 0.0f;
	MSG msg = { 0 };
	while (msg.message != WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if (!IsPaused())
			{
				m_oTime.Tick();
				l_fElapsedTime += m_oTime.DeltaTime();

				while (l_fElapsedTime >= l_fTargetDeltaTime)
				{
					Update(l_fTargetDeltaTime);
					Render();
					l_fElapsedTime -= l_fTargetDeltaTime;
				}
				ComputeFrameStats();
			}
		}
	}

}

void core::GameApp::Close()
{
	GameLogic::Release();
	SAFE_DELETE(m_pRenderer);
	DestroyWindow(m_oHWnd);
	audio::AudioManager::Release();
	event::EventManager::Release();
	script::LuaManager::Release();
	logger::LogManager::Release();
}

void core::GameApp::Update(float fDeltaTime) const
{
	GameLogic::GetSingleton().Update(fDeltaTime);
}

void core::GameApp::Render() const
{
	m_pRenderer->PreRender();
	GameLogic::GetSingleton().Render();
	m_pRenderer->PostRender();
}

void core::GameApp::OnExit()
{
	m_bExit = true;
}

void core::GameApp::OnResize() const
{
	if (m_pRenderer)
	{
		m_pRenderer->OnResize(m_iWindowWidth, m_iWindowHeight);
	}

	event::EventPublisher<Dimension> l_oOnResizeEvent;
	l_oOnResizeEvent.SetPath("Window/Resize");
	Dimension l_oResizeDim{ static_cast<float>(m_iWindowWidth), static_cast<float>(m_iWindowHeight) };
	l_oOnResizeEvent.Raise(l_oResizeDim);
}

void core::GameApp::OnPause()
{
	m_bIsPaused = true;
	m_oTime.Stop();
}

void core::GameApp::OnUnPause()
{
	m_bIsPaused = false;
	m_oTime.Start();
}

void core::GameApp::SetupFromXML(const char * szConfigFilename)
{
	assert(szConfigFilename && strlen(szConfigFilename) > 0);
	tinyxml2::XMLDocument l_oConfigXMLDoc;
	l_oConfigXMLDoc.LoadFile(szConfigFilename);
	if (!l_oConfigXMLDoc.Error())
	{
		const tinyxml2::XMLElement* l_pRootElem = l_oConfigXMLDoc.RootElement();
		if (!l_pRootElem)
			return;

		const tinyxml2::XMLElement* l_pGraphicsElem = l_pRootElem->FirstChildElement("Graphics");
		if (l_pGraphicsElem)
		{
			int l_iTargetFPS = l_pGraphicsElem->IntAttribute("targetFPS");
			if (l_iTargetFPS)
				m_iTargetFPS = l_iTargetFPS;

			int l_iWindowWidth = l_pGraphicsElem->IntAttribute("width");
			if (l_iWindowWidth)
				m_iWindowWidth = l_iWindowWidth;

			int l_iWindowHeight = l_pGraphicsElem->IntAttribute("height");
			if (l_iWindowHeight)
				m_iWindowHeight = l_iWindowHeight;

			m_szWindowTitle = l_pGraphicsElem->Attribute("windowTitle");
		}
	}
}

bool core::GameApp::InitWindow()
{
	std::string _sMainWindowClassName{ "WndClassName" };
	HINSTANCE _oHAppInst = GetModuleHandle(0);

	WNDCLASS _oWindowClass;
	_oWindowClass.style = CS_HREDRAW | CS_VREDRAW;
	_oWindowClass.lpfnWndProc = MainWndProc;
	_oWindowClass.cbClsExtra = 0;
	_oWindowClass.cbWndExtra = 0;
	_oWindowClass.hInstance = _oHAppInst;
	_oWindowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	_oWindowClass.hCursor = LoadCursor(0, IDC_ARROW);
	_oWindowClass.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	_oWindowClass.lpszMenuName = 0;
	_oWindowClass.lpszClassName = _sMainWindowClassName.c_str();

	if (!RegisterClass(&_oWindowClass))
	{
		LOG_ERROR("Failed to register the WindowClass!");
		return false;
	}
	// Compute window rectangle dimensions based on requested client area dimensions.
	RECT _oRect = { 0, 0, m_iWindowWidth, m_iWindowHeight };
	AdjustWindowRect(&_oRect, WS_OVERLAPPEDWINDOW, false);
	int _iWidth = _oRect.right - _oRect.left;
	int _iHeight = _oRect.bottom - _oRect.top;

	m_oHWnd = CreateWindow(_sMainWindowClassName.c_str(),
		m_szWindowTitle.c_str(),
		WS_OVERLAPPEDWINDOW,
		0,
		0,
		_iWidth,
		_iHeight,
		0,
		0,
		_oHAppInst,
		0);

	if (!m_oHWnd)
	{
		LOG_ERROR("Failed to create the Main Window!");
		return false;
	}

	ShowWindow(m_oHWnd, SW_SHOW);
	UpdateWindow(m_oHWnd);

	return true;
}

void core::GameApp::ComputeFrameStats() const
{
	static int _iFrameCnt = 0;
	static float _fTimeElapsed = 0.0f;

	_iFrameCnt++;

	// Compute averages over one second period.
	if ((m_oTime.TotalTime() - _fTimeElapsed) >= 1.0f)
	{
		float _fFps = static_cast<float>(_iFrameCnt);
		float _fMspf = 1000.0f / _fFps;

		std::ostringstream _oOutSS;
		_oOutSS.precision(6);
		_oOutSS << m_szWindowTitle << "  " << "FPS: " << _fFps << "  " << "Frame Time: " << _fMspf << " (ms)";
		SetWindowText(m_oHWnd, _oOutSS.str().c_str());

		// Reset for next average.
		_iFrameCnt = 0;
		_fTimeElapsed += 1.0f;
	}
}

bool core::GameApp::ProcessAppMessage(const AppMessage & oEvent)
{
	switch (oEvent.m_oMessageId)
	{
	case WM_DESTROY:
	{
		OnExit();
		PostQuitMessage(0);
		return true;
	}
	case WM_ACTIVATE:
	{
		if (LOWORD(oEvent.m_oParam) == WA_INACTIVE)
		{
			OnPause();
		}
		else
		{
			OnUnPause();
		}
		return true;
	}
	case WM_ENTERSIZEMOVE:
	{
		OnPause();
		m_bIsResizing = true;
		return true;
	}
	case WM_EXITSIZEMOVE:
	{
		OnUnPause();
		m_bIsResizing = false;
		OnResize();
		return true;
	}
	case WM_SIZE:
	{
		m_iWindowWidth = LOWORD(oEvent.m_oParamExt);
		m_iWindowHeight = HIWORD(oEvent.m_oParamExt);

		if (oEvent.m_oParam == SIZE_MINIMIZED)
		{
			OnPause();
		}
		else if (oEvent.m_oParam == SIZE_MAXIMIZED)
		{
			OnUnPause();
			OnResize();
		}
		else if (oEvent.m_oParam == SIZE_RESTORED)
		{
			if (!m_bIsResizing)
			{
				OnUnPause();
				OnResize();
			}
		}

		return true;
	}
	}

	if (GameLogic::GetSingletonPtr())
	{
		return GameLogic::GetSingleton().ProcessAppMessage(oEvent);
	}

	return false;
}
