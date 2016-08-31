
#include "CameraComponent.h"
#include "Actor.h"
#include "../event/EventPublisher.h"
#include "../log/LogManager.h"

const char* actor::CameraComponent::sk_szTypeName = "CameraComponent";
const char* actor::CameraComponent::sk_szMetatableName = "CameraComponentMetatable";

actor::CameraComponent::CameraComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_eCameraType{ ORTHOGRAPHIC }
	, m_fNearPlane{ 0.1f }
	, m_fFarPlane{ 1.0f }
	, m_fViewWidth{ 2.0f }
	, m_fViewHeight{ 2.0f }
	, m_fFovH{ utilities::MathHelper::ConvertFromDegreesToRadians(60.0f) }
	, m_fAspectRatio{ 1.0f }
	, m_fScreenHeight{ 1.0f }
	, m_fScreenWidth{ 1.0f }
	, m_bIsChanged{ true }
	, m_oOnResizeEvent{}
{
	CreateLuaObject();
	m_oOnResizeEvent.Subscribe(this, &CameraComponent::OnResize, "Window/Resize");
}

void actor::CameraComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("IsCameraChanged", (CameraComponent*)0, &CameraComponent::IsChanged);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::CameraComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<CameraComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::CameraComponent::Init(const tinyxml2::XMLElement* pData)
{
	if (InitOrthographicInfo(pData) || InitPerspectiveInfo(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "CameraComponent info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init CameraComponent info of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::CameraComponent::PostInit()
{
	event::EventPublisher<ActorId> l_oCameraInitEvent;
	l_oCameraInitEvent.SetPath("Init/ActorComponent/CameraComponent");
	l_oCameraInitEvent.Raise(GetOwner()->GetId());
}

void actor::CameraComponent::OnResize(const Dimension& oDimension)
{
	m_fScreenWidth = oDimension.m_fWidth;
	m_fScreenHeight = oDimension.m_fHeight;
	SetAspectRatio(oDimension.m_fWidth / oDimension.m_fHeight);
}

const Mat4x4& actor::CameraComponent::GetProjectionMatrix()
{
	if (m_bIsChanged)
	{
		UpdateProjectionMatrix();
	}
	return m_oProjectionMatrix;
}

void actor::CameraComponent::UpdateProjectionMatrix()
{
	switch (m_eCameraType)
	{
	case actor::CameraComponent::ORTHOGRAPHIC:
	{
		utilities::MathHelper::SetOrthographicProjectionMatrix(m_oProjectionMatrix, m_fViewWidth, m_fViewHeight, m_fNearPlane, m_fFarPlane);
		break;
	}
	case actor::CameraComponent::PERSPECTIVE:
	{
		utilities::MathHelper::SetPerspectiveProjectionMatrix(m_oProjectionMatrix, m_fFovH, m_fAspectRatio, m_fNearPlane, m_fFarPlane);
		break;
	}
	}

	m_bIsChanged = false;
}

bool actor::CameraComponent::InitOrthographicInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pOrtoElem = pData->FirstChildElement("Orthographic");
	if (l_pOrtoElem)
	{
		m_eCameraType = ORTHOGRAPHIC;

		tinyxml2::XMLError l_oError = l_pOrtoElem->QueryFloatAttribute("viewWidth", &m_fViewWidth);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set viewWidth attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		l_oError = l_pOrtoElem->QueryFloatAttribute("viewHeight", &m_fViewHeight);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set viewHeight attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		l_oError = l_pOrtoElem->QueryFloatAttribute("nearPlane", &m_fNearPlane);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set nearPlane attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		l_oError = l_pOrtoElem->QueryFloatAttribute("farPlane", &m_fFarPlane);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set farPlane attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		std::stringstream l_oSS;
		l_oSS << "Orthographic info of CameraComponent successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}

	return false;
}

bool actor::CameraComponent::InitPerspectiveInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pOrtoElem = pData->FirstChildElement("Perspective");
	if (l_pOrtoElem)
	{
		m_eCameraType = PERSPECTIVE;

		float l_fFovHDegrees = 60.0f;

		tinyxml2::XMLError l_oError = l_pOrtoElem->QueryFloatAttribute("fovH", &l_fFovHDegrees);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set fovH attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		m_fFovH = utilities::MathHelper::ConvertFromDegreesToRadians(l_fFovHDegrees);

		l_oError = l_pOrtoElem->QueryFloatAttribute("nearPlane", &m_fNearPlane);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set nearPlane attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		l_oError = l_pOrtoElem->QueryFloatAttribute("farPlane", &m_fFarPlane);
		if (l_oError != tinyxml2::XMLError::XML_NO_ERROR)
		{
			std::stringstream l_oSS;
			l_oSS << "Error while trying to set farPlane attribute of CameraComponent for Actor " << GetOwner()->GetName();
			LOG_ERROR(l_oSS.str());
			return false;
		}

		std::stringstream l_oSS;
		l_oSS << "Orthographic info of CameraComponent successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}

	return false;
}
