#pragma once

#include "gep/traits.h"

namespace gep
{
    namespace MemoryUtils
    {
        /// generic version
        template <bool, class T>
        struct uninitializedConstructImpl
        {
            inline static void uninitializedConstruct(T* dst, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    new (dst+i) T();
            }
        };

        /// primitive type version (int, float, pointers, etc)
        template <class T>
        struct uninitializedConstructImpl<true, T>
        {
            inline static void uninitializedConstruct(T* dst, size_t count)
            {
            }
        };

        /// \brief default constructs count instances of T in dst. Dst should be unitialized
        template <class T>
        void uninitializedConstruct(T* dst, size_t count)
        {
            uninitializedConstructImpl<std::is_fundamental<T>::value || std::is_compound<T>::value && std::is_trivially_constructible<T>::value, T>
                ::uninitializedConstruct(dst, count);
        }

        // non pod case
        template <bool, class T>
        struct uninitializedCopyImpl
        {
            static void uninitializedCopy(T* dst, const T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    new (dst+i) T(src[i]);
            }
        };

        // pod case
        template <class T>
        struct uninitializedCopyImpl<true, T>
        {
            static void uninitializedCopy(T* dst, const T* src, size_t count)
            {
                if(dst + count < src || dst > src + count)
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief copy constructs count instances of T in dst from src. Dst is uninitialized
        template <class T>
        void uninitializedCopy(T* dst, const T* src, size_t count)
        {
            uninitializedCopyImpl<isPod<T>::value, T>::uninitializedCopy(dst, src, count);
        }

        //non-pod case
        template <bool, class T>
        struct uninitializedMoveImpl
        {
            static void uninitializedMove(T* dst, T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    new (dst+i) T(std::move(src[i]));
            }
        };

        //pod case
        template <class T>
        struct uninitializedMoveImpl<true, T>
        {
            static void uninitializedMove(T* dst, T* src, size_t count)
            {
                if(dst + count < src || dst > src + count)
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief move constructs count instaces of T in dst from src. Dst is uninitialized
        template <class T>
        void uninitializedMove(T* dst, T* src, size_t count)
        {
            uninitializedMoveImpl<isPod<T>::value, T>::uninitializedMove(dst, src, count);
        }

        /// \brief destroys count instances of T in dst
        template <class T>
        void destroy(T* dst, size_t count)
        {
            for(size_t i=0; i<count; i++)
                (dst+i)->~T();
        }

        // non pod case
        template <bool, class T>
        struct copyImpl
        {
            static void copy(T* dst, const T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    dst[i] = src[i];
            }
        };

        // pod case
        template <class T>
        struct copyImpl<true, T>
        {
            static void copy(T* dst, const T* src, size_t count)
            {
                if(dst + count < src && dst > src + count)
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief copies count instances from src to dst. Dst is initialized
        template <class T>
        void copy(T* dst, const T* src, size_t count)
        {
            copyImpl<isPod<T>::value, T>::copy(dst, src, count);
        }

        //non pod case
        template <bool, class T>
        struct moveImpl
        {
            static void move(T* dst, const T* src, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    dst[i] = std::move(src[i]);
            }
        };

        //pod case
        template <class T>
        struct moveImpl<true, T>
        {
            static void move(T* dst, const T* src, size_t count)
            {
                if(dst + count < src || dst > src + count)
                    memcpy(dst, src, sizeof(T) * count);
                else
                    memmove(dst, src, sizeof(T) * count);
            }
        };

        /// \brief moves count instances from src to dst.
        template <class T>
        void move(T* dst, const T* src, size_t count)
        {
            moveImpl<isPod<T>::value, T>::move(dst, src, count);
        }

        // non pod version
        template <bool, class T>
        struct equalsImpl
        {
            static bool equals(const T* lh, const T* rh, size_t count)
            {
                for(size_t i=0; i<count; i++)
                    if(!(lh[i] == rh[i]))
                        return false;
                return true;
            }
        };

        // pod version
        template <class T>
        struct equalsImpl<true, T>
        {
            static bool equals(const T* lh, const T* rh, size_t count)
            {
                return (memcmp(lh, rh, sizeof(T) * count) == 0);
            }
        };


        /// \brief checks if lh is equal to rh in count entries
        template <class T>
        bool equals(const T* lh, const T* rh, size_t count)
        {
            return equalsImpl<isPod<T>::value, T>::equals(lh, rh, count);
        }
    }
}
