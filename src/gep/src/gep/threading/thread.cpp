#include "stdafx.h"
#include "gep/threading/thread.h"
#include <process.h>

void __cdecl gep::threadEntryFunction(void* pThread)
{
    static_cast<Thread*>(pThread)->run();
    static_cast<Thread*>(pThread)->m_handle = nullptr;
}

gep::Thread::Thread() :
    m_handle(nullptr)
{
}

gep::Thread::~Thread()
{
    GEP_ASSERT(m_handle == nullptr, "thread is still running");
}

void gep::Thread::start()
{
    m_handle = (HANDLE)_beginthread(&threadEntryFunction, 0, (void*)this);
}

void gep::Thread::join()
{
    if(m_handle != nullptr)
    {
        WaitForSingleObject( m_handle, INFINITE );
    }
}
