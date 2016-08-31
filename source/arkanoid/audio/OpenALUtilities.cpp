
#include "OpenALUtilities.h"
#include "../log/LogManager.h"

namespace OpenAlUtilities
{
	const char * GetOpenALErrorString(int errID)
	{
		if (errID == AL_NO_ERROR) return "";
		if (errID == AL_INVALID_NAME) return "Invalid name";
		if (errID == AL_INVALID_ENUM) return " Invalid enum ";
		if (errID == AL_INVALID_VALUE) return " Invalid value ";
		if (errID == AL_INVALID_OPERATION) return " Invalid operation ";
		if (errID == AL_OUT_OF_MEMORY) return " Out of memory like! ";

		return " Unknown Error ";
	}

	void CheckOpenALError(std::string szMessage)
	{
		ALenum err = alGetError();
		if (err != AL_NO_ERROR)
		{
			szMessage += GetOpenALErrorString(err);
			LOG_ERROR(szMessage.c_str());
		}
	}

	bool LoadWaveFile(const char *szFilename, ALint& iFrequency, ALint& iDataSize, ALenum& eBufferFormat, std::vector<char>& vData)
	{
		bool bResult = false;
		LPWAVEFILEINFO pWaveInfo = new WAVEFILEINFO;
		if (pWaveInfo)
		{
			if (OpenAlUtilities::ParseWaveFile(szFilename, pWaveInfo))
			{
				iDataSize = pWaveInfo->ulDataSize;
				iFrequency = pWaveInfo->wfEXT.Format.nSamplesPerSec;
				eBufferFormat = OpenAlUtilities::GetWaveALBufferFormat(pWaveInfo);
				vData.reserve(iDataSize);
				// Seek to start of audio data
				fseek(pWaveInfo->pFile, pWaveInfo->ulDataOffset, SEEK_SET);
				// Read Sample Data
				if (fread(vData.data(), 1, pWaveInfo->ulDataSize, pWaveInfo->pFile) != pWaveInfo->ulDataSize)
				{
					std::string l_error("ERROR READING WAVE FILE DATA: ");
					l_error += szFilename;
					LOG_ERROR(l_error.c_str());
				}
				else
				{
					bResult = true;
				}

				fclose(pWaveInfo->pFile);
				pWaveInfo->pFile = 0;
			}
			else
			{
				std::string l_error("ERROR PARSING WAVE FILE: ");
				l_error += szFilename;
				LOG_ERROR(l_error.c_str());
			}
		}

		delete pWaveInfo;
		return bResult;
	}

	bool ParseWaveFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo)
	{
		WAVEFILEHEADER	waveFileHeader;
		RIFFCHUNK		riffChunk;
		WAVEFMT			waveFmt;
		bool l_bResult = false;

		if (!szFilename || !pWaveInfo)
			return l_bResult;

		memset(pWaveInfo, 0, sizeof(WAVEFILEINFO));

		// Open the wave file for reading
		fopen_s(&pWaveInfo->pFile, szFilename, "rb");
		if (pWaveInfo->pFile)
		{
			// Read Wave file header
			fread(&waveFileHeader, 1, sizeof(WAVEFILEHEADER), pWaveInfo->pFile);
			if (!_strnicmp(waveFileHeader.szRIFF, "RIFF", 4) && !_strnicmp(waveFileHeader.szWAVE, "WAVE", 4))
			{
				while (fread(&riffChunk, 1, sizeof(RIFFCHUNK), pWaveInfo->pFile) == sizeof(RIFFCHUNK))
				{
					if (!_strnicmp(riffChunk.szChunkName, "fmt ", 4))
					{
						if (riffChunk.ulChunkSize <= sizeof(WAVEFMT))
						{
							fread(&waveFmt, 1, riffChunk.ulChunkSize, pWaveInfo->pFile);

							// Determine if this is a WAVEFORMATEX or WAVEFORMATEXTENSIBLE wave file
							if (waveFmt.usFormatTag == WAVE_FORMAT_PCM)
							{
								pWaveInfo->wfType = WF_EX;
								memcpy(&pWaveInfo->wfEXT.Format, &waveFmt, sizeof(PCMWAVEFORMAT));
							}
							else if (waveFmt.usFormatTag == WAVE_FORMAT_EXTENSIBLE)
							{
								pWaveInfo->wfType = WF_EXT;
								memcpy(&pWaveInfo->wfEXT, &waveFmt, sizeof(WAVEFORMATEXTENSIBLE));
							}
						}
						else
						{
							fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
						}
					}
					else if (!_strnicmp(riffChunk.szChunkName, "data", 4))
					{
						pWaveInfo->ulDataSize = riffChunk.ulChunkSize;
						pWaveInfo->ulDataOffset = ftell(pWaveInfo->pFile);
						fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
					}
					else
					{
						fseek(pWaveInfo->pFile, riffChunk.ulChunkSize, SEEK_CUR);
					}

					// Ensure that we are correctly aligned for next chunk
					if (riffChunk.ulChunkSize & 1)
						fseek(pWaveInfo->pFile, 1, SEEK_CUR);
				}

				if (pWaveInfo->ulDataSize && pWaveInfo->ulDataOffset && ((pWaveInfo->wfType == WF_EX) || (pWaveInfo->wfType == WF_EXT)))
					l_bResult = true;
				else
					fclose(pWaveInfo->pFile);
			}
		}

		return l_bResult;
	}

	ALenum GetWaveALBufferFormat(LPWAVEFILEINFO pWaveInfo)
	{
		if (pWaveInfo->wfType == WF_EX)
		{
			if (pWaveInfo->wfEXT.Format.nChannels == 1)
			{
				switch (pWaveInfo->wfEXT.Format.wBitsPerSample)
				{
				case 4:
					return alGetEnumValue("AL_FORMAT_MONO_IMA4");
				case 8:
					return alGetEnumValue("AL_FORMAT_MONO8");
				case 16:
					return alGetEnumValue("AL_FORMAT_MONO16");
				}
			}
			else if (pWaveInfo->wfEXT.Format.nChannels == 2)
			{
				switch (pWaveInfo->wfEXT.Format.wBitsPerSample)
				{
				case 4:
					return alGetEnumValue("AL_FORMAT_STEREO_IMA4");
				case 8:
					return alGetEnumValue("AL_FORMAT_STEREO8");
				case 16:
					return alGetEnumValue("AL_FORMAT_STEREO16");
				}
			}
			else if ((pWaveInfo->wfEXT.Format.nChannels == 4) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16))
				return alGetEnumValue("AL_FORMAT_QUAD16");
		}
		else if (pWaveInfo->wfType == WF_EXT)
		{
			if ((pWaveInfo->wfEXT.Format.nChannels == 1) &&
				((pWaveInfo->wfEXT.dwChannelMask == SPEAKER_FRONT_CENTER) ||
					(pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)) ||
					(pWaveInfo->wfEXT.dwChannelMask == 0)))
			{
				switch (pWaveInfo->wfEXT.Format.wBitsPerSample)
				{
				case 4:
					return alGetEnumValue("AL_FORMAT_MONO_IMA4");
				case 8:
					return alGetEnumValue("AL_FORMAT_MONO8");
				case 16:
					return alGetEnumValue("AL_FORMAT_MONO16");
				}
			}
			else if ((pWaveInfo->wfEXT.Format.nChannels == 2) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT)))
			{
				switch (pWaveInfo->wfEXT.Format.wBitsPerSample)
				{
				case 4:
					return alGetEnumValue("AL_FORMAT_STEREO_IMA4");
				case 8:
					return alGetEnumValue("AL_FORMAT_STEREO8");
				case 16:
					return alGetEnumValue("AL_FORMAT_STEREO16");
				}
			}
			else if ((pWaveInfo->wfEXT.Format.nChannels == 2) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)))
				return alGetEnumValue("AL_FORMAT_REAR16");
			else if ((pWaveInfo->wfEXT.Format.nChannels == 4) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)))
				return alGetEnumValue("AL_FORMAT_QUAD16");
			else if ((pWaveInfo->wfEXT.Format.nChannels == 6) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT)))
				return alGetEnumValue("AL_FORMAT_51CHN16");
			else if ((pWaveInfo->wfEXT.Format.nChannels == 7) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_BACK_CENTER)))
				return alGetEnumValue("AL_FORMAT_61CHN16");
			else if ((pWaveInfo->wfEXT.Format.nChannels == 8) && (pWaveInfo->wfEXT.Format.wBitsPerSample == 16) && (pWaveInfo->wfEXT.dwChannelMask == (SPEAKER_FRONT_LEFT | SPEAKER_FRONT_RIGHT | SPEAKER_FRONT_CENTER | SPEAKER_LOW_FREQUENCY | SPEAKER_BACK_LEFT | SPEAKER_BACK_RIGHT | SPEAKER_SIDE_LEFT | SPEAKER_SIDE_RIGHT)))
				return alGetEnumValue("AL_FORMAT_71CHN16");
		}

		LOG_ERROR("ERROR UNKNOWN AUDIO FORMAT");
		return alGetEnumValue("AL_FORMAT_MONO8");
	}

	void Swap(short &s1, short &s2)
	{
		short sTemp = s1;
		s1 = s2;
		s2 = sTemp;
	}

	bool LoadOggFile(const char * szFilename, ALint & iFrequency, ALint & iDataSize, ALenum & eBufferFormat, std::vector<char>& vData)
	{
		// Open Ogg Stream
		OggVorbis_File	sOggVorbisFile;
		vorbis_info		*psVorbisInfo;
		unsigned long	ulFrequency = 0;
		unsigned long	ulChannels = 0;
		unsigned long	ulBufferSize;
		eBufferFormat = 0;

		// Open the OggVorbis file
		FILE *pOggVorbisFile = nullptr;
		fopen_s(&pOggVorbisFile, szFilename, "rb");
		if (!pOggVorbisFile)
		{
			std::string l_error("ERROR OPENING OGG FILE: ");
			l_error += szFilename;
			LOG_ERROR(l_error.c_str());
			return false;
		}

		if (ov_open_callbacks(pOggVorbisFile, &sOggVorbisFile, NULL, 0, OV_CALLBACKS_DEFAULT) == 0)
		{
			// Get some information about the file (Channels, Format, and Frequency)
			psVorbisInfo = ov_info(&sOggVorbisFile, -1);
			if (psVorbisInfo)
			{
				ulFrequency = psVorbisInfo->rate;
				iFrequency = psVorbisInfo->rate;
				ulChannels = psVorbisInfo->channels;
				if (psVorbisInfo->channels == 1)
				{
					eBufferFormat = AL_FORMAT_MONO16;
					// Set BufferSize to 250ms (Frequency * 2 (16bit) divided by 4 (quarter of a second))
					ulBufferSize = ulFrequency >> 1;
					// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
					ulBufferSize -= (ulBufferSize % 2);
				}
				else if (psVorbisInfo->channels == 2)
				{
					eBufferFormat = AL_FORMAT_STEREO16;
					// Set BufferSize to 250ms (Frequency * 4 (16bit stereo) divided by 4 (quarter of a second))
					ulBufferSize = ulFrequency;
					// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
					ulBufferSize -= (ulBufferSize % 4);
				}
				else if (psVorbisInfo->channels == 4)
				{
					eBufferFormat = alGetEnumValue("AL_FORMAT_QUAD16");
					// Set BufferSize to 250ms (Frequency * 8 (16bit 4-channel) divided by 4 (quarter of a second))
					ulBufferSize = ulFrequency * 2;
					// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
					ulBufferSize -= (ulBufferSize % 8);
				}
				else if (psVorbisInfo->channels == 6)
				{
					eBufferFormat = alGetEnumValue("AL_FORMAT_51CHN16");
					// Set BufferSize to 250ms (Frequency * 12 (16bit 6-channel) divided by 4 (quarter of a second))
					ulBufferSize = ulFrequency * 3;
					// IMPORTANT : The Buffer Size must be an exact multiple of the BlockAlignment ...
					ulBufferSize -= (ulBufferSize % 12);
				}
			}
			else
			{
				std::string l_error("ERROR GETTING INFORMATION ABOUT THE OGG FILE: ");
				l_error += szFilename;
				LOG_ERROR(l_error.c_str());
				ov_clear(&sOggVorbisFile);
				return false;
			}

			if (eBufferFormat != 0)
			{
				// Allocate a buffer to be used to store decoded data for all Buffers
				char* pDecodeBuffer = new char[ulBufferSize];
				if (!pDecodeBuffer)
				{
					LOG_ERROR("Failed to allocate memory for decoded OggVorbis data");
					ov_clear(&sOggVorbisFile);
					return false;
				}

				// Fill the Buffer with decoded audio data from the OggVorbis file
				unsigned long ulBytesWritten;
				do
				{
					ulBytesWritten = DecodeOggVorbis(&sOggVorbisFile, pDecodeBuffer, ulBufferSize, ulChannels);
					if (ulBytesWritten)
					{
						iDataSize += ulBytesWritten;
						// Append to end of buffer
						vData.insert(vData.end(), pDecodeBuffer, pDecodeBuffer + ulBytesWritten);
					}
				} while (ulBytesWritten > 0);

				
				if (pDecodeBuffer)
				{
					delete[] pDecodeBuffer;
					pDecodeBuffer = NULL;
				}

			}
			else
			{
				LOG_ERROR("Failed to find format information, or unsupported format");
				return false;
			}

			// Close OggVorbis stream
			ov_clear(&sOggVorbisFile);
		}

		return true;
	}

	unsigned long DecodeOggVorbis(OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, unsigned long ulBufferSize, unsigned long ulChannels)
	{
		int current_section;
		long lDecodeSize;
		unsigned long ulSamples;
		short *pSamples;

		unsigned long ulBytesDone = 0;
		while (1)
		{
			lDecodeSize = ov_read(psOggVorbisFile, pDecodeBuffer + ulBytesDone, ulBufferSize - ulBytesDone, 0, 2, 1, &current_section);
			if (lDecodeSize > 0)
			{
				ulBytesDone += lDecodeSize;

				if (ulBytesDone >= ulBufferSize)
					break;
			}
			else
			{
				break;
			}
		}

		// Mono, Stereo and 4-Channel files decode into the same channel order as WAVEFORMATEXTENSIBLE,
		// however 6-Channels files need to be re-ordered
		if (ulChannels == 6)
		{
			pSamples = (short*)pDecodeBuffer;
			for (ulSamples = 0; ulSamples < (ulBufferSize >> 1); ulSamples += 6)
			{
				// WAVEFORMATEXTENSIBLE Order : FL, FR, FC, LFE, RL, RR
				// OggVorbis Order            : FL, FC, FR,  RL, RR, LFE
				Swap(pSamples[ulSamples + 1], pSamples[ulSamples + 2]);
				Swap(pSamples[ulSamples + 3], pSamples[ulSamples + 5]);
				Swap(pSamples[ulSamples + 4], pSamples[ulSamples + 5]);
			}
		}

		return ulBytesDone;
	}
}
	