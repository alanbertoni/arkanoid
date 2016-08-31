#pragma once

#include "../core/Common.h"
#include "../core/Singleton.h"
#include "../event/EventListener.h"

namespace audio
{
	class AudioBuffer;
}

namespace audio
{
	class AudioManager : public core::Singleton<AudioManager>
	{
	public:
		static void CreateInstance();
		~AudioManager();

		bool Init();

		const AudioBuffer& GetAudioBuffer(const char* szFilename);

	private:
		AudioManager();
		AudioManager(const AudioManager&) = delete;
		AudioManager(AudioManager&&) = delete;
		AudioManager& operator=(const AudioManager&) = delete;
		AudioManager& operator=(AudioManager&&) = delete;

		bool InitOpenAL();
		void ShutdownOpenAL();

		AudioBuffer* LoadAudioBuffer(const char* szFilename) const;
		void ReleaseAudioBuffer(const char* szFilename);

		event::EventListener<AudioManager, const char*> m_oOnReleaseAudioSourceEvent;
		std::map<AudioId, std::pair<size_t, AudioBuffer*>> m_oAudioBufferMap;
		bool m_bIsInit;
	};
}
