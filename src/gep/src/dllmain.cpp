#include "stdafx.h"
#include <Windows.h>

#ifndef GEP_EXPORTS
#error "GEP_EXPORTS is not defined"
#endif

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
