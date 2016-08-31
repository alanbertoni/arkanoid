
#include "AudioListenerComponent.h"
#include "Actor.h"
#include "../audio/OpenALUtilities.h"
#include "../log/LogManager.h"
#include "../script/LuaUtilities.h"

const char* actor::AudioListenerComponent::sk_szTypeName = "AudioListenerComponent";
const char* actor::AudioListenerComponent::sk_szMetatableName = "AudioListenerComponentMetatable";

actor::AudioListenerComponent::AudioListenerComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_vDirection{ 0.0f, 0.0f, 1.0f }
	, m_vPosition{ 0.0f, 0.0f, 0.0f }
	, m_vVelocity{ 0.0f, 0.0f, 0.0f }
	, m_vUp{ 0.0f, 1.0f, 0.0f }
	, m_fVolume{ 1.0f }
{
	CreateLuaObject();
}

void actor::AudioListenerComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("IsMute", (AudioListenerComponent*)0, &AudioListenerComponent::IsMute);
	l_oMetaTable.RegisterObjectDirect("ToggleMute", (AudioListenerComponent*)0, &AudioListenerComponent::ToggleMute);
	l_oMetaTable.RegisterObjectDirect("GetVolume", (AudioListenerComponent*)0, &AudioListenerComponent::GetVolume);
	l_oMetaTable.RegisterObjectDirect("SetVolume", (AudioListenerComponent*)0, &AudioListenerComponent::SetVolume);
	l_oMetaTable.RegisterObjectDirect("GetPosition", (AudioListenerComponent*)0, &AudioListenerComponent::GetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("SetPosition", (AudioListenerComponent*)0, &AudioListenerComponent::SetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("GetVelocity", (AudioListenerComponent*)0, &AudioListenerComponent::GetVelocityFromScript);
	l_oMetaTable.RegisterObjectDirect("SetVelocity", (AudioListenerComponent*)0, &AudioListenerComponent::SetVelocityFromScript);
	l_oMetaTable.RegisterObjectDirect("GetDirection", (AudioListenerComponent*)0, &AudioListenerComponent::GetDirectionFromScript);
	l_oMetaTable.RegisterObjectDirect("SetDirection", (AudioListenerComponent*)0, &AudioListenerComponent::SetDirectionFromScript);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::AudioListenerComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<AudioListenerComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::AudioListenerComponent::Init(const tinyxml2::XMLElement * pData)
{
	if (InitAudioListenerInfo(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init AudioListenerComponent info of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::AudioListenerComponent::PostInit()
{
	ApplyPosition();
	ApplyDirection();
	ApplyVelocity();
	ApplyVolume();
}

void actor::AudioListenerComponent::ToggleMute()
{
	float fVolume = 0.0f;
	if (IsMute())
	{
		fVolume = 1.0f;
	}
	SetVolume(fVolume);
}

void actor::AudioListenerComponent::SetVolume(float fVolume)
{
	if (fVolume < 0.0f)
	{
		m_fVolume = 0.0f;
	}
	else if (fVolume > 1.0f)
	{
		m_fVolume = 1.0f;
	}
	else
	{
		m_fVolume = fVolume;
	}

	ApplyVolume();
}

void actor::AudioListenerComponent::SetPosition(const Vec3 & i_oPosition)
{
	m_vPosition = i_oPosition;
	ApplyPosition();
}

void actor::AudioListenerComponent::SetVelocity(const Vec3 & i_oVelocity)
{
	m_vVelocity = i_oVelocity;
	ApplyVelocity();
}

void actor::AudioListenerComponent::SetDirection(const Vec3 & i_oDirection, const Vec3 & i_oUp)
{
	m_vDirection = i_oDirection;
	m_vUp = i_oUp;
	ApplyDirection();
}

void actor::AudioListenerComponent::ApplyVolume()
{
	alListenerf(AL_GAIN, m_fVolume);
	OpenAlUtilities::CheckOpenALError("ApplyVolume");
}

void actor::AudioListenerComponent::ApplyPosition()
{
	alListener3f(AL_POSITION, m_vPosition.x, m_vPosition.y, m_vPosition.z);
	OpenAlUtilities::CheckOpenALError("ApplyPosition");
}

void actor::AudioListenerComponent::ApplyVelocity()
{
	alListener3f(AL_VELOCITY, m_vVelocity.x, m_vVelocity.y, m_vVelocity.z);
	OpenAlUtilities::CheckOpenALError("ApplyVelocity");
}

void actor::AudioListenerComponent::ApplyDirection()
{
	ALfloat listenerOri[] = { m_vDirection.x, m_vDirection.y, m_vDirection.z, m_vUp.x, m_vUp.y, m_vUp.z };
	alListenerfv(AL_ORIENTATION, listenerOri);
	OpenAlUtilities::CheckOpenALError("ApplyDirection");
}

void actor::AudioListenerComponent::SetPositionFromScript(LuaPlus::LuaObject i_oVec3LuaObject)
{
	Vec3 _oVec3Position;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Position);
	SetPosition(_oVec3Position);
}

void actor::AudioListenerComponent::SetVelocityFromScript(LuaPlus::LuaObject i_oVec3LuaObject)
{
	Vec3 _oVec3Velocity;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Velocity);
	SetVelocity(_oVec3Velocity);
}

void actor::AudioListenerComponent::SetDirectionFromScript(LuaPlus::LuaObject i_oVec3LuaObject, LuaPlus::LuaObject i_oVec3UpLuaObject)
{
	Vec3 _oVec3Direction;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Direction);
	Vec3 _oVec3Up;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3UpLuaObject, _oVec3Up);
	SetDirection(_oVec3Direction, _oVec3Up);
}

LuaPlus::LuaObject actor::AudioListenerComponent::GetPositionFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetPosition(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

LuaPlus::LuaObject actor::AudioListenerComponent::GetVelocityFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetVelocity(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

LuaPlus::LuaObject actor::AudioListenerComponent::GetDirectionFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetDirection(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

bool actor::AudioListenerComponent::InitAudioListenerInfo(const tinyxml2::XMLElement * pData)
{
	const tinyxml2::XMLElement* pPosition = pData->FirstChildElement("Position");
	if (pPosition)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		x = pPosition->FloatAttribute("x");
		y = pPosition->FloatAttribute("y");
		z = pPosition->FloatAttribute("z");
		m_vPosition = Vec3(x, y, z);

		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent position successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pVelocity = pData->FirstChildElement("Velocity");
	if (pVelocity)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
		x = pVelocity->FloatAttribute("x");
		y = pVelocity->FloatAttribute("y");
		z = pVelocity->FloatAttribute("z");
		m_vVelocity = Vec3(x, y, z);

		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent velocity successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pDirection = pData->FirstChildElement("Direction");
	if (pDirection)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 1.0f;
		x = pDirection->FloatAttribute("x");
		y = pDirection->FloatAttribute("y");
		z = pDirection->FloatAttribute("z");
		m_vDirection = Vec3(x, y, z);

		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent direction successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pUp = pData->FirstChildElement("Up");
	if (pUp)
	{
		float x = 0.0f;
		float y = 1.0f;
		float z = 0.0f;
		x = pUp->FloatAttribute("x");
		y = pUp->FloatAttribute("y");
		z = pUp->FloatAttribute("z");
		m_vUp = Vec3(x, y, z);

		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent direction successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pVolume = pData->FirstChildElement("Volume");
	if (pVolume)
	{
		m_fVolume = pVolume->FloatAttribute("value");

		std::stringstream l_oSS;
		l_oSS << "AudioListenerComponent volume successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	return true;
}
