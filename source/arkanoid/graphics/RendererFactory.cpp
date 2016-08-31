
#include "D3D11Renderer.h"
#include "RendererFactory.h"

#include <assert.h>
#include <string>
#include <tinyxml2.h>
using namespace graphics;

IRenderer* RendererFactory::CreateRenderer(const char* szConfigFilename, core::WindowHandle oWindowHandle)
{
	assert(szConfigFilename && strlen(szConfigFilename) > 0);
	// set default value for the renderer type
	std::string l_sRendererType{ "D3D11" };

	// parse the configuration file to determine the IRenderer to create
	tinyxml2::XMLDocument l_oConfigXMLDoc;
	l_oConfigXMLDoc.LoadFile(szConfigFilename);
	if (!l_oConfigXMLDoc.Error())
	{
		const tinyxml2::XMLElement* l_pRootElem = l_oConfigXMLDoc.RootElement();
		if (l_pRootElem)
		{
			const tinyxml2::XMLElement* l_pGraphicsElem = l_pRootElem->FirstChildElement("Graphics");
			if (l_pGraphicsElem)
			{
				l_sRendererType = l_pGraphicsElem->Attribute("renderer");
			}
		}
	}

	if (l_sRendererType == "D3D11")
	{
		IRenderer* l_pRenderer = new D3D11Renderer{ oWindowHandle };
		l_pRenderer->Init();
		return l_pRenderer;
	}

	assert(0 && "FAILED TO CREATE THE RENDERER");
	return nullptr;
}
