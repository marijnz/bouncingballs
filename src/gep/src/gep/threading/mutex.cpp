#include "stdafx.h"
#include "gep/threading/mutex.h"
#include <Windows.h>

gep::Mutex::Mutex()
{
    InitializeCriticalSection(&m_criticalSection);
}

gep::Mutex::~Mutex()
{
    DeleteCriticalSection(&m_criticalSection);
}

void gep::Mutex::lock()
{
    EnterCriticalSection(&m_criticalSection);
}

void gep::Mutex::unlock()
{
    LeaveCriticalSection(&m_criticalSection);
}

gep::Result gep::Mutex::tryLock()
{
    return TryEnterCriticalSection(&m_criticalSection) ? SUCCESS : FAILURE;
}
