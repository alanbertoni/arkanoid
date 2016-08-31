#pragma once

#include "../log/LogManager.h"
#include "../utilities/Types.h"
#include "../utilities/String.h"

#include <assert.h>
#include <Windows.h>

namespace core
{
	using WindowHandle = HWND;
	using MessageId = UINT;
	using MessageParam = WPARAM;
	using MessageParamExt = LPARAM;
	using MessageResult = LRESULT;

	struct AppMessage
	{
		WindowHandle m_oWindowHandle;
		MessageId m_oMessageId;
		MessageParam m_oParam;
		MessageParamExt m_oParamExt;
	};
}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x) { if((x)){ (x)->Release(); (x) = 0; } }
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(x) { delete (x); (x) = 0; }
#endif

#if defined(DEBUG) | defined(_DEBUG)

#ifndef HR
#define HR(x)												   \
	{                                                          \
		HRESULT hr = (x);                                      \
		if(FAILED(hr))                                         \
		{                                                      \
			LOG_ERROR(#x);									   \
		}                                                      \
	}
#endif

#else

#ifndef HR
#define HR(x) (x)
#endif

#endif 