
#include "AudioSourceComponent.h"
#include "Actor.h"
#include "../audio/AudioBuffer.h"
#include "../audio/AudioManager.h"
#include "../event/EventPublisher.h"
#include "../log/LogManager.h"
#include "../script/LuaUtilities.h"

const char* actor::AudioSourceComponent::sk_szTypeName = "AudioSourceComponent";
const char* actor::AudioSourceComponent::sk_szMetatableName = "AudioSourceComponentMetatable";

actor::AudioSourceComponent::AudioSourceComponent(Actor* pOwner)
	: ActorComponent{ pOwner }
	, m_uiBuffer{ 0 }
	, m_uiSource{ 0 }
	, m_vDirection{ 0.0f, 0.0f, -1.0f }
	, m_vPosition{ 0.0f, 0.0f, 0.0f }
	, m_vVelocity{ 0.0f, 0.0f, 0.0f }
	, m_bIsLooping{ false }
	, m_fPitch{ 1.0f }
	, m_fVolume{ 1.0f }
	, m_szAudioFilename{}
	, m_bIsInit{ false }
{
	CreateLuaObject();
}

actor::AudioSourceComponent::~AudioSourceComponent()
{
	Stop();
	DeleteSource();
	DeleteBuffer();

	std::string pEventName("Audio/Release/AudioSourceComponent");
	event::EventPublisher<const char*> oEventPublisher;
	oEventPublisher.SetPath(pEventName.c_str());
	oEventPublisher.Raise(m_szAudioFilename.c_str());
}

void actor::AudioSourceComponent::RegisterScriptFunction()
{
	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().CreateTable(sk_szMetatableName);
	l_oMetaTable.SetObject("__index", l_oMetaTable);
	l_oMetaTable.RegisterObjectDirect("Play", (AudioSourceComponent*)0, &AudioSourceComponent::Play);
	l_oMetaTable.RegisterObjectDirect("Pause", (AudioSourceComponent*)0, &AudioSourceComponent::Pause);
	l_oMetaTable.RegisterObjectDirect("Stop", (AudioSourceComponent*)0, &AudioSourceComponent::Stop);
	l_oMetaTable.RegisterObjectDirect("Rewind", (AudioSourceComponent*)0, &AudioSourceComponent::Rewind);
	l_oMetaTable.RegisterObjectDirect("IsPlaying", (AudioSourceComponent*)0, &AudioSourceComponent::IsPlaying);
	l_oMetaTable.RegisterObjectDirect("IsPaused", (AudioSourceComponent*)0, &AudioSourceComponent::IsPaused);
	l_oMetaTable.RegisterObjectDirect("IsStopped", (AudioSourceComponent*)0, &AudioSourceComponent::IsStopped);
	l_oMetaTable.RegisterObjectDirect("IsLooping", (AudioSourceComponent*)0, &AudioSourceComponent::IsLooping);
	l_oMetaTable.RegisterObjectDirect("SetLoop", (AudioSourceComponent*)0, &AudioSourceComponent::SetLoop);
	l_oMetaTable.RegisterObjectDirect("GetVolume", (AudioSourceComponent*)0, &AudioSourceComponent::GetVolume);
	l_oMetaTable.RegisterObjectDirect("SetVolume", (AudioSourceComponent*)0, &AudioSourceComponent::SetVolume);
	l_oMetaTable.RegisterObjectDirect("GetPitch", (AudioSourceComponent*)0, &AudioSourceComponent::GetPitch);
	l_oMetaTable.RegisterObjectDirect("SetPitch", (AudioSourceComponent*)0, &AudioSourceComponent::SetPitch);
	l_oMetaTable.RegisterObjectDirect("GetPosition", (AudioSourceComponent*)0, &AudioSourceComponent::GetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("SetPosition", (AudioSourceComponent*)0, &AudioSourceComponent::SetPositionFromScript);
	l_oMetaTable.RegisterObjectDirect("GetVelocity", (AudioSourceComponent*)0, &AudioSourceComponent::GetVelocityFromScript);
	l_oMetaTable.RegisterObjectDirect("SetVelocity", (AudioSourceComponent*)0, &AudioSourceComponent::SetVelocityFromScript);
	l_oMetaTable.RegisterObjectDirect("GetDirection", (AudioSourceComponent*)0, &AudioSourceComponent::GetDirectionFromScript);
	l_oMetaTable.RegisterObjectDirect("SetDirection", (AudioSourceComponent*)0, &AudioSourceComponent::SetDirectionFromScript);

	l_oMetaTable.SetMetaTable(script::LuaManager::GetSingleton().GetGlobalVars().GetByName(ActorComponent::sk_szMetatableName));
}

void actor::AudioSourceComponent::CreateLuaObject()
{
	m_oLuaObject.AssignNewTable(script::LuaManager::GetSingleton().GetLuaState());

	LuaPlus::LuaObject l_oMetaTable = script::LuaManager::GetSingleton().GetGlobalVars().GetByName(sk_szMetatableName);

	m_oLuaObject.SetLightUserData("__object", const_cast<AudioSourceComponent*>(this));
	m_oLuaObject.SetMetaTable(l_oMetaTable);
}

bool actor::AudioSourceComponent::Init(const tinyxml2::XMLElement * pData)
{
	if (InitAudioSourceInfo(pData))
	{
		std::stringstream l_oSS;
		l_oSS << "AudioSourceComponent info successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return true;
	}
	else
	{
		std::stringstream l_oSS;
		l_oSS << "Failed to init AudioSourceComponent info of Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
		return false;
	}
}

void actor::AudioSourceComponent::PostInit()
{
	assert(!m_szAudioFilename.empty());

	// Generate an AL Buffer
	alGenBuffers(1, &m_uiBuffer);
	OpenAlUtilities::CheckOpenALError("alGenBuffers");

	// Load audio file into OpenAL Buffer
	const audio::AudioBuffer& oAudioBuffer = audio::AudioManager::GetSingleton().GetAudioBuffer(m_szAudioFilename.c_str());

	alBufferData(m_uiBuffer, oAudioBuffer.GetBufferFormat(), oAudioBuffer.GetData(), oAudioBuffer.GetDataSize(), oAudioBuffer.GetFrequency());
	OpenAlUtilities::CheckOpenALError("alBufferData");

	// Generate a Source to playback the Buffer
	alGenSources(1, &m_uiSource);
	OpenAlUtilities::CheckOpenALError("alGenSources");

	// Attach Source to Buffer
	alSourcei(m_uiSource, AL_BUFFER, m_uiBuffer);
	OpenAlUtilities::CheckOpenALError("AttachSource");

	if (alIsSource(m_uiSource) && alIsBuffer(m_uiBuffer))
	{
		m_bIsInit = true;
	}

	ApplyPosition();
	ApplyDirection();
	ApplyVelocity();
	ApplyPitch();
}

void actor::AudioSourceComponent::Play(float fVolume, bool bLoop)
{
	assert(m_bIsInit);
	if (IsPlaying())
	{
		return;
	}

	SetVolume(fVolume);
	SetLoop(bLoop);

	alSourcePlay(m_uiSource);
	OpenAlUtilities::CheckOpenALError("Play");
}

void actor::AudioSourceComponent::Pause()
{
	assert(m_bIsInit);
	if (IsPaused())
	{
		return;
	}

	alSourcePause(m_uiSource);
	OpenAlUtilities::CheckOpenALError("Pause");
}

void actor::AudioSourceComponent::Stop()
{
	assert(m_bIsInit);
	if (IsStopped())
	{
		return;
	}

	alSourceStop(m_uiSource);
	OpenAlUtilities::CheckOpenALError("Stop");
}

void actor::AudioSourceComponent::Rewind()
{
	assert(m_bIsInit);
	alSourceRewind(m_uiSource);
	OpenAlUtilities::CheckOpenALError("Rewind");
}

void actor::AudioSourceComponent::SetVolume(float fVolume)
{
	assert(m_bIsInit);
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

	alSourcef(m_uiSource, AL_GAIN, m_fVolume);
	OpenAlUtilities::CheckOpenALError("SetVolume");
}

void actor::AudioSourceComponent::SetLoop(bool bLoop)
{
	assert(m_bIsInit);
	m_bIsLooping = bLoop;
	if (bLoop)
	{
		alSourcei(m_uiSource, AL_LOOPING, AL_TRUE);
	}
	else
	{
		alSourcei(m_uiSource, AL_LOOPING, AL_FALSE);
	}

	OpenAlUtilities::CheckOpenALError("SetLoop");
}

void actor::AudioSourceComponent::SetPitch(float fPitch)
{
	if (fPitch < 0.0f)
	{
		m_fPitch = 0.1f;
	}
	else
	{
		m_fPitch = fPitch;
	}

	ApplyPitch();
}

void actor::AudioSourceComponent::SetPosition(const Vec3 & i_oPosition)
{
	m_vPosition = i_oPosition;
	ApplyPosition();
}

void actor::AudioSourceComponent::SetVelocity(const Vec3 & i_oVelocity)
{
	m_vVelocity = i_oVelocity;
	ApplyVelocity();
}

void actor::AudioSourceComponent::SetDirection(const Vec3 & i_oDirection)
{
	m_vDirection = i_oDirection;
	ApplyDirection();
}

void actor::AudioSourceComponent::ApplyPitch()
{
	assert(m_bIsInit);
	alSourcef(m_uiSource, AL_PITCH, m_fPitch);
	OpenAlUtilities::CheckOpenALError("ApplyPitch\t");
}

void actor::AudioSourceComponent::ApplyPosition()
{
	assert(m_bIsInit);
	alSource3f(m_uiSource, AL_POSITION, m_vPosition.x, m_vPosition.y, m_vPosition.z);
	OpenAlUtilities::CheckOpenALError("ApplyPosition\t");
}

void actor::AudioSourceComponent::ApplyVelocity()
{
	assert(m_bIsInit);
	alSource3f(m_uiSource, AL_VELOCITY, m_vVelocity.x, m_vVelocity.y, m_vVelocity.z);
	OpenAlUtilities::CheckOpenALError("ApplyVelocity\t");
}

void actor::AudioSourceComponent::ApplyDirection()
{
	assert(m_bIsInit);
	alSource3f(m_uiSource, AL_DIRECTION, m_vDirection.x, m_vDirection.y, m_vDirection.z);
	OpenAlUtilities::CheckOpenALError("ApplyDirection\t");
}

void actor::AudioSourceComponent::DeleteSource()
{
	if (alIsSource(m_uiSource))
	{
		alDeleteSources(1, &m_uiSource);
		OpenAlUtilities::CheckOpenALError("alDeleteSources\t");
	}
}

void actor::AudioSourceComponent::DeleteBuffer()
{
	if (alIsBuffer(m_uiBuffer))
	{
		alDeleteBuffers(1, &m_uiBuffer);
		OpenAlUtilities::CheckOpenALError("alDeleteBuffers\t");
	}
}

void actor::AudioSourceComponent::SetPositionFromScript(LuaPlus::LuaObject i_oVec3LuaObject)
{
	Vec3 _oVec3Position;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Position);
	SetPosition(_oVec3Position);
}

void actor::AudioSourceComponent::SetVelocityFromScript(LuaPlus::LuaObject i_oVec3LuaObject)
{
	Vec3 _oVec3Velocity;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Velocity);
	SetVelocity(_oVec3Velocity);
}

void actor::AudioSourceComponent::SetDirectionFromScript(LuaPlus::LuaObject i_oVec3LuaObject)
{
	Vec3 _oVec3Direction;
	script::LuaUtilities::ConvertLuaObjectToVec3(i_oVec3LuaObject, _oVec3Direction);
	SetDirection(_oVec3Direction);
}

LuaPlus::LuaObject actor::AudioSourceComponent::GetPositionFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetPosition(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

LuaPlus::LuaObject actor::AudioSourceComponent::GetVelocityFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetVelocity(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

LuaPlus::LuaObject actor::AudioSourceComponent::GetDirectionFromScript()
{
	LuaPlus::LuaObject _oVec3LuaObject;
	script::LuaUtilities::ConvertVec3ToLuaObject(GetDirection(), _oVec3LuaObject);
	return _oVec3LuaObject;
}

bool actor::AudioSourceComponent::InitAudioSourceInfo(const tinyxml2::XMLElement * pData)
{
	assert(pData);

	const tinyxml2::XMLElement* pFile = pData->FirstChildElement("File");
	assert(pFile);
	m_szAudioFilename = pFile->Attribute("value");

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
		l_oSS << "AudioSourceComponent position successfully initialized for Actor " << GetOwner()->GetName();
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
		l_oSS << "AudioSourceComponent velocity successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pDirection = pData->FirstChildElement("Direction");
	if (pDirection)
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = -1.0f;
		x = pDirection->FloatAttribute("x");
		y = pDirection->FloatAttribute("y");
		z = pDirection->FloatAttribute("z");
		m_vDirection = Vec3(x, y, z);

		std::stringstream l_oSS;
		l_oSS << "AudioSourceComponent direction successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pVolume = pData->FirstChildElement("Volume");
	if (pVolume)
	{
		m_fVolume = pVolume->FloatAttribute("value");

		std::stringstream l_oSS;
		l_oSS << "AudioSourceComponent volume successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pPitch = pData->FirstChildElement("Pitch");
	if (pPitch)
	{
		m_fPitch = pPitch->FloatAttribute("value");

		std::stringstream l_oSS;
		l_oSS << "AudioSourceComponent pitch successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	const tinyxml2::XMLElement* pLoop = pData->FirstChildElement("Loop");
	if (pLoop)
	{
		m_bIsLooping = pLoop->BoolAttribute("value");

		std::stringstream l_oSS;
		l_oSS << "AudioSourceComponent loop successfully initialized for Actor " << GetOwner()->GetName();
		LOG("ACTOR", l_oSS.str());
	}

	return true;
}
