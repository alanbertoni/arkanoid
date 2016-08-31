
#include "AudioManager.h"
#include "AudioBuffer.h"
#include "OpenALUtilities.h"

void audio::AudioManager::CreateInstance()
{
	if (GetSingletonPtr() == nullptr)
	{
		new AudioManager{};
	}
}

audio::AudioManager::AudioManager()
	: Singleton{}
	, m_oAudioBufferMap{}
	, m_bIsInit{false}
{
	m_oOnReleaseAudioSourceEvent.Subscribe(this, &AudioManager::ReleaseAudioBuffer, "Audio/Release/AudioSourceComponent");
}

audio::AudioManager::~AudioManager()
{
	ShutdownOpenAL();
}

bool audio::AudioManager::Init()
{
	if (InitOpenAL())
	{
		LOG_INFO("AudioManager succesfully initialized");
		return true;
	}
	else
	{
		LOG_ERROR("Failed to initialize the AudioManager");
		return false;
	}
}

bool audio::AudioManager::InitOpenAL()
{
	ALCdevice *pDevice = alcOpenDevice(NULL);
	OpenAlUtilities::CheckOpenALError("alcOpenDevice");
	if (pDevice)
	{
		ALCcontext *pContext = alcCreateContext(pDevice, NULL);
		OpenAlUtilities::CheckOpenALError("alcCreateContext");
		if (pContext)
		{
			alcMakeContextCurrent(pContext);
			OpenAlUtilities::CheckOpenALError("alcMakeContextCurrent");
			LOG_INFO("OpenAL initialized!");
			m_bIsInit = true;
		}
		else
		{
			alcCloseDevice(pDevice);
			OpenAlUtilities::CheckOpenALError("alcCloseDevice");
			LOG_ERROR("ERROR CREATING OPENAL CONTEXT");
		}
	}
	else
	{
		LOG_ERROR("ERROR CREATING OPENAL DEVICE");
	}

	return m_bIsInit;
}

void audio::AudioManager::ShutdownOpenAL()
{
	if (m_bIsInit)
	{
		ALCcontext *pContext = alcGetCurrentContext();
		OpenAlUtilities::CheckOpenALError("alcGetCurrentContext");
		ALCdevice *pDevice = alcGetContextsDevice(pContext);
		OpenAlUtilities::CheckOpenALError("alcGetContextsDevice");

		alcMakeContextCurrent(NULL);
		OpenAlUtilities::CheckOpenALError("alcMakeContextCurrent");
		alcDestroyContext(pContext);
		OpenAlUtilities::CheckOpenALError("alcDestroyContext");
		alcCloseDevice(pDevice);
		OpenAlUtilities::CheckOpenALError("alcCloseDevice");

		LOG_INFO("AudioManager successfully destroyed");
	}
}

const audio::AudioBuffer& audio::AudioManager::GetAudioBuffer(const char* szFilename)
{
	assert(szFilename);
	AudioId l_oId = utilities::HashString(szFilename);
	auto l_oIt = m_oAudioBufferMap.find(l_oId);
	if (l_oIt != m_oAudioBufferMap.end())
	{
		auto& l_oPair = l_oIt->second;
		++l_oPair.first;
		return *(l_oPair.second);
	}
	else
	{
		AudioBuffer* l_pAudioBuffer = LoadAudioBuffer(szFilename);
		m_oAudioBufferMap.emplace(l_oId, std::make_pair(1, l_pAudioBuffer));
		return *l_pAudioBuffer;
	}
}

audio::AudioBuffer* audio::AudioManager::LoadAudioBuffer(const char* szFilename) const
{
	AudioBuffer* pAudioBuffer = new AudioBuffer(szFilename);
	pAudioBuffer->LoadFile();
	return pAudioBuffer;
}

void audio::AudioManager::ReleaseAudioBuffer(const char* szFilename)
{
	assert(szFilename);
	AudioId l_oId = utilities::HashString(szFilename);
	auto l_oIt = m_oAudioBufferMap.find(l_oId);
	assert(l_oIt != m_oAudioBufferMap.end());

	auto& l_oPair = l_oIt->second;
	--l_oPair.first;
	if (l_oPair.first < 1)
	{
		delete l_oPair.second;
		m_oAudioBufferMap.erase(l_oIt);
		std::stringstream l_oSS;
		l_oSS << "Deleting AudioBuffer object with filename " << szFilename;
		LOG_INFO(l_oSS.str());
	}
}
