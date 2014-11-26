#include "stdafx.h"
#include "gep/comleakfinder.h"

gep::ComLeakFinder* volatile gep::DoubleLockingSingleton<gep::ComLeakFinder>::s_instance;
gep::Mutex gep::DoubleLockingSingleton<gep::ComLeakFinder>::s_creationMutex;

gep::ComLeakFinder::ComLeakFinder()
{
}

gep::ComLeakFinder::~ComLeakFinder()
{
    ScopedLock<Mutex> lock(m_mutex);

    if(m_trackedInstances.count() > 0)
    {
        FILE* f = fopen("ComLeaks.log", "w");
        auto kit = m_trackedInstances.keys().begin();
        auto kend = m_trackedInstances.keys().end();
        auto vit = m_trackedInstances.values().begin();
        while(kit != kend)
        {
            auto k = (*kit);
            auto& v = (*vit);
            fprintf(f, "leaked instance 0x%x\n", k);
            for(auto& t : v.traces)
            {
                fprintf(f, "\nTrace: ");
                switch(t.type)
                {
                case TraceType::initial:
                    fprintf(f, "initial\n");
                    break;
                case TraceType::addRef:
                    fprintf(f, "AddRef\n");
                    break;
                case TraceType::release:
                    fprintf(f, "Release\n");
                    break;
                }
                #define LINE_LENGTH 256
                char outBuf[LINE_LENGTH * GEP_ARRAY_SIZE(t.addresses)];
                StackWalker::resolveCallstack(ArrayPtr<StackWalker::address_t>(t.addresses, t.numAddresses), outBuf, LINE_LENGTH);
                for(uint16 i=0; i<t.numAddresses; i++)
                {
                    fprintf(f, "%s\n", outBuf + (LINE_LENGTH * i));
                }
            }
            delete[] v.newVptr;
            fprintf(f, "=========================================\n");
            fflush(f);
            ++kit;
            ++vit;
        }
        fclose(f);
    }
}

ULONG gep::ComLeakFinder::hookAddRef(IUnknown* self)
{
    auto& inst = ComLeakFinder::instance();
    ScopedLock<Mutex> lock(inst.m_mutex);

    GEP_ASSERT(inst.m_trackedInstances.exists(self), "object is not tracked");

    auto& info = inst.m_trackedInstances[self];
    info.refCount++;

    info.traces.resize(info.traces.length() + 1);
    auto& t = info.traces[info.traces.length() - 1];
    t.refCount = info.refCount;
    t.numAddresses = (uint16)StackWalker::getCallstack(1, ArrayPtr<StackWalker::address_t>(t.addresses));
    t.type = TraceType::addRef;

    return info.AddRef(self);
}

ULONG gep::ComLeakFinder::hookRelease(IUnknown* self)
{
    auto& inst = ComLeakFinder::instance();
    ScopedLock<Mutex> lock(inst.m_mutex);

    GEP_ASSERT(inst.m_trackedInstances.exists(self), "object is not tracked");

    auto& info = inst.m_trackedInstances[self];
    info.refCount--;

    info.traces.resize(info.traces.length() + 1);
    auto& t = info.traces[info.traces.length() - 1];
    t.refCount = info.refCount;
    t.numAddresses = (uint16)StackWalker::getCallstack(1, ArrayPtr<StackWalker::address_t>(t.addresses));
    t.type = TraceType::release;

    if(info.refCount == 0)
    {
        auto h = reinterpret_cast<Unknown*>(self);
        delete[] h->vptr;
        h->vptr = info.oldVptr;
        inst.m_trackedInstances.remove(self);
    }
    return info.Release(self);
}

void gep::ComLeakFinder::trackComObject(IUnknown* pObject)
{
    ScopedLock<Mutex> lock(m_mutex);

    // Do not add it twice
    if(m_trackedInstances.exists(pObject))
        return;

    //Check if the reference count is 1
    pObject->AddRef();
    ULONG count = pObject->Release();
    GEP_ASSERT(count == 1, "reference count of com object is not 1", count);

    auto h = reinterpret_cast<Unknown*>(pObject);

    // copy the vtable
    size_t vtableSize = 0;
    while(h->vptr[vtableSize] != nullptr && vtableSize < 64)
    {
        vtableSize++;
    }
    GEP_ASSERT(vtableSize >= 3);
    void** newVtable = new void*[vtableSize];
    memcpy(newVtable, h->vptr, sizeof(void*) * vtableSize);

    //patch the methods
    newVtable[1] = &hookAddRef;
    newVtable[2] = &hookRelease;

    auto& info = m_trackedInstances[pObject] = InterfaceData(h->vptr,
                                                            newVtable,
                                                            reinterpret_cast<AddRefFunc>(h->vptr[1]),
                                                            reinterpret_cast<ReleaseFunc>(h->vptr[2]));
    h->vptr = newVtable;

    // do the stacktrace
    info.traces.resize(1);
    auto& t = info.traces[0];
    t.numAddresses = (uint16)StackWalker::getCallstack(0, ArrayPtr<StackWalker::address_t>(t.addresses));
    t.refCount = info.refCount;
    t.type = TraceType::initial;
}
