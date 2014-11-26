#pragma once

#include "gep/math3d/vec3.h"
#include <limits>

namespace gep
{
    //Forwad declarations
    template <typename T>
    struct Plane_t;

    /// \brief a ray in 3d
    template <typename T>
    struct Ray_t {
        vec3_t<T> pos; ///> point on the ray
        vec3_t<T> dir; ///> direction of the ray

        /// \brief constructor
        /// \param pos
        ///   start position of the ray
        /// \param dir
        ///   direction of the ray
        Ray_t(const vec3_t<T>& pos, const vec3_t<T>& dir) : pos(pos), dir(dir)
        {
        }

        /// \brief Creates a ray using two points
        /// \param p1 first point on the ray
        /// \param p2 second point on the ray
        static const Ray_t<T> createFromPoints(const vec3_t<T>& p1, const vec3_t<T>& p2)
        {
            return Ray_t<T>(p1, p2-p1);
        }

        /// \brief intersects this ray with a plane
        /// \param p the plane to intersect with
        /// \return the intersection distance on the ray
        T intersectWith(const Plane_t<T>& p) const;

        /// \brief gets a point on the ray
        /// \param f the distance on the ray to get the point for
        /// \return the computed position
        const vec3_t<T> get(T f) const {
            return pos + dir * f;
        }

        /// \brief returns the end of the ray
        const vec3_t<T> end() const {
            return pos + dir;
        }
    };

    typedef Ray_t<float> Ray;
}

// Implementations with cyclic dependencies
#include "gep/math3d/plane.h"

namespace gep
{
    template <typename T>
    T Ray_t<T>::intersectWith(const Plane_t<T>& p) const
    {
        T d = p.m_Eq.x * dir.x + p.m_Eq.y * dir.y + p.m_Eq.z * dir.z;
        if(epsilonCompare(d, 0)){
            return std::numeric_limits<T>::quiet_NaN();
        }
        return (p.m_Eq.w - p.m_Eq.x * pos.x - p.m_Eq.y * pos.y - p.m_Eq.z * pos.z) / d;
    }
}
