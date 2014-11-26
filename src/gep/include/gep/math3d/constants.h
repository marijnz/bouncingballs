#pragma once

namespace gep
{

    template <typename T>
    struct GetEpsilon
    {
    };

    template <>
    struct GetEpsilon<float>
    {
        inline static float value() { return 0.01f; }
    };

    template <>
    struct GetEpsilon<double>
    {
        inline static double value() { return 0.00001; }
    };

    template <>
    struct GetEpsilon<int>
    {
        inline static int value() { return 0; }
    };

    template <>
    struct GetEpsilon<unsigned int>
    {
        inline static int value() { return 0; }
    };

    template <typename T>
    struct GetPi
    {
    };

    template <>
    struct GetPi<float>
    {
        inline static float value()
        {
            return 3.14159265359f;
        }
    };

    template <>
    struct GetPi<double>
    {
        inline static double value()
        {
            return 3.14159265358979323846264338327950288419716;
        }
    };
};
