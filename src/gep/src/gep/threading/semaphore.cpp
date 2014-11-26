#include "stdafx.h"
#include "gep/threading/semaphore.h"

gep::Semaphore::Semaphore(uint8 startCount)
{
    m_handle = CreateSemaphore(
    nullptr,           // default security attributes
    startCount,        // initial count
    255,               // maximum count
    nullptr);          // unnamed semaphore
}

gep::Semaphore::~Semaphore()
{
    CloseHandle(m_handle);
}

void gep::Semaphore::waitAndDecrement()
{
    DWORD waitResult = WaitForSingleObject(m_handle, INFINITE);
    GEP_ASSERT(waitResult == WAIT_OBJECT_0);
}

gep::Result gep::Semaphore::waitAndDecrement(uint32 millisecondsToWait)
{
    DWORD waitResult = WaitForSingleObject(m_handle, millisecondsToWait);
    return (waitResult == WAIT_OBJECT_0) ? SUCCESS : FAILURE;
}

void gep::Semaphore::increment()
{
    ReleaseSemaphore(m_handle, 1, nullptr);
}
