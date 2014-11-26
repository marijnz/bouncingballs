#pragma once

namespace gep
{
    //the result type
    enum Result
    {
        FAILURE = 0,
        SUCCESS = 1
    };

    //don't initialize type
    enum DoNotInitialize
    {
      DO_NOT_INITIALIZE
    };

    enum class TakeOwnership
    {
        no,
        yes
    };

    // integer types
    typedef unsigned char uint8;
    typedef char int8;
    typedef unsigned short uint16;
    typedef short int16;
    typedef unsigned int uint32;
    typedef int int32;
    typedef unsigned long long uint64;
    typedef long long int64;

    //test integer types
    static_assert(sizeof(uint8) == 1, "uint8 is not 8 bit wide");
    static_assert(sizeof(int8) == 1, "int8 is not 8 bit wide");
    static_assert(sizeof(uint16) == 2, "uint16 is not 16 bit wide");
    static_assert(sizeof(int16) == 2, "int16 is not 16 bit wide");
    static_assert(sizeof(int32) == 4, "int32 is not 32 bit wide");
    static_assert(sizeof(uint32) == 4, "uint32 is not 32 bit wide");
    static_assert(sizeof(int64) == 8, "int64 is not 64 bit wide");
    static_assert(sizeof(uint64) == 8, "uint64 is not 64 bit wide");
};
