#pragma once

#include <type_traits>

namespace gep
{
    template <class T> struct ArrayPtr;

    template <bool>
    struct BooleanResult : public std::false_type
    {
    };

    template <>
    struct BooleanResult<true> : public std::true_type
    {
    };

    /// \brief helper template that checks if a type is a POD (plain old data) type or not
    ///
    /// Access the result via isPod<type>::value
    template <class T>
    struct isPod : public BooleanResult<std::is_trivially_copyable<T>::value>
    {
    };

    template <class T>
    struct isPod<ArrayPtr<T>> : public BooleanResult<true>
    {
    };

    template <class T>
    struct isArrayPtr : public BooleanResult<false>
    {
    };

    template <class T>
    struct isArrayPtr<ArrayPtr<T>> : public BooleanResult<true>
    {
    };
};
