
#include "Time.h"

#include <windows.h>
using namespace utilities;

Time::Time()
	: m_dSecondsPerCount{ 0.0 }
	, m_dDeltaTime{ -1.0 }
	, m_iBaseTime{ 0 }
	, m_iPausedTime{ 0 }
	, m_iPrevTime{ 0 }
	, m_iCurrTime{ 0 }
	, m_bStopped{ false }
{
	__int64 _iCountsPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&_iCountsPerSec);
	m_dSecondsPerCount = 1.0 / static_cast<double>(_iCountsPerSec);
}

float Time::TotalTime() const
{
	if (m_bStopped)
	{
		return static_cast<float>(((m_iStopTime - m_iPausedTime) - m_iBaseTime) * m_dSecondsPerCount);
	}
	else
	{
		return static_cast<float>(((m_iCurrTime - m_iPausedTime) - m_iBaseTime) * m_dSecondsPerCount);
	}
}

float Time::DeltaTime() const
{
	return static_cast<float>(m_dDeltaTime);
}

void Time::Reset()
{
	__int64 _iCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&_iCurrTime);

	m_iBaseTime = _iCurrTime;
	m_iPrevTime = _iCurrTime;
	m_iStopTime = 0;
	m_bStopped = false;
}

void Time::Start()
{
	if (m_bStopped)
	{
		__int64 _iStartTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&_iStartTime);

		m_iPausedTime += (_iStartTime - m_iStopTime);

		m_iPrevTime = _iStartTime;
		m_iStopTime = 0;
		m_bStopped = false;
	}
}

void Time::Stop()
{
	if (!m_bStopped)
	{
		__int64 _iCurrTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&_iCurrTime);

		m_iStopTime = _iCurrTime;
		m_bStopped = true;
	}
}

void Time::Tick()
{
	if (m_bStopped)
	{
		m_dDeltaTime = 0.0;
		return;
	}

	__int64 _iCurrTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&_iCurrTime);
	m_iCurrTime = _iCurrTime;

	m_dDeltaTime = (m_iCurrTime - m_iPrevTime) * m_dSecondsPerCount;

	m_iPrevTime = m_iCurrTime;

	if (m_dDeltaTime < 0.0)
	{
		m_dDeltaTime = 0.0;
	}
}
