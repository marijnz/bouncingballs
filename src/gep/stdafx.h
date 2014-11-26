#pragma once

// prevent min & max macros
#define NOMINMAX

#include <cmath>
#include <fstream>
#include <exception>
#include <locale>
#include <codecvt>
#include <string>
#include <windows.h>

//fix directX include clash with windows 8 sdk
#include <winerror.h>
#undef DXGI_STATUS_OCCLUDED
#undef DXGI_STATUS_CLIPPED
#undef DXGI_STATUS_NO_REDIRECTION
#undef DXGI_STATUS_NO_DESKTOP_ACCESS
#undef DXGI_STATUS_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_STATUS_MODE_CHANGED
#undef DXGI_STATUS_MODE_CHANGE_IN_PROGRESS
#undef DXGI_ERROR_INVALID_CALL
#undef DXGI_ERROR_NOT_FOUND
#undef DXGI_ERROR_MORE_DATA
#undef DXGI_ERROR_UNSUPPORTED
#undef DXGI_ERROR_DEVICE_REMOVED
#undef DXGI_ERROR_DEVICE_HUNG
#undef DXGI_ERROR_DEVICE_RESET
#undef DXGI_ERROR_WAS_STILL_DRAWING
#undef DXGI_ERROR_FRAME_STATISTICS_DISJOINT
#undef DXGI_ERROR_GRAPHICS_VIDPN_SOURCE_IN_USE
#undef DXGI_ERROR_DRIVER_INTERNAL_ERROR
#undef DXGI_ERROR_NONEXCLUSIVE
#undef DXGI_ERROR_NOT_CURRENTLY_AVAILABLE
#undef DXGI_ERROR_REMOTE_CLIENT_DISCONNECTED
#undef DXGI_ERROR_REMOTE_OUTOFMEMORY
#undef D3D11_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D11_ERROR_FILE_NOT_FOUND
#undef D3D11_ERROR_TOO_MANY_UNIQUE_VIEW_OBJECTS
#undef D3D11_ERROR_DEFERRED_CONTEXT_MAP_WITHOUT_INITIAL_DISCARD
#undef D3D10_ERROR_TOO_MANY_UNIQUE_STATE_OBJECTS
#undef D3D10_ERROR_FILE_NOT_FOUND

#include <d3d11.h>
#include <D3DX11.h>

#include <XInput.h>


// Some havok configuration stuff.
#include "gepimpl/havok/config.h"
#include "gepimpl/subsystems/physics/havok/config.h"
#include "gepimpl/subsystems/animation/havok/config.h"

// Havok related headers.
#include "gepimpl/havok/includes.h"
#include "gepimpl/subsystems/physics/havok/includes.h"
#include "gepimpl/subsystems/animation/havok/includes.h"

// Havok features setup.
#include "gepimpl/havok/features.h"


#include "gep/gepmodule.h"
#include "gep/common.h"
#include "gep/memory/allocator.h"
#include "gep/ArrayPtr.h"
