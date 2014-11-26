#include "stdafx.h"
#include "gep/file.h"

bool gep::fileExists(const char* name)
{
    DWORD attributes = GetFileAttributesA(name);
    return (attributes != 0xFFFFFFFF &&
            !(attributes & FILE_ATTRIBUTE_DIRECTORY));
}
