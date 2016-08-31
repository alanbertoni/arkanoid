#include "AudioBuffer.h"
#include "../log/LogManager.h"
#include "../utilities/String.h"

audio::AudioBuffer::AudioBuffer(const char* szFilename)
	: m_szFilename{ szFilename }
	, m_iDataSize{ 0 }
	, m_iFrequency{ 0 }
{
}

void audio::AudioBuffer::LoadFile()
{
	std::vector<std::string> l_szSplitFilename;
	utilities::SplitString(m_szFilename, l_szSplitFilename, '.');
	if (l_szSplitFilename[l_szSplitFilename.size() - 1] == "wav")
	{
		OpenAlUtilities::LoadWaveFile(m_szFilename.c_str(), m_iFrequency, m_iDataSize, m_eBufferFormat, m_vData);
	}
	else if (l_szSplitFilename[l_szSplitFilename.size() - 1] == "ogg")
	{
		OpenAlUtilities::LoadOggFile(m_szFilename.c_str(), m_iFrequency, m_iDataSize, m_eBufferFormat, m_vData);
	}
}
