#pragma once

#include "gep/math3d/vec3.h"
#include "gep/math3d/algorithm.h"

namespace gep
{
    //Forward declarations
    template <typename T>
    struct Ray_t;

    /// \brief a plane in 3d
    template <typename T>
    struct Plane_t {
        // the plane equation (a * x + b * y + c * z - d = 0)
        vec3_t<T> normal;
        T distanceFromOrigin;

        /// \brief constructor
        /// \param pos a point on the plane
        /// \param dir the normal of the plane
        Plane_t(const vec3_t<T>& pos, vec3_t<T> normal){
            normal = normal.normalized();
            this->normal = normal;
            this->distanceFromOrigin = pos.dot(normal);
        }

        /// \brief construct from three points
        Plane_t(const vec3_t<T>& v1, const vec3_t<T>& v2, const vec3_t<T>& v3){
            vec3_t<T> dir1 = v2 - v1;
            vec3_t<T> dir2 = v3 - v1;
            vec3_t<T> normal = dir1.cross(dir2).normalized();
            this->normal = normal;
            this->distanceFromOrigin = v1.dot(normal);
        }

        /// \brief Constructor
        /// \param x x part of normal
        /// \param y y part of normal
        /// \param z z part of normal
        /// \param d distance from origin
        Plane_t(T x, T y, T z, T d){
            normal = vec3_t<T>(x, y, z);
            distanceFromOrigin = d;
        }

        /// \brief computes the distance from a given point to the plane
        T distance(const vec3_t<T>& point) const {
            return normal.dot(point) - distanceFromOrigin;
        }

        /// \brief computes the intersection point of this and 2 other planes
        /// \param p2 first other plane
        /// \param p3 second other plane
        /// \return the intersection point (float.nan in all 3 components if there is more then 1 intersection point)
        const vec3_t<T>& intersectWith(const Plane_t<T>& p2, const Plane_t<T>& p3) const {
            vec3_t<T> result(std::numeric_limits<T>::quiet_NAN());
            T d = normal.x*p2.normal.y*p3.normal.z + normal.y*p2.normal.z*p3.normal.x + normal.z*p2.normal.x*p3.normal.y - p3.normal.x*p2.normal.y*normal.z - p3.normal.y*p2.normal.z*normal.x - p3.normal.z*p2.normal.x*normal.y;
            if(!epsilonCompare(d, 0))
            {
                result.x = distanceFromOrigin*p2.normal.y*p3.normal.z + normal.y*p2.normal.z*p3.distanceFromOrigin + normal.z*p2.distanceFromOrigin*p3.normal.y - p3.distanceFromOrigin*p2.normal.y*normal.z - p3.normal.y*p2.normal.z*distanceFromOrigin - p3.normal.z*p2.distanceFromOrigin*normal.y;
                result.y = normal.x*p2.distanceFromOrigin*p3.normal.z + distanceFromOrigin*p2.normal.z*p3.normal.x + normal.z*p2.normal.x*p3.distanceFromOrigin - p3.normal.x*p2.distanceFromOrigin*normal.z - p3.distanceFromOrigin*p2.normal.z*normal.x - p3.normal.z*p2.normal.x*distanceFromOrigin;
                result.z = normal.x*p2.normal.y*p3.distanceFromOrigin + normal.y*p2.distanceFromOrigin*p3.normal.x + distanceFromOrigin*p2.normal.x*p3.normal.y - p3.normal.x*p2.normal.y*distanceFromOrigin - p3.normal.y*p2.distanceFromOrigin*normal.x - p3.distanceFromOrigin*p2.normal.x*normal.y;

                result.x /= d;
                result.y /= d;
                result.z /= d;
            }

            return result;
        }

        /**
        * computes the intersection of this plane and a other one
        * Params:
        *  other = the other plane to intersect with
        * Returns: the intersection ray
        */
        const Ray_t<T> intersectWith(const Plane_t<T>& other) const;

        /// \brief returns a normalized copy of the plane
        const Plane_t<T> normalized() const {
            T length = normal.length();
            return Plane_t<T>(normal / length, distanceFromOrigin / length);
        }
    };

    typedef Plane_t<float> Plane;

}

// Implementations with cyclic dependencies
#include "gep/math3d/ray.h"

namespace gep
{
    template <typename T>
    const Ray_t<T> Plane_t<T>::intersectWith(const Plane_t<T>& other) const {
        T dot = this->normal.dot(other.normal);
        if(abs(dot) >= 1.0f - GetEpsilon<T>::value())
        {
            //the planes are paralell
            return Ray(vec3_t<T>(std::numeric_limits<T>::quiet_NaN()),
                vec3_t<T>(std::numeric_limits<T>::quiet_NaN()));
        }

        T invDet = 1 / (1 - dot * dot);
        T cThis =  (this->distanceFromOrigin - dot * other.distanceFromOrigin) * invDet;
        T cOther = (other.distanceFromOrigin - dot * this->distanceFromOrigin) * invDet;
        return Ray(this->normal * cThis + other.normal * cOther, this->normal.cross(other.normal).normalized());
    }
}
