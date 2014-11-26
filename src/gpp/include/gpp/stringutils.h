#pragma once
#include <string>

namespace gpp
{
    GPP_API void stringSplit(const std::string& stringToSplit,
                             char splitChar,
                             std::string& out_left,
                             std::string& out_right);
}
