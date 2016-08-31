#pragma once

#include "../core/Common.h"

namespace graphics
{
	class IRenderer;

	class RendererFactory
	{
	public:
		static IRenderer* CreateRenderer(const char* szConfigFilename, core::WindowHandle oWindowHandle);
	};
}