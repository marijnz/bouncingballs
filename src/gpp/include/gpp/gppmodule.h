#pragma once
/**
 * Holds all definitions neccessary for the Game Engine Programming DLL Module
 */

// prevent min & max macros
#define NOMINMAX

#ifdef GPP_EXPORTS
    #define GPP_API __declspec(dllexport)
#else
    #define GPP_API __declspec(dllimport)
#endif
