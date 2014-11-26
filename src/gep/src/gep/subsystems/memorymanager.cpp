#include "stdafx.h"
#include "gepimpl/subsystems/memoryManager.h"
#include "gep/memory/allocator.h"

#include "gep/globalManager.h"
#include "gep/interfaces/renderer.h"

void gep::MemoryManager::initialize()
{

}

void gep::MemoryManager::destroy()
{
    GEP_ASSERT(m_allocators.length() == 0, "not all allocators have been deregistered");
}

void gep::MemoryManager::update(float elapsedTime)
{
    //TODO draw statistics here


    auto& debugRenderer = g_globalManager.getRenderer()->getDebugRenderer();
    std::stringstream message;
    message << "Allocators:\n";
    for (auto& allocatorInfo : m_allocators)
    {
        auto pAllocator = allocatorInfo.pAllocator;
        message << allocatorInfo.name            << " | +"
            << pAllocator->getNumAllocations()   << " | -"
            << pAllocator->getNumFrees()         << " | "
            << pAllocator->getNumBytesUsed()     << 'B';

        if (pAllocator->getNumBytesReserved() > 0)
        {
            message << " / " << pAllocator->getNumBytesReserved() << 'B';
        }

        message << '\n';
    }
    debugRenderer.printText(vec2(-0.95f, 0.0f), message.str().c_str());

}

void gep::MemoryManager::registerAllocator(const char* name, IAllocatorStatistics* pAllocator)
{
    m_allocators.append(AllocatorInfo(name, pAllocator));
}

void gep::MemoryManager::deregisterAllocator(IAllocatorStatistics* pAllocator)
{
    size_t i=0;
    for(; i<m_allocators.length(); i++)
    {
        if(m_allocators[i].pAllocator == pAllocator)
            break;
    }
    if(i < m_allocators.length())
        m_allocators.removeAtIndex(i);
}

