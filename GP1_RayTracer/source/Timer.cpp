#include "Timer.h"

#include <iostream>
#include <numeric>

#include <iostream>
#include <fstream>

#include "SDL.h"
using namespace dae;

Timer::Timer()
{
	const uint64_t countsPerSecond = SDL_GetPerformanceFrequency();
	m_SecondsPerCount = 1.0f / static_cast<float>(countsPerSecond);
}

void Timer::Reset()
{
	const uint64_t currentTime = SDL_GetPerformanceCounter();

	m_BaseTime = currentTime;
	m_PreviousTime = currentTime;
	m_StopTime = 0;
	m_FPSTimer = 0.0f;
	m_FPSCount = 0;
	m_IsStopped = false;
}

void Timer::Start()
{
	const uint64_t startTime = SDL_GetPerformanceCounter();

	if (m_IsStopped)
	{
		m_PausedTime += (startTime - m_StopTime);

		m_PreviousTime = startTime;
		m_StopTime = 0;
		m_IsStopped = false;
	}
}

void Timer::StartBenchmark(int numFrames)
{
	if (m_BenchmarkActive)
	{
		std::cout << "(Benchmark already running)";
		return;
	}

	m_BenchmarkActive = true;

	m_BenchmarkAvg = 0.f;
	m_BenchmarkHigh = FLT_MIN;
	m_BenchmarkLow = FLT_MAX;

	m_BenchmarkFrames = numFrames;
	m_BenchmarkCurrFrame = 0;

	m_Benchmarks.clear();
	m_Benchmarks.resize(m_BenchmarkFrames);

	std::cout << "**BENCHMARK STARTED**\n";
}

void Timer::Update()
{
	if (m_IsStopped)
	{
		m_FPS = 0;
		m_ElapsedTime = 0.0f;
		m_TotalTime = (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_BaseTime);
		return;
	}

	const uint64_t currentTime = SDL_GetPerformanceCounter();
	m_CurrentTime = currentTime;

	m_ElapsedTime = (float)((m_CurrentTime - m_PreviousTime) * m_SecondsPerCount);
	m_PreviousTime = m_CurrentTime;

	if (m_ElapsedTime < 0.0f)
		m_ElapsedTime = 0.0f;

	if (m_ForceElapsedUpperBound && m_ElapsedTime > m_ElapsedUpperBound)
	{
		m_ElapsedTime = m_ElapsedUpperBound;
	}

	m_TotalTime = (float)(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);

	//FPS LOGIC
	m_FPSTimer += m_ElapsedTime;
	++m_FPSCount;
	if (m_FPSTimer >= 1.0f)
	{
		m_dFPS = m_FPSCount / m_FPSTimer;
		m_FPS = m_FPSCount;
		m_FPSCount = 0;
		m_FPSTimer = 0.0f;

		if (m_BenchmarkActive)
		{
			m_Benchmarks[m_BenchmarkCurrFrame] = m_dFPS;

			m_BenchmarkLow = std::min(m_BenchmarkLow, m_dFPS);
			m_BenchmarkHigh = std::max(m_BenchmarkHigh, m_dFPS);

			++m_BenchmarkCurrFrame;
			if (m_BenchmarkCurrFrame >= m_BenchmarkFrames)
			{
				m_BenchmarkActive = false;
				m_BenchmarkAvg = std::accumulate(m_Benchmarks.begin(), m_Benchmarks.end(), 0.f) / float(m_BenchmarkFrames);

				//print
				std::cout << "**BENCHMARK FINISHED**\n";
				std::cout << ">> HIGH = " << m_BenchmarkHigh << std::endl;
				std::cout << ">> LOW = " << m_BenchmarkLow << std::endl;
				std::cout << ">> AVG = " << m_BenchmarkAvg << std::endl;

				//file save
				std::ofstream fileStream("benchmark.txt");
				fileStream << "FRAMES = " << m_BenchmarkCurrFrame << std::endl;
				fileStream << "HIGH = " << m_BenchmarkHigh << std::endl;
				fileStream << "LOW = " << m_BenchmarkLow << std::endl;
				fileStream << "AVG = " << m_BenchmarkAvg << std::endl;
				fileStream.close();
			}
		}
	}
}

void Timer::Stop()
{
	if (!m_IsStopped)
	{
		const uint64_t currentTime = SDL_GetPerformanceCounter();

		m_StopTime = currentTime;
		m_IsStopped = true;
	}
}
