#pragma once

#include "gep/math3d/vec3.h"

namespace gep
{
    template<size_t dim>
    struct numVerticesHelper
    {
        static const size_t value = numVerticesHelper<dim-1>::value * 2;
    };

    template <>
    struct numVerticesHelper<1>
    {
        static const size_t value = 2;
    };

    /// \brief a axis aligned box
    template <typename T>
    struct AxisAlignedBox_t
    {
    private:
        T m_min,m_max;

    public:

        /// \brief default constructor
        inline AxisAlignedBox_t() {}

        /// \brief no initialization constructor
        inline AxisAlignedBox_t(DoNotInitialize arg) : m_min(DO_NOT_INITIALIZE), m_max(DO_NOT_INITIALIZE) {}

        /// \brief constructor
        ///  \param m_min
        ///    the m_minimum bounds of the box
        ///  \param m_max
        ///    the m_maximum bounds of the box
        inline AxisAlignedBox_t(const T& m_min, const T& m_max)
        {
            this->m_min = m_min;
            this->m_max = m_max;
            GEP_ASSERT(isValid(), "invalid bounding box");
        }

        /// \brief returns the m_minimum bounds
        inline const T getMin() const { return m_min; }

        /// \brief returns the m_maximum bounds
        inline const T getMax() const { return m_max; }

        /// \brief returns the size of the box
        const T size() const
        {
            return (m_max - m_min);
        }

        /// \brief computes all corner vertices of the box
        /// \param vertices
        ///   the resulting vertices (out)
        void getVertices(T (&vertices)[numVerticesHelper<T::dimension>::value]) const
        {
            for(size_t i=0; i<GEP_ARRAY_SIZE(vertices); i++)
            {
                size_t mod = 2;
                for(size_t el=0; el < T::dimension; el++, mod *= 2)
                {
                    vertices[i].data[el] = ((i % mod) < mod / 2) ? m_min.data[el] : m_max.data[el];
                }
            }
        }

        /// \brief moves the bounding box by a given amount
        const AxisAlignedBox_t<T> move(const T& amount) const
        {
            return AxisAlignedBox_t<T>(m_min+rh, m_max+rh);
        }

        /// \brief checks if a point is within the box
        bool contains(const T& rh) const
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
            {
                if(!(m_min.data[i] < rh.data[i] && rh.data[i] < m_max.data[i]))
                    return false;
            }
            return true;
        }

        /// \brief checks if a another bounding is completely inside
        bool contains(const AxisAlignedBox_t<T>& rh) const
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(rh.m_min.data[i] < m_max.data[i]))
                    return false;
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(rh.m_min.data[i] > m_min.data[i]))
                    return false;
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(rh.m_max.data[i] < m_max.data[i]))
                    return false;
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(rh.m_max.data[i] > m_min.data[i]))
                    return false;
            return true;
        }

        /// \brief checks if this bounding box intersects another one
        bool intersects(const AxisAlignedBox_t<T>& rh) const
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(m_min.data[i] <= rh.m_max.data[i] && m_max.data[i] >= rh.m_min.data[i]))
                    return false;
            return true;
        }

        /// \brief helper method to check if the bounding box is valid
        bool isValid() const
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(m_min.data); i++)
                if(!(m_min.data[i] < m_max.data[i]))
                    return false;
            return true;
        }
    };

    typedef AxisAlignedBox_t<vec3> AABB;
}
