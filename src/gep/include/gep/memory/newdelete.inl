// implement overloading of new/delete here
// strip-start "NewDeleteOverload"
void* operator new(size_t size)
{
    return g_stdAllocator.allocateMemory(size);
}

void* operator new[](size_t size)
{
    return g_stdAllocator.allocateMemory(size);
}

void operator delete(void* mem) throw()
{
    return g_stdAllocator.freeMemory(mem);
}

void operator delete[](void* mem)
{
    return g_stdAllocator.freeMemory(mem);
}
// strip-end
