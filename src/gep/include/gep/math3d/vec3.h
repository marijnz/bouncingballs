#pragma once

#include "gep/math3d/constants.h"
#include "gep/math3d/vec2.h"

#include "gep/interfaces/scripting.h"

namespace gep
{
    template <typename T>
    struct vec3_t {
        union {
            struct {
                T x,y,z;
            };
            T data[3];
        };

        typedef T component_t;
        static const size_t dimension = 3;

        /// \brief default constructor
        inline vec3_t()
        {
            this->x = 0.0f;
            this->y = 0.0f;
            this->z = 0.0f;
        }

        /// \brief constructor for a unitialized instance
        inline vec3_t(DoNotInitialize) {}

        /// \brief constructor
        inline vec3_t(T x, T y, T z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        /// \brief constructor
        inline explicit vec3_t(T data[3])
        {
            this->data[0] = data[0];
            this->data[1] = data[1];
            this->data[2] = data[2];
        }

        /// \brief constructor
        inline explicit vec3_t(const T xyz)
        {
            this->x = xyz;
            this->y = xyz;
            this->z = xyz;
        }

        /// \brief constructor
        inline vec3_t(vec2_t<T> xy, T z)
        {
            this->x = xy.x;
            this->y = xy.y;
            this->z = z;
        }

        /// \brief + operator for adding another 3 component vector
        inline const vec3_t<T> operator + (const vec3_t<T>& rh) const
        {
            return vec3_t<T>(this->x + rh.x,
                this->y + rh.y,
                this->z + rh.z);
        }

        /// \brief - operator for subtracting another 3 component vector
        inline const vec3_t<T> operator - (const vec3_t<T>& rh) const
        {
            return vec3_t<T>(this->x - rh.x,
                this->y - rh.y,
                this->z - rh.z);
        }

        /// \brief * operator for multiplying with a scalar
        inline const vec3_t<T> operator * (const T rh) const
        {
            return vec3_t<T>(this->x * rh,
                this->y * rh,
                this->z * rh);
        }

        /// \brief * operator for multipying with another vector (component vise)
        inline const vec3_t<T> operator * (const vec3_t<T>& rh) const
        {
            return vec3_t<T>(this->x * rh.x,
                this->y * rh.y,
                this->z * rh.z);
        }

        /// \brief / operator for dividing by a scalar
        inline const vec3_t<T> operator / (const T rh) const
        {
            return vec3_t<T>(this->x / rh,
                this->y / rh,
                this->z / rh);
        }

        /// \brief / operator for dividing witha nother vector (component vise)
        inline const vec3_t<T> operator / (const vec3_t<T>& rh) const
        {
            return vec3_t<T>(this->x / rh.x,
                this->y / rh.y,
                this->z / rh.z);
        }

        /// \brief += operator
        inline vec3_t<T>& operator += (const vec3_t<T>& rh)
        {
            this->x += rh.x;
            this->y += rh.y;
            this->z += rh.z;
            return *this;
        }

        /// \brief -= operator
        inline vec3_t<T>& operator -= (const vec3_t<T>& rh)
        {
            this->x -= rh.x;
            this->y -= rh.y;
            this->z -= rh.z;
            return *this;
        }

        /// \brief *= operator (scalar)
        inline vec3_t<T>& operator *= (const T rh)
        {
            this->x *= rh;
            this->y *= rh;
            this->z *= rh;
            return *this;
        }

        /// \brief *= operator (component vise)
        inline vec3_t<T>& operator *= (const vec3_t<T>& rh)
        {
            this->x *= rh.x;
            this->y *= rh.y;
            this->z *= rh.z;
            return *this;
        }

        /// \brief /= operator (scalar)
        inline vec3_t<T>& operator /= (const T rh)
        {
            this->x /= rh;
            this->y /= rh;
            this->z /= rh;
            return *this;
        }

        /// \brief /= operator (component vise)
        inline vec3_t<T>& operator /= (const vec3_t<T>& rh)
        {
            this->x /= rh.x;
            this->y /= rh.y;
            this->z /= rh.z;
            return *this;
        }

        /// \brief unary - operator
        inline const vec3_t<T> operator - () const
        {
            return vec3_t<T>(-x,-y,-z);
        }

        /// \brief Sets all components to the specified value
        inline void set(T x, T y, T z)
        {
            this->x = x;
            this->y = y;
            this->z = z;
        }

        /// \brief comparing two instances with an epsilon
        inline bool epsilonCompare (const vec3_t<T>& rh, const T e = GetEpsilon<T>::value()) const
        {
            return ( (this->x - e <= rh.x && this->x + e >= rh.x) &&
                (this->y - e <= rh.y && this->y + e >= rh.y) &&
                (this->z - e <= rh.z && this->z + e >= rh.z) );
        }

        /// \brief returns the length of the vector
        inline typename CalcSqrt<T>::result_t length() const
        {
            return CalcSqrt<T>::sqrt(x*x + y*y + z*z);
        }

        /// \brief returns the squared length of the vector
        inline T squaredLength() const
        {
            return x*x + y*y + z*z;
        }

        /// \brief computes the dot product of this and another 3 component vector
        inline T dot(const vec3_t<T>& rh) const
        {
            return this->x * rh.x + this->y * rh.y + this->z * rh.z;
        }

        /// \brief computes the cross product of this and another 3 component vector
        inline const vec3_t<T> cross(const vec3_t<T>& rh) const
        {
            vec3_t<T> res;
            res.x = this->y * rh.z - this->z * rh.y;
            res.y = this->z * rh.x - this->x * rh.z;
            res.z = this->x * rh.y - this->y * rh.x;
            return res;
        }

        /// \brief returns a normalized version of this vector
        inline const vec3_t<T> normalized() const
        {
            auto len = this->length();
            GEP_ASSERT(isNonZero(len), "Cannot normalize a zero-length vec3!");
            return (*this) / len;
        }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(length)
            LUA_BIND_FUNCTION(squaredLength)
            LUA_BIND_FUNCTION(normalized)
            LUA_BIND_FUNCTION_NAMED(negated, "negated")
            LUA_BIND_FUNCTION_NAMED(negated, "__unm")
            LUA_BIND_FUNCTION_NAMED(addFromScript, "add")
            LUA_BIND_FUNCTION_NAMED(addFromScript, "__add")
            LUA_BIND_FUNCTION_NAMED(subFromScript, "sub")
            LUA_BIND_FUNCTION_NAMED(subFromScript, "__sub")
            LUA_BIND_FUNCTION_NAMED(mulFromScript, "mul")
            LUA_BIND_FUNCTION_NAMED(mulFromScript, "__mul")
            LUA_BIND_FUNCTION_NAMED(mulScalarFromScript, "mulScalar")
            LUA_BIND_FUNCTION_NAMED(divFromScript, "div")
            LUA_BIND_FUNCTION_NAMED(divFromScript, "__div")
            LUA_BIND_FUNCTION_NAMED(divScalarFromScript, "divScalar")
            LUA_BIND_FUNCTION(cross)
            LUA_BIND_FUNCTION(dot)
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(x)
            LUA_BIND_MEMBER(y)
            LUA_BIND_MEMBER(z)
        LUA_BIND_VALUE_TYPE_END

    private:
        const vec3_t<T> negated(const vec3_t<T>& rh) const { return -*this; }
        const vec3_t<T> addFromScript(const vec3_t<T>& rh) const { return *this + rh; }
        const vec3_t<T> subFromScript(const vec3_t<T>& rh) const { return *this - rh; }
        const vec3_t<T> mulFromScript(const vec3_t<T>& rh) const { return *this * rh; }
        const vec3_t<T> mulScalarFromScript(T rh) const { return *this * rh; }
        const vec3_t<T> divFromScript(const vec3_t<T>& rh) const { return *this / rh; }
        const vec3_t<T> divScalarFromScript(T rh) const { return *this / rh; }
    };

    typedef vec3_t<float> vec3;
    typedef vec3_t<int> ivec3;
};
