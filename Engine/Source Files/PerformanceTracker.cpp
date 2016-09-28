#include "pch.h"
#include "PerformanceTracker.h"


PerformanceTracker::PerformanceTracker()
{
}


PerformanceTracker::~PerformanceTracker()
{
}


void PerformanceTracker::Initialize()
{
    PDH_STATUS status;


    // Initialize the flag indicating whether this object can read the system cpu usage or not.
    m_canReadCpu = true;

    // Create a query object to poll cpu usage.
    status = PdhOpenQuery(NULL, 0, &m_queryHandle);
    if(status != ERROR_SUCCESS)
    {
        m_canReadCpu = false;
    }

    // Set query object to poll all cpus in the system.
    status = PdhAddCounter(m_queryHandle, TEXT("\\Processor(_Total)\\% processor time"), 0, &m_counterHandle);
    if(status != ERROR_SUCCESS)
    {
        m_canReadCpu = false;
    }

    // Initialize the start time and cpu usage.
    m_lastSampleTime = GetTickCount(); 
    m_cpuUsage = 0;

    return;
}


void PerformanceTracker::Shutdown()
{
    if(m_canReadCpu)
    {
        PdhCloseQuery(m_queryHandle);
    }

    return;
}


void PerformanceTracker::Frame()
{
    PDH_FMT_COUNTERVALUE value; 

    if(m_canReadCpu)
    {
        // If it has been 1 second then update the current cpu usage and reset the 1 second timer again.
        if((m_lastSampleTime + 1000) < GetTickCount())
        {
            m_lastSampleTime = GetTickCount(); 

            PdhCollectQueryData(m_queryHandle);
        
            PdhGetFormattedCounterValue(m_counterHandle, PDH_FMT_LONG, NULL, &value);

            m_cpuUsage = value.longValue;
        }
    }

    return;
}


int PerformanceTracker::GetCpuPercentage()
{
    int usage;


    // If the class can read the cpu from the operating system then return the current usage.  If not then return zero.
    if(m_canReadCpu)
    {
        usage = (int)m_cpuUsage;
    }
    else
    {
        usage = 0;
    }

    return usage;
}