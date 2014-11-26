#include "stdafx.h"
#include "gep/ReferenceCounting.h"

void* gep::ReferenceCounted::operator new(size_t size)
{
    GEP_ASSERT(false, "should not be called");
    return nullptr;
}

void* gep::ReferenceCounted::operator new(size_t size, void* pWhere)
{
    return pWhere;
}
