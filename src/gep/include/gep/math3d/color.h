#pragma once
#include "gep/interfaces/scripting.h"

namespace gep
{
    template <typename T>
    struct Color_t
    {
        inline static Color_t<T> black()   { return Color_t<T>(0.0f, 0.0f, 0.0f, 1.0f); }
        inline static Color_t<T> white()   { return Color_t<T>(1.0f, 1.0f, 1.0f, 1.0f); }
        inline static Color_t<T> red()     { return Color_t<T>(1.0f, 0.0f, 0.0f, 1.0f); }
        inline static Color_t<T> yellow()  { return Color_t<T>(1.0f, 1.0f, 0.0f, 1.0f); }
        inline static Color_t<T> green()   { return Color_t<T>(0.0f, 1.0f, 0.0f, 1.0f); }
        inline static Color_t<T> cyan()    { return Color_t<T>(0.0f, 1.0f, 1.0f, 1.0f); }
        inline static Color_t<T> blue()    { return Color_t<T>(0.0f, 0.0f, 1.0f, 1.0f); }
        inline static Color_t<T> magenta() { return Color_t<T>(1.0f, 0.0f, 1.0f, 1.0f); }

        union
        {
            struct { float r, g, b, a; };
            float data[4];
        };

        typedef T component_t;

        /// \brief default constructor
        inline Color_t()
        {
            this->r = 0;
            this->g = 0;
            this->b = 0;
            this->a = 0;
        }

        /// \brief constructor
        inline Color_t(const T r, const T g, const T b, const T a)
        {
            this->r = r;
            this->g = g;
            this->b = b;
            this->a = a;
        }

        /// \brief constructor for an uninitialized instance
        inline Color_t(DoNotInitialize) {}

        /// \brief + operator for adding another color
        inline const Color_t<T> operator + (const Color_t<T>& rh) const
        {
            return Color_t<T>(this->r + rh.r,
                this->g + rh.g,
                this->b + rh.b,
                this->a + rh.a);
        }

        /// \brief - operator for subtracting another color
        inline const Color_t<T> operator - (const Color_t<T>& rh) const
        {
            return Color_t<T>(this->r - rh.r,
                this->g - rh.g,
                this->b - rh.b,
                this->a - rh.a);
        }

        /// \brief * operator for multiplying with a scalar
        inline const Color_t<T> operator * (const float rh) const
        {
            return Color_t<T>(this->r * f,
                this->g * f,
                this->b * f,
                this->a * f);
        }

        /// \brief * operator for multipying with another color (component wise)
        inline const Color_t<T> operator * (const Color_t<T>& rh) const
        {
            return Color_t<T>(this->r * rh.r,
                this->g * rh.g,
                this->b * rh.b,
                this->a * rh.a);
        }

        /// \brief / operator for dividing by a scalar
        inline const Color_t<T> operator / (const float rh) const
        {
            return Color_t<T>(this->r / rh,
                this->g / rh,
                this->b / rh,
                this->a / rh);
        }

        /// \brief / operator for dividing with another color (component wise)
        inline const Color_t<T> operator / (const Color_t<T>& rh) const
        {
            return Color_t<T>(this->r / rh.r,
                this->g / rh.g,
                this->b / rh.b,
                this->a / rh.a);
        }

        /// \brief += operator
        inline Color_t<T>& operator += (const Color_t<T>& rh)
        {
            this->r += rh.r;
            this->g += rh.g;
            this->b += rh.b;
            this->a += rh.a;
            return *this;
        }

        /// \brief -= operator
        inline Color_t<T>& operator -= (const Color_t<T>& rh)
        {
            this->r -= rh.r;
            this->g -= rh.g;
            this->b -= rh.b;
            this->a -= rh.a;
            return *this;
        }

        /// \brief *= operator (scalar)
        inline Color_t<T>& operator *= (const float rh)
        {
            this->r *= f;
            this->g *= f;
            this->b *= f;
            this->a *= f;
            return *this;
        }

        /// \brief *= operator (component wise)
        inline Color_t<T>& operator *= (const Color_t<T>& rh)
        {
            this->r *= rh.r;
            this->g *= rh.g;
            this->b *= rh.b;
            this->a *= rh.a;
            return *this;
        }

        /// \brief /= operator (scalar)
        inline Color_t<T>& operator /= (const float rh)
        {
            this->r /= rh;
            this->g /= rh;
            this->b /= rh;
            this->a /= rh;
            return *this;
        }

        /// \brief /= operator (component wise)
        inline Color_t<T>& operator /= (const Color_t<T>& rh)
        {
            this->r /= rh.r;
            this->g /= rh.g;
            this->b /= rh.b;
            this->a /= rh.a;
            return *this;
        }

        /// \brief unary - operator
        inline const Color_t<T> operator - () const
        {
            return Color_t<T>(-r, -g, -b, -a);
        }

        /// \brief == operator
        inline bool operator == (const Color_t<T>& rh) const
        {
            return (this->r == rh.r && this->g == rh.g && this->b == rh.b && this->a == rh.a);
        }

        /// \brief != operator
        inline bool operator != (const Color_t<T>& rh) const
        {
            return ! operator == (rh);
        }
		
        LUA_BIND_VALUE_TYPE_BEGIN
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(r)
            LUA_BIND_MEMBER(g)
            LUA_BIND_MEMBER(b)
            LUA_BIND_MEMBER(a)
        LUA_BIND_VALUE_TYPE_END
    };

    typedef Color_t<float> Color;
};
