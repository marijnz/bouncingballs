#pragma once

#include "gep/math3d/constants.h"
#include <cmath>

namespace gep
{
    /// Helper struct for floor
    template <class T>
    struct CalcFloor
    {
        inline static const T floor(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcFloor<T::component_t>::floor(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcFloor<float>
    {
        inline static float floor(const float value) { return ::floorf(value); }
        typedef float result_t;
    };

    template <>
    struct CalcFloor<double>
    {
        inline static double floor(const double value) { return ::floor(value); }
        typedef double result_t;
    };

    /// Helper struct for ceil
    template <class T>
    struct CalcCeil
    {
        inline static const T ceil(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcCeil<T::component_t>::ceil(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcCeil<float>
    {
        inline static float ceil(const float value) { return ::ceilf(value); }
        typedef float result_t;
    };

    template <>
    struct CalcCeil<double>
    {
        inline static double ceil(const double value) { return ::ceil(value); }
        typedef double result_t;
    };

    /// Helper struct for abs
    template <class T>
    struct CalcAbs
    {
        inline static const T abs(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcAbs<T::component_t>::ceil(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcAbs<float>
    {
        inline static float abs(const float value)
        {
            return fabs(value);
        }
        typedef float result_t;
    };

    template <>
    struct CalcAbs<double>
    {
        inline static double abs(const double value)
        {
            return abs(value);
        }
        typedef double result_t;
    };

    template <class T>
    struct CalcSin
    {
        inline static const T sin(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcSin<T::component_t>::sin(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcSin<float>
    {
        inline static const float sin(const float value)
        {
            return sinf(value);
        }
        typedef float result_t;
    };

    template <>
    struct CalcSin<double>
    {
        inline static const double sin(const double value)
        {
            return sin(value);
        }
        typedef double result_t;
    };

    template <class T>
    struct CalcCos
    {
        inline static const T cos(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcCos<T::component_t>::cos(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcCos<float>
    {
        inline static const float cos(const float value)
        {
            return cosf(value);
        }
        typedef float result_t;
    };

    template <>
    struct CalcCos<double>
    {
        inline static const double cos(const double value)
        {
            return cos(value);
        }
        typedef double result_t;
    };

    template <class T>
    struct CalcACos
    {
        inline static const T acos(const T& value)
        {
            typename T::component_t temp[GEP_ARRAY_SIZE(value.data)];
            for(size_t i=0; i < GEP_ARRAY_SIZE(value.data); i++)
                temp[i] = CalcCos<T::component_t>::acos(value.data[i]);
            return T(temp);
        }
        typedef T result_t;
    };

    template <>
    struct CalcACos<float>
    {
        inline static const float acos(const float value)
        {
            return acosf(value);
        }
        typedef float result_t;
    };

    template <>
    struct CalcACos<double>
    {
        inline static const double acos(const double value)
        {
            return acos(value);
        }
        typedef double result_t;
    };

    ///helper function for sqrt
    template <typename T>
    struct CalcSqrt
    {
        inline static float sqrt(const T value) { return sqrtf((float)value); }
        typedef float result_t;
    };

    template <>
    struct CalcSqrt<double>
    {
        inline static double sqrt(const double value) { return sqrt(value); }
        typedef double result_t;
    };

    // helper struct for epsilonCompare
    template <class T>
    struct CalcEpsilonCompare
    {
        inline static bool epsilonCompare(const T& lh, const T& rh)
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(lh.data); i++)
                if(!CalcEpsilonCompare<T::component_t>::epsilonCompare(lh.data[i], rh.data[i]))
                    return false;
            return true;
        }
    };

    template <>
    struct CalcEpsilonCompare<float>
    {
        inline static bool epsilonCompare(const float lh, const float rh)
        {
            return (lh - GetEpsilon<float>::value() < rh && lh + GetEpsilon<float>::value() > rh);
        }
    };

    template <>
    struct CalcEpsilonCompare<double>
    {
        inline static bool epsilonCompare(const double lh, const double rh)
        {
            return (lh - GetEpsilon<double>::value() < rh && lh + GetEpsilon<double>::value() > rh);
        }
    };

    template <class T>
    struct ConvertAngleUnits
    {
        typedef float result_t;

        /// \brief Converts degrees to radians
        inline static float toRadians(const T degrees)
        {
            return (float)degrees / 180.0f * GetPi<result_t>::value();
        }

        /// \brief Converts radians to degrees
        inline static float toDegrees(const T radians)
        {
            return (float)degrees / GetPi<result_t>::value() * 180.0f;
        }
    };

    template <>
    struct ConvertAngleUnits<double>
    {
        typedef double result_t;

        inline static double toRadians(const double degrees)
        {
            return degrees / 180.0 * GetPi<double>::value();
        }

        /// \brief Converts radians to degrees
        inline static double toDegrees(const double radians)
        {
            return radians / GetPi<double>::value() * 180.0;
        }
    };

    ///generic floor function
    template <class T>
    inline const typename CalcFloor<T>::result_t floor(const T& value)
    {
        return CalcFloor<T>::floor(value);
    }

    ///generic ceil function
    template <class T>
    inline const typename CalcCeil<T>::result_t ceil(const T& value)
    {
        return CalcCeil<T>::ceil(value);
    }

    ///generic abs function
    template <class T>
    inline const typename CalcAbs<T>::result_t abs(const T& value)
    {
        return CalcAbs<T>::abs(value);
    }

    ///generic epsilon compare
    template <class T>
    inline bool epsilonCompare(const T& lh, const T& rh)
    {
        return CalcEpsilonCompare<T>::epsilonCompare(lh, rh);
    }

    ///generic check if lh is zero
    template <class T>
    inline bool isZero(const T& lh)
    {
        return CalcEpsilonCompare<T>::epsilonCompare(lh, (T)0);
    }

    ///generic check if lh is not zero
    template <class T>
    inline bool isNonZero(const T& lh)
    {
        return !isZero(lh);
    }

    ///generic sin function
    template <class T>
    inline const typename CalcSin<T>::result_t sin(const T& value)
    {
        return CalcSin<T>::sin(value);
    }

    ///generic cos function
    template <class T>
    inline const typename CalcCos<T>::result_t cos(const T& value)
    {
        return CalcCos<T>::cos(value);
    }

    ///generic acos function
    template <class T>
    inline const typename CalcACos<T>::result_t acos(const T& value)
    {
        return CalcACos<T>::acos(value);
    }

    ///generic sqrt function
    template <typename T>
    inline const typename CalcSqrt<T>::result_t sqrt(const T& value)
    {
        return CalcSqrt<T>::sqrt(value);
    }

    template<typename T>
    inline const typename ConvertAngleUnits<T>::result_t toRadians(const T degrees)
    {
        return ConvertAngleUnits<T>::toRadians(degrees);
    }

    template<typename T>
    inline const typename ConvertAngleUnits<T>::result_t toDegrees(const T radians)
    {
        return ConvertAngleUnits<T>::toDegrees(radians);
    }

    template<typename T>
    inline const T clamp(T value, T min, T max)
    {
        if(value < min) return min;
        if(value > max) return max;
        return value;
    }
};
