#pragma once

#include "gep/math3d/vec3.h"
#include "gep/math3d/algorithm.h"
#include "gep/math3d/ray.h"

namespace gep
{
    /// \brief a sphere
    template <typename T>
    struct Sphere_t
    {
        vec3_t<T> pos;
        T radiusSquared;

        /// \brief constructor
        Sphere_t(vec3_t<T> pos, T radius)
        {
            this->pos = pos;
            this->radiusSquared = radius * radius;
        }

        // returns the radius of the sphere
        T radius() const
        {
            return gep::sqrt(radiusSquared);
        }

        /// \brief
        ///   Tests if this sphere interects with a given ray
        bool intersects(const Ray_t<T>& ray) const
        {
            //Compute A, B and C coefficients
            vec3 offset = ray.pos - pos;
            float a = ray.dir.dot(ray.dir);
            float b = ray.dir.dot(offset);
            float c = offset.dot(offset) - radiusSquared;

            //Find discriminant
            float disc = b * b - a * c;

            // if discriminant is negative there are no real roots, so return
            // false as ray misses the Sphere
            return (disc >= 0.0f);
        }

        /// \brief computes the nearest intersection with a ray
        ///
        /// \param ray
        ///   the ray to intersect with
        ///
        /// \param distanceOnRay
        ///   the distance on the ray to the intersection
        bool computeNearestIntersection(const Ray_t<T>& ray, T& distanceOnRay) const
        {
            //Compute A, B and C coefficients
            vec3_t<T> offset = ray.pos - pos;
            T a = ray.dir.dot(ray.dir);
            T b = 2 * ray.dir.dot(offset);
            T c = offset.dot(offset) - radiusSquared;

            //Find discriminant
            T disc = b * b - 4 * a * c;

            // if discriminant is negative there are no real roots, so return
            // false as ray misses Sphere_t<T>
            if (disc < 0)
                return false;

            T discSqrt = gep::sqrt(disc);

            T t0 = (-b - discSqrt) / (2 * a);
            T t1 = (-b + discSqrt) / (2 * a);

            // make sure t0 is smaller than t1
            if (t0 > t1)
            {
                // if t0 is bigger than t1 swap them around
                swap(t0, t1);
            }

            // if t1 is less than zero, the object is in the ray's negative direction
            // and consequently the ray misses the Sphere_t<T>
            if (t1 < 0)
                return false;

            // if t0 is less than zero, the intersection point is at t1
            if (t0 < 0)
            {
                distanceOnRay = t1;
                return true;
            }
            // else the intersection point is at t0
            distanceOnRay = t0;
            return true;
        }

        /// \brief checks if this sphere contains another sphere
        bool contains(const Sphere_t<T>& sphere)
        {
            T a = ((sphere.pos - pos).length() + sphere.radius());
            T b = radius();
            return a <= b;
        }
    };

    typedef Sphere_t<float> Sphere;
}
