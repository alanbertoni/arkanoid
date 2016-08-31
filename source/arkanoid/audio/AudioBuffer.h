#pragma once
#include "OpenALUtilities.h"
#include "../utilities/Types.h"

namespace audio
{
	class AudioBuffer
	{
	public:
		AudioBuffer(const char* szFilename);
		~AudioBuffer() = default;

		AudioBuffer(const AudioBuffer&) = delete;
		AudioBuffer& operator=(const AudioBuffer&) = delete;
		AudioBuffer(AudioBuffer&&) = delete;
		AudioBuffer& operator=(AudioBuffer&&) = delete;

		void LoadFile();

		ALint GetFrequency() const;
		ALint GetDataSize() const;
		ALenum GetBufferFormat() const;
		const void* GetData() const;

	private:
		std::string m_szFilename;
		ALint m_iDataSize;
		ALint m_iFrequency;
		ALenum m_eBufferFormat;
		std::vector<char> m_vData;
	};

	inline ALint AudioBuffer::GetFrequency() const
	{
		return m_iFrequency;
	}

	inline ALint AudioBuffer::GetDataSize() const
	{
		return m_iDataSize;
	}
	inline ALenum AudioBuffer::GetBufferFormat() const
	{
		return m_eBufferFormat;
	}

	inline const void* AudioBuffer::GetData() const
	{
		return m_vData.data();
	}
}
