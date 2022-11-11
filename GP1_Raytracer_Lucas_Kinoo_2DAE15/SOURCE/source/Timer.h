#pragma once

//Standard includes
#include <cstdint>
#include <vector>

namespace dae
{
	class Timer
	{
	public:
		Timer();
		virtual ~Timer() = default;

		Timer(const Timer&) = delete;
		Timer(Timer&&) noexcept = delete;
		Timer& operator=(const Timer&) = delete;
		Timer& operator=(Timer&&) noexcept = delete;

		void StartBenchmark(int numFrames = 10);

		void Reset();
		void Start();
		void Update();
		void Stop();

		uint32_t GetFPS() const { return m_FPS; };
		float GetdFPS() const { return m_dFPS; };
		float GetElapsed() const { return m_ElapsedTime; };
		float GetTotal() const { return m_TotalTime; };
		bool IsRunning() const { return !m_IsStopped; };

	private:
		uint64_t m_BaseTime = 0;
		uint64_t m_PausedTime = 0;
		uint64_t m_StopTime = 0;
		uint64_t m_PreviousTime = 0;
		uint64_t m_CurrentTime = 0;

		uint32_t m_FPS = 0;
		float m_dFPS = 0.0f;
		uint32_t m_FPSCount = 0;

		float m_TotalTime = 0.0f;
		float m_ElapsedTime = 0.0f;
		float m_SecondsPerCount = 0.0f;
		float m_ElapsedUpperBound = 0.03f;
		float m_FPSTimer = 0.0f;

		bool m_IsStopped = true;
		bool m_ForceElapsedUpperBound = false;

		bool m_BenchmarkActive = false;
		float m_BenchmarkHigh{ 0.f };
		float m_BenchmarkLow{ 0.f };
		float m_BenchmarkAvg{ 0.f };
		int m_BenchmarkFrames{ 0 };
		int m_BenchmarkCurrFrame{ 0 };
		std::vector<float> m_Benchmarks{};
	};
}
