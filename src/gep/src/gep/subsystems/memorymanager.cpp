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


template<typename T>
static void reduceToSmallestUnitWithDecimalSuffix(T& out_number, const char*& out_szSuffix)
{
    static const char* suffixes[] = {
        "",
        "K",
        "M",
        "G",
        "T",
        "P",
        "E",
        "Z",
        "Y",
    };

    int index = 0;
    while(out_number >= (T)1000)
    {
        ++index;
        out_number /= (T)1000;
    }
    GEP_ASSERT(index < GEP_ARRAY_SIZE(suffixes), "Number too large.", out_number);
    out_szSuffix = suffixes[index];
}

template<typename T>
static void reduceToSmallestUnitWithBinarySuffix(T& out_number, const char*& out_szSuffix)
{
    static const char* suffixes[] = {
        "",
        "Ki",
        "Mi",
        "Gi",
        "Ti",
        "Pi",
        "Ei",
        "Zi",
        "Yi",
    };

    int index = 0;
    while(out_number >= (T)1024)
    {
        ++index;
        out_number /= (T)1024;
    }
    GEP_ASSERT(index < GEP_ARRAY_SIZE(suffixes), "Number too large.", out_number);
    out_szSuffix = suffixes[index];
}


void gep::MemoryManager::update(float elapsedTime)
{
    //TODO draw statistics here


    m_message << "Allocators:\n";
    for (auto& allocatorInfo : m_allocators)
    {
        const char* suffix = nullptr; // "Ki", or "G", etc.
        auto pAllocator = allocatorInfo.pAllocator;

        {
            auto numAllocations = pAllocator->getNumAllocations();
            reduceToSmallestUnitWithDecimalSuffix(numAllocations, suffix);
            m_message << '+' << numAllocations << ' ' << suffix;
        }

        {
            auto numFrees = pAllocator->getNumFrees();
            reduceToSmallestUnitWithDecimalSuffix(numFrees, suffix);
            m_message << " | -" << numFrees << ' ' << suffix;
        }

        {
            auto numAlive = pAllocator->getNumAllocations() - pAllocator->getNumFrees();
            reduceToSmallestUnitWithDecimalSuffix(numAlive, suffix);
            m_message << " | =" << numAlive << ' ' << suffix;
        }

        {
            auto numBytesUsed = pAllocator->getNumBytesUsed();
            reduceToSmallestUnitWithBinarySuffix(numBytesUsed, suffix);
            m_message << " | " << numBytesUsed << ' ' << suffix << 'B';
        }

        {
            auto numBytesReserved = pAllocator->getNumBytesReserved();
            if(numBytesReserved > 0)
            {
                reduceToSmallestUnitWithBinarySuffix(numBytesReserved, suffix);
                m_message << " / " << numBytesReserved << ' ' << suffix << 'B';
            }
        }

        m_message << " | " << allocatorInfo.name << '\n';
    }
    g_globalManager.getRenderer()->getDebugRenderer().printText(vec2(-0.95f, 0.0f),
                                                                m_message.str().c_str());
    // Clear the string stream
    m_message.clear(); // clears only special internal flags
    m_message.str(std::string()); // empties the stream content

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

