#include "stdafx.h"
#include "gep/utils.h"

std::string gep::format(const char* fmt, ...)
{
    va_list argptr;
    va_start(argptr, fmt);
    SCOPE_EXIT{ va_end(argptr); });
    return vformat(fmt, argptr);
}

std::string gep::vformat(GEP_PRINTF_FORMAT_STRING const char* fmt, va_list argptr)
{
    static const size_t maxAllocationSize(1024 * 1024 * 10); // 10 MB
    static const size_t bufferSize(1024);

    char buffer[bufferSize];

    int result = vsnprintf(buffer, bufferSize, fmt, argptr);

    if(result < 0)
    {
        for(size_t allocationSize = 2 * bufferSize;
            allocationSize < maxAllocationSize;
            allocationSize *= 2)
        {
            char* largeBuffer = new char[allocationSize];
            SCOPE_EXIT{ delete[] largeBuffer; });

            result = vsnprintf(largeBuffer, allocationSize, fmt, argptr);
            if(result >= 0)
            {
                return std::string(largeBuffer);
            }
        }
        GEP_ASSERT(false, "The string you want to format is too large! "
                   "It did not even fit into %u characters.",
                   maxAllocationSize);
    }

    return std::string(buffer);
}

std::wstring gep::convertToWideString(const std::string& toConvert)
{
    // Implementation taken from http://stackoverflow.com/a/18597384/1282221
    std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
    return converter.from_bytes(toConvert);
}
