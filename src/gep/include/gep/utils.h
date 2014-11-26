#pragma once

#include <string>
#include <varargs.h>

namespace gep
{
    GEP_API std::string format(GEP_PRINTF_FORMAT_STRING const char* fmt, ...);
    GEP_API std::string vformat(GEP_PRINTF_FORMAT_STRING const char* fmt, va_list args);

    GEP_API std::wstring convertToWideString(const std::string& toConvert);
}
