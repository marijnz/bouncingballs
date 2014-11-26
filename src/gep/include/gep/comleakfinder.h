#pragma once

#include <gep/container/hashmap.h>
#include <gep/container/DynamicArray.h>
#include <gep/singleton.h>
#include <gep/stackWalker.h>
#include <gep/threading/mutex.h>

namespace gep
{

    /// \brief
    ///   A helper class that helps to find leaks of reference counted com objects which inherit from IUnknown
    class GEP_API ComLeakFinder : public DoubleLockingSingleton<ComLeakFinder>
    {
    private:
        typedef ULONG (__stdcall *AddRefFunc)(IUnknown* self);
        typedef ULONG (__stdcall *ReleaseFunc)(IUnknown* self);

        enum class TraceType : uint16
        {
            initial,
            addRef,
            release
        };

        struct trace
        {
            StackWalker::address_t addresses[16];
            uint16 numAddresses;
            uint16 refCount;
            TraceType type;
        };

        struct InterfaceData
        {
            void** oldVptr;
            void** newVptr;
            DynamicArray<trace> traces;
            uint32 refCount;
            AddRefFunc AddRef;
            ReleaseFunc Release;

            InterfaceData() {}

            InterfaceData(void** oldVptr, void** newVptr, AddRefFunc addRef, ReleaseFunc release)
                : oldVptr(oldVptr)
                , newVptr(newVptr)
                , refCount(1)
                , AddRef(addRef)
                , Release(release)
            {

            }
        };

        struct Unknown
        {
            void** vptr;
        };

        Hashmap<IUnknown*, InterfaceData> m_trackedInstances;
        Mutex m_mutex;

        static ULONG __stdcall hookAddRef(IUnknown* self);
        static ULONG __stdcall hookRelease(IUnknown* self);
    public:

        ComLeakFinder();

        /// \brief Destructors. Writes a ComLeaks.log file if leaks are found.
        ~ComLeakFinder();

        /// \brief adds the given object for tracking
        void trackComObject(IUnknown* pObject);
    };

}
