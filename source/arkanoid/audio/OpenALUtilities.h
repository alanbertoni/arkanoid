
// Adapted from the example source code of OpenAL
#pragma once

#include <al.h>
#include <alc.h>
#include <EFX-Util.h>
#include <efx.h>
#include <efx-creative.h>

#include <windows.h>
#include <stdio.h>
#include <ks.h>
#include <ksmedia.h>
#include <errno.h>

#include <ogg/ogg.h>
#include <ogg/os_types.h>
#include <vorbis/vorbisfile.h>
#include <vorbis/codec.h>

#include "../utilities/Types.h"

enum WAVEFILETYPE
{
	WF_EX = 1,
	WF_EXT = 2
};

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX
{
	WORD    wFormatTag;
	WORD    nChannels;
	DWORD   nSamplesPerSec;
	DWORD   nAvgBytesPerSec;
	WORD    nBlockAlign;
	WORD    wBitsPerSample;
	WORD    cbSize;
} WAVEFORMATEX;
#endif /* _WAVEFORMATEX_ */

#ifndef _WAVEFORMATEXTENSIBLE_
#define _WAVEFORMATEXTENSIBLE_
typedef struct {
	WAVEFORMATEX    Format;
	union {
		WORD wValidBitsPerSample;       /* bits of precision  */
		WORD wSamplesPerBlock;          /* valid if wBitsPerSample==0 */
		WORD wReserved;                 /* If neither applies, set to zero. */
	} Samples;
	DWORD           dwChannelMask;      /* which channels are */
										/* present in stream  */
	GUID            SubFormat;
} WAVEFORMATEXTENSIBLE, *PWAVEFORMATEXTENSIBLE;
#endif // !_WAVEFORMATEXTENSIBLE_

typedef struct
{
	WAVEFILETYPE	wfType;
	WAVEFORMATEXTENSIBLE wfEXT;		// For non-WAVEFORMATEXTENSIBLE wavefiles, the header is stored in the Format member of wfEXT
	char			*pData;
	unsigned long	ulDataSize;
	FILE			*pFile;
	unsigned long	ulDataOffset;
} WAVEFILEINFO, *LPWAVEFILEINFO;

#pragma pack(push, 4)

typedef struct
{
	char			szRIFF[4];
	unsigned long	ulRIFFSize;
	char			szWAVE[4];
} WAVEFILEHEADER;

typedef struct
{
	char			szChunkName[4];
	unsigned long	ulChunkSize;
} RIFFCHUNK;

typedef struct
{
	unsigned short	usFormatTag;
	unsigned short	usChannels;
	unsigned long	ulSamplesPerSec;
	unsigned long	ulAvgBytesPerSec;
	unsigned short	usBlockAlign;
	unsigned short	usBitsPerSample;
	unsigned short	usSize;
	unsigned short  usReserved;
	unsigned long	ulChannelMask;
	GUID            guidSubFormat;
} WAVEFMT;

#pragma pack(pop)

namespace OpenAlUtilities
{
	const char * GetOpenALErrorString(int errID);
	void CheckOpenALError(std::string szMessage);

	bool LoadWaveFile(const char *szFilename, ALint& iFrequency, ALint& iDataSize, ALenum& eBufferFormat, std::vector<char>& vData);
	bool ParseWaveFile(const char *szFilename, LPWAVEFILEINFO pWaveInfo);
	ALenum GetWaveALBufferFormat(LPWAVEFILEINFO pWaveInfo);

	bool LoadOggFile(const char *szFilename, ALint& iFrequency, ALint& iDataSize, ALenum& eBufferFormat, std::vector<char>& vData);
	unsigned long DecodeOggVorbis(OggVorbis_File *psOggVorbisFile, char *pDecodeBuffer, unsigned long ulBufferSize, unsigned long ulChannels);
}
