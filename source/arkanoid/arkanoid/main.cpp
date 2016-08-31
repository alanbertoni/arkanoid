
#include "../core/GameApp.h"

int main()
{
	using namespace core;
	GameApp::CreateInstance();

	if (GameApp::GetSingleton().Init())
	{
		GameApp::GetSingleton().Run();
	}

	GameApp::Release();
	return 0;
}
