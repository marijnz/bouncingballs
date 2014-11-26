#include "stdafx.h"
#include <Windows.h>

#ifndef GPP_EXPORTS
#error "GPP_EXPORTS is not defined"
#endif

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    return TRUE;
}
