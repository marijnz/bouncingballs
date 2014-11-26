// stdafx.h : Includedatei f�r Standardsystem-Includedateien
// oder h�ufig verwendete projektspezifische Includedateien,
// die nur in unregelm��igen Abst�nden ge�ndert werden.
//

#pragma once

#define GEP_UNITTESTS

// disable security warnings
#define _CRT_SECURE_NO_WARNINGS

#include <string>

#include "gep/gepmodule.h"
#include "gep/common.h"
#include "gep/memory/allocator.h"
#include "gep/ArrayPtr.h"

#include "gep/unittest/UnittestManager.h"
#include <Windows.h>

#include "gpp/gppmodule.h"
