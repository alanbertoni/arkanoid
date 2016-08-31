#pragma once
// Adapted from "3d Game Programming With DirectX11" by Frank Luna

namespace utilities
{
	class Time
	{
	public:
		Time();
		~Time() = default;

		Time(const Time&) = delete;
		Time& operator=(const Time&) = delete;
		Time(Time&&) = delete;
		Time& operator=(Time&&) = delete;

		float TotalTime() const;  // in seconds
		float DeltaTime() const; // in seconds

		void Tick();  // Call every frame.
		void Reset(); // Call before message loop.
		void Start(); // Call when unpaused.
		void Stop();  // Call when paused.

	private:
		double m_dSecondsPerCount;
		double m_dDeltaTime;

		__int64 m_iBaseTime;
		__int64 m_iPausedTime;
		__int64 m_iStopTime;
		__int64 m_iPrevTime;
		__int64 m_iCurrTime;

		bool m_bStopped;
	};
}
