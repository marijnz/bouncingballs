#include "stdafx.h"
#include "gep/timer.h"
#include "gep/exception.h"
#include <Windows.h>

gep::Timer::Timer()
{
    if(!QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency)){
        throw Exception("no performance timer available");
    }
    QueryPerformanceCounter((LARGE_INTEGER*)&m_start);
    m_resolution = 1.0 / (double)m_frequency;
    m_pausedTime = 0;
}

float gep::Timer::getTimeAsFloat() const
{
    return (float)(getTime() * m_resolution);
}

double gep::Timer::getTimeAsDouble() const
{
    return getTime() * m_resolution;
}

gep::uint64 gep::Timer::getTime() const
{
    if(m_pausedTime > 0)
        return m_pausedTime;
    uint64 time;
    QueryPerformanceCounter((LARGE_INTEGER*)&time);
    return time - m_start;
}

void gep::Timer::pause()
{
    GEP_ASSERT(m_pausedTime == 0, "timer is already paused");
    m_pausedTime = getTime();
}

void gep::Timer::unpause()
{
    GEP_ASSERT(m_pausedTime > 0, "timer is not paused");
    auto temp = m_pausedTime;
    m_pausedTime = 0;
    m_start += getTime() - temp;
}

gep::PointInTime::PointInTime(Timer& timer)
    : m_timer(&timer)
{
    m_time = m_timer->getTime();
}

float gep::PointInTime::operator - (const PointInTime& rh) const
{
   return (float)((double)(m_time - rh.m_time) * m_timer->getResolution());
}

bool gep::PointInTime::operator < (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time < rh.m_time;
}

bool gep::PointInTime::operator > (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time > rh.m_time;
}

bool gep::PointInTime::operator <= (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time <= rh.m_time;
}

bool gep::PointInTime::operator >= (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time >= rh.m_time;
}

bool gep::PointInTime::operator == (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time == rh.m_time;
}

bool gep::PointInTime::operator != (const PointInTime& rh) const
{
    GEP_ASSERT(m_timer == rh.m_timer, "comparing with different timers");
    return m_time != rh.m_time;
}
