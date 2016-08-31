
#include "TransformComponent.h"
#include "Actor.h"
#include "../log/LogManager.h"
#include "../script/LuaUtilities.h"

const char* actor::TransformComponent::sk_szTypeName = "TransformComponent";
const char* actor::TransformComponent::sk_szMetatableName = "TransformComponentMetatable";

actor::TransformComponent::TransformComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_oPosition{ 0.0f, 0.0f, 0.0f }
	, m_oRotation{ 0.0f, 0.0f, 0.0f, 1.0f }
	, m_oScale{ 1.0f, 1.0f, 1.0f }
	, m_oToWorldMatrix{}
	, m_oFromWorldMatrix{}
	, m_bIsChanged{ true }
{
	CreateLuaObject();
}

void actor::TransformComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("Translate", (TransformComponent*)0, &TransformComponent::TranslateFromScript);
	l_oMetaTable.RegisterObjectDirect("GetPosition", (TransformComponent*)0, &TransformComponent::GetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("SetPosition", (TransformComponent*)0, &TransformComponent::SetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("GetScale", (TransformComponent*)0, &TransformComponent::GetScaleFromScript);
	l_oMetaTable.RegisterObjectDirect("SetScale", (TransformComponent*)0, &TransformComponent::SetScaleFromScript);
	l_oMetaTable.RegisterObjectDirect("Rotate", (TransformComponent*)0, &TransformComponent::RotateFromScript);
	l_oMetaTable.RegisterObjectDirect("SetRotation", (TransformComponent*)0, &TransformComponent::SetRotationFromScript);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::TransformComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<TransformComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::TransformComponent::Init(const tinyxml2::XMLElement * pData)
{
	if (InitPositionInfo(pData) && InitRotationInfo(pData) && InitScaleInfo(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "TransformComponent info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init TransformComponent info of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

const Mat4x4& actor::TransformComponent::GetToWorldMatrix()
{
	if (m_bIsChanged)
	{
		UpdateWorldMatrices();		
	}

	return m_oToWorldMatrix;
}

const Mat4x4& actor::TransformComponent::GetFromWorldMatrix()
{
	if (m_bIsChanged)
	{
		UpdateWorldMatrices();
	}

	return m_oFromWorldMatrix;
}

void actor::TransformComponent::UpdateWorldMatrices()
{
	utilities::MathHelper::SetAffineTransformationMatrix(m_oToWorldMatrix, m_oPosition, m_oRotation, m_oScale);
	utilities::MathHelper::MatrixInverse(m_oFromWorldMatrix, m_oToWorldMatrix);
	m_bIsChanged = false;
}

void actor::TransformComponent::SetPosition(Vec3 oPosition)
{
	m_oPosition = oPosition;
	m_bIsChanged = true;
}

void actor::TransformComponent::Translate(float fX, float fY, float fZ)
{
	Vec3 l_oOffset{ fX, fY, fZ };
	Translate(l_oOffset);
}

void actor::TransformComponent::Translate(Vec3 oOffset)
{
	m_oPosition = m_oPosition + oOffset;
	m_bIsChanged = true;
}

void actor::TransformComponent::SetScale(Vec3 oScale)
{
	m_oScale = oScale;
	m_bIsChanged = true;
}

void actor::TransformComponent::SetRotation(const Vec3& oAxis, float fDegrees)
{
	utilities::MathHelper::SetQuaternionFromAxisAnge(m_oRotation, oAxis, fDegrees);
	m_bIsChanged = true;
}

void actor::TransformComponent::Rotate(const Vec3& oAxis, float fDegrees)
{
	Quat l_oRotQuat;
	utilities::MathHelper::SetQuaternionFromAxisAnge(l_oRotQuat, oAxis, fDegrees);
	utilities::MathHelper::MultiplyQuaternion(m_oRotation, m_oRotation, l_oRotQuat);
	m_bIsChanged = true;
}

LuaPlus::LuaObject actor::TransformComponent::GetScaleFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetScale(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

void actor::TransformComponent::SetScaleFromScript(float fScaleFactor)
{
	SetScale(Vec3{ fScaleFactor, fScaleFactor, fScaleFactor });
}

LuaPlus::LuaObject actor::TransformComponent::GetPositionFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetPosition(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

void actor::TransformComponent::SetPositionFromScript(LuaPlus::LuaObject oVec3LuaObject)
{
	Vec3 _oVec3Position;
	script::LuaUtilities::ConvertLuaObjectToVec3(oVec3LuaObject, _oVec3Position);
	SetPosition(_oVec3Position);
}

void actor::TransformComponent::TranslateFromScript(float fX, float fY, float fZ)
{
	Vec3 l_oOffset{ fX, fY, fZ };
	Translate(l_oOffset);
}

void actor::TransformComponent::SetRotationFromScript(LuaPlus::LuaObject oVec3LuaObject, float fDegrees)
{
	Vec3 l_oRotAxis;
	script::LuaUtilities::ConvertLuaObjectToVec3(oVec3LuaObject, l_oRotAxis);
	SetRotation(l_oRotAxis, fDegrees);
}

void actor::TransformComponent::RotateFromScript(LuaPlus::LuaObject oVec3LuaObject, float fDegrees)
{
	Vec3 l_oRotAxis;
	script::LuaUtilities::ConvertLuaObjectToVec3(oVec3LuaObject, l_oRotAxis);
	Rotate(l_oRotAxis, fDegrees);
}

bool actor::TransformComponent::InitPositionInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pPositionElem = pData->FirstChildElement("Position");
	if (l_pPositionElem)
	{
		l_pPositionElem->QueryFloatAttribute("x", &m_oPosition.x);

		l_pPositionElem->QueryFloatAttribute("y", &m_oPosition.y);

		l_pPositionElem->QueryFloatAttribute("z", &m_oPosition.z);

		std::stringstream l_oSS;
		l_oSS << "Position info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Position element in TransformComponent of Actor " << GetOwner()->GetType() << ", therefore set to default (0,0,0)";
		LOG("ACTOR", l_oSS.str());
	}

	return true;
}

bool actor::TransformComponent::InitScaleInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pScaleElem = pData->FirstChildElement("Scale");
	if (l_pScaleElem)
	{
		float l_fScaleFactor = 1.0f;
		l_pScaleElem->QueryFloatAttribute("factor", &l_fScaleFactor);

		SetScale(Vec3{ l_fScaleFactor, l_fScaleFactor, l_fScaleFactor });

		std::stringstream l_oSS;
		l_oSS << "Scale info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Scale element in TransformComponent of Actor " << GetOwner()->GetType() << ", therefore set to default (1,1,1)";
		LOG("ACTOR", l_oSS.str());
	}

	return true;
}

bool actor::TransformComponent::InitRotationInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* l_pRotElem = pData->FirstChildElement("YawPitchRoll");
	if (l_pRotElem)
	{
		float fPitch = 0.0f;
		l_pRotElem->QueryFloatAttribute("x", &fPitch);
		fPitch = utilities::MathHelper::ConvertFromDegreesToRadians(fPitch);

		float fYaw = 0.0f;
		l_pRotElem->QueryFloatAttribute("y", &fYaw);
		fYaw = utilities::MathHelper::ConvertFromDegreesToRadians(fYaw);

		float fRoll = 0.0f;
		l_pRotElem->QueryFloatAttribute("z", &fRoll);
		fRoll = utilities::MathHelper::ConvertFromDegreesToRadians(fRoll);

		utilities::MathHelper::SetQuaternionFromYawPitchRoll(m_oRotation, fPitch, fYaw, fRoll);

		std::stringstream l_oSS;
		l_oSS << "Rotation info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Missing Rotation element in TransformComponent of Actor " << GetOwner()->GetType() << ", therefore set to default";
		LOG("ACTOR", l_oSS.str());
	}

	return true;
}
