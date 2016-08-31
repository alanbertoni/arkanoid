
#include "RendererComponent.h"
#include "Actor.h"
#include "../graphics/GeometryGenerator.h"
#include "../graphics/UnlitMaterial.h"
#include "../log/LogManager.h"
#include "../scene/Scene.h"
#include "../script/LuaUtilities.h"

const char* actor::RendererComponent::sk_szTypeName = "RendererComponent";
const char* actor::RendererComponent::sk_szMetatableName = "RendererComponentMetatable";

actor::RendererComponent::RendererComponent(Actor * pOwner)
	: ActorComponent{ pOwner }
	, m_eMeshShape{ graphics::EMeshShape::QUAD }
	, m_uiMeshId{}
	, m_pMaterial{ nullptr }
	, m_oDiffuseColor{ 1.0f, 1.0f, 1.0f, 1.0f }
	, m_szTextureFile{}
	, m_vTextureScaleAndOffset{ 0.0f, 0.0f, 1.0f, 1.0f}
{
	CreateLuaObject();
}

actor::RendererComponent::~RendererComponent()
{
	delete m_pMaterial;
}

void actor::RendererComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("SetDiffuseColor", (RendererComponent*)0, &RendererComponent::SetDiffuseColorFromScript);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::RendererComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<RendererComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::RendererComponent::Init(const tinyxml2::XMLElement* pData)
{
	if (InitMeshInfo(pData) && InitMaterialInfo(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "RendererComponent of Actor " << GetOwner()->GetName() << " successfully initialized";
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init RendererComponent of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::RendererComponent::InitForRendering(scene::Scene& oScene, const scene::SceneNode& oSceneNode)
{
	graphics::MeshData& l_oMeshData = graphics::GeometryGenerator::GetSingleton().CreateMeshData(m_eMeshShape);
	m_uiMeshId = l_oMeshData.m_uiId;
	oScene.GetRenderer().CreateMesh(l_oMeshData);

	m_pMaterial->SetupForRendering(oScene.GetRenderer(), *this);
}

void actor::RendererComponent::Render(scene::Scene& oScene, scene::SceneNode& oSceneNode)
{
	if (!GetOwner()->IsEnable() || !IsEnable())
	{
		return;
	}
	m_pMaterial->UpdateMaterial(oScene, oSceneNode, *this);
	m_pMaterial->BindMaterial(oScene.GetRenderer());
	oScene.GetRenderer().RenderMesh(m_uiMeshId);
}

void actor::RendererComponent::ReleaseRenderingResources(const scene::Scene & oScene, const scene::SceneNode & oSceneNode)
{
	m_pMaterial->ReleaseMaterial(oScene.GetRenderer());
	oScene.GetRenderer().ReleaseMesh(m_uiMeshId);
	graphics::GeometryGenerator::GetSingleton().DestroyMeshData(m_uiMeshId);
}

void actor::RendererComponent::SetDiffuseColor(const Color& oColor)
{
	m_oDiffuseColor = oColor;
}

void actor::RendererComponent::SetDiffuseColorFromScript(LuaPlus::LuaObject oColorLuaObject)
{
	Color l_oColor;
	script::LuaUtilities::ConvertLuaObjectToColor(oColorLuaObject, l_oColor);
	SetDiffuseColor(l_oColor);
}

bool actor::RendererComponent::InitMeshInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pMeshElem = pData->FirstChildElement("Mesh");
	if (l_pMeshElem)
	{
		const char* l_szMeshShape = l_pMeshElem->Attribute("shape");
		if (l_szMeshShape)
		{
			if (_strcmpi(l_szMeshShape, "FULLSCREENQUAD") == 0)
			{
				m_eMeshShape = graphics::EMeshShape::FULLSCREENQUAD;
			}
			else if (_strcmpi(l_szMeshShape, "QUAD") == 0)
			{
				m_eMeshShape = graphics::EMeshShape::QUAD;
			}
			else if (_strcmpi(l_szMeshShape, "QUAD2W1H") == 0)
			{
				m_eMeshShape = graphics::EMeshShape::QUAD2W1H;
			}
			else
			{
				std::stringstream l_oSS;
				l_oSS << "Unknown Mesh shape attribute for RendererComponent of Actor " << GetOwner()->GetName();
				LOG_ERROR(l_oSS.str());
				return false;
			}

			std::stringstream l_oSS;
			l_oSS << "Mesh info successfully initialized for Actor " << GetOwner()->GetName();
			LOG("ACTOR", l_oSS.str());
			return true;
		}
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Mesh element in RendererComponent of Actor " << GetOwner()->GetType();
		LOG("ACTOR", l_oSS.str());
	}

	return false;
}

bool actor::RendererComponent::InitMaterialInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pMaterialElem = pData->FirstChildElement("Material");
	if (l_pMaterialElem)
	{
		const char* l_szMaterialType = l_pMaterialElem->Attribute("type");
		if (l_szMaterialType)
		{
			if (_strcmpi(l_szMaterialType, "UnlitTextured") == 0)
			{
				m_pMaterial = new graphics::UnlitMaterial{};
			}
			else
			{
				std::stringstream l_oSS;
				l_oSS << "Unknown Material Type attribute " << l_szMaterialType << " in RendererComponent of Actor " << GetOwner()->GetType();
				LOG("ACTOR", l_oSS.str());
				return false;
			}
		}
		else
		{
			std::stringstream l_oSS;
			l_oSS << "Missing Material Type attribute in RendererComponent of Actor " << GetOwner()->GetType();
			LOG("ACTOR", l_oSS.str());
			return false;
		}

		InitColorInfo(l_pMaterialElem);
		InitTextureInfo(l_pMaterialElem);

		std::stringstream l_oSS;
		l_oSS << "Material info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Material element in RendererComponent of Actor " << GetOwner()->GetType();
		LOG("ACTOR", l_oSS.str());
		return false;
	}

	std::stringstream l_oSS;
	l_oSS << "Failed to init RendererComponent of Actor " << GetOwner()->GetType();
	LOG("ACTOR", l_oSS.str());
	return false;
}

bool actor::RendererComponent::InitColorInfo(const tinyxml2::XMLElement* pData)
{
	const tinyxml2::XMLElement* l_pColorElem = pData->FirstChildElement("Color");
	if (l_pColorElem)
	{
		const tinyxml2::XMLElement* l_pDiffuseColorElem = l_pColorElem->FirstChildElement("DiffuseColor");
		if (l_pDiffuseColorElem)
		{
			int l_iR = 255;
			l_pDiffuseColorElem->QueryIntAttribute("r", &l_iR);
			int l_iG = 255;
			l_pDiffuseColorElem->QueryIntAttribute("g", &l_iG);
			int l_iB = 255;
			l_pDiffuseColorElem->QueryIntAttribute("b", &l_iB);
			int l_iA = 255;
			l_pDiffuseColorElem->QueryIntAttribute("a", &l_iA);

			Color l_oInitColor;
			l_oInitColor.r = l_iR;
			l_oInitColor.g = l_iG;
			l_oInitColor.b = l_iB;
			l_oInitColor.a = l_iA;
			SetDiffuseColor(l_oInitColor);
		}
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Color element in RendererComponent of Actor " << GetOwner()->GetType();
		LOG("ACTOR", l_oSS.str());
		return false;
	}

	return true;
}

bool actor::RendererComponent::InitTextureInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pTextureElem = pData->FirstChildElement("Texture");
	if (l_pTextureElem)
	{
		//Texture WIDTH attribute
		float l_fTextureWidth = 1.0f;
		tinyxml2::XMLError l_oError = l_pTextureElem->QueryFloatAttribute("textureWidth", &l_fTextureWidth);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set textureWidth attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		//Texture HEIGHT attribute
		float l_fTextureHeight = 1.0f;
		l_oError = l_pTextureElem->QueryFloatAttribute("textureHeight", &l_fTextureHeight);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set textureHeight attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		//OFFSET X attribute
		float l_fOffsetX = 0.0f;
		l_oError = l_pTextureElem->QueryFloatAttribute("offsetX", &l_fOffsetX);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set offsetX attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}
		m_vTextureScaleAndOffset.x = l_fOffsetX / l_fTextureWidth;

		//OFFSET Y attribute
		float l_fOffsetY = 0.0f;
		l_oError = l_pTextureElem->QueryFloatAttribute("offsetY", &l_fOffsetY);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set offsetY attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}
		m_vTextureScaleAndOffset.y = l_fOffsetY / l_fTextureHeight;

		//WIDTH attribute
		float l_fWidth = 1.0f;
		l_oError = l_pTextureElem->QueryFloatAttribute("width", &l_fWidth);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set width attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}
		m_vTextureScaleAndOffset.z = l_fWidth / l_fTextureWidth;

		//HEIGHT attribute
		float l_fHeight = 1.0f;
		l_oError = l_pTextureElem->QueryFloatAttribute("height", &l_fHeight);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set height attribute of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}
		m_vTextureScaleAndOffset.w = l_fHeight / l_fTextureHeight;

		const char* l_szTextureFilename = l_pTextureElem->Attribute("filename");
		if (l_szTextureFilename)
		{
			m_szTextureFile = l_szTextureFilename;
			std::stringstream l_oSS;
			l_oSS << "Texture element info successfully init for RendererComponent of Actor " << GetOwner()->GetType();
			l_oSS << " texture filename is " << m_szTextureFile;
			LOG("ACTOR", l_oSS.str());
			return true;
		}
		else
		{
			std::stringstream l_oSS;
			l_oSS << "Missing filename attribute in Texture element of RendererComponent's Texture element for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "No Texture element in RendererComponent of Actor " << GetOwner()->GetType();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}
