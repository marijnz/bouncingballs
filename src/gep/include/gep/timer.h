#pragma once

namespace gep
{
    /// \brief for measuring time
    class GEP_API Timer
    {
    private:
        uint64 m_start;
        double m_resolution;
        uint64 m_frequency;
        uint64 m_pausedTime;

    public:
        Timer();

        /// \brief returns the time elapsed since the start of the timer as a float (in seconds)
        float getTimeAsFloat() const;
        /// \brief returns the time elapsed since the start of the timer as a double (in seconds)
        double getTimeAsDouble() const;
        /// \brief returns the time elapsed since the start of the timer as a 64 bit int (in the smallest measurable unit)
        uint64 getTime() const;
        /// \brief returns the resolution of the timer
        inline double getResolution() const { return m_resolution; }
        /// \brief pauses the timer
        void pause();
        /// \brief continues the timer after pausing
        void unpause();
    };

    /// \brief stores a point in time with maximum percision
    class GEP_API PointInTime
    {
    private:
        Timer* m_timer;
        uint64 m_time;

    public:
        /// \brief constructor
        /// \param timer the timer to use for measuring time
        PointInTime(Timer& timer);

        /// \brief computes the time difference between two points in time
        float operator - (const PointInTime& rh) const;

        // comparison operators
        bool operator > (const PointInTime& rh) const;
        bool operator < (const PointInTime& rh) const;
        bool operator >= (const PointInTime& rh) const;
        bool operator <= (const PointInTime& rh) const;
        bool operator == (const PointInTime& rh) const;
        bool operator != (const PointInTime& rh) const;
    };
}
