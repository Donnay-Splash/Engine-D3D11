#pragma once


//////////////
// INCLUDES //
//////////////
#include <pdh.h>


namespace Engine
{
    class PerformanceTracker
    {
    public:
        PerformanceTracker();
        PerformanceTracker(const PerformanceTracker&) = delete;
        ~PerformanceTracker();

        void Initialize();
        void Shutdown();
        void Frame();
        int GetCpuPercentage();

    private:
        bool m_canReadCpu;
        HQUERY m_queryHandle;
        HCOUNTER m_counterHandle;
        unsigned long m_lastSampleTime;
        long m_cpuUsage;
    };
}

