#pragma once

#include "gep/threading/mutex.h"
#include "gep/memory/allocator.h"
#include "gep/exit.h"

#pragma warning( disable : 4661 )

namespace gep
{
    /// \brief a singleton which uses the double locking pattern
    template <class T, class AllocatorPolicy = StdAllocatorPolicy>
    class DoubleLockingSingleton
    {
    private:
        static T* volatile s_instance;
        static Mutex s_creationMutex;

        DoubleLockingSingleton(const DoubleLockingSingleton<T>& other); //non copyable
        void operator = (const DoubleLockingSingleton<T>& rh); //non copyable
        DoubleLockingSingleton(DoubleLockingSingleton<T>&& other); // non moveable
        void operator = (DoubleLockingSingleton<T>&& rh){}; //non

    protected:
        DoubleLockingSingleton() {}
        virtual ~DoubleLockingSingleton() {}

    public:
        /// \brief returns the only instance
        static T& instance()
        {
            if(s_instance == nullptr)
            {
                ScopedLock<Mutex> lock(s_creationMutex);
                if(s_instance == nullptr)
                {
                    s_instance = GEP_NEW(AllocatorPolicy::getAllocator(), T)();
                    gep::atexit(&destroyInstance);
                }
            }
            return *s_instance;
        }

        static void destroyInstance()
        {
            if(s_instance != nullptr)
            {
                auto temp = s_instance;
                s_instance = nullptr;
                temp->~T();
                AllocatorPolicy::getAllocator()->freeMemory((void*)temp);
            }
        }
    };
}
