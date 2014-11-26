#include "stdafx.h"
#include "gpp/stringUtils.h"

GPP_API void gpp::stringSplit(const std::string& stringToSplit,
                              char splitChar,
                              std::string& out_left,
                              std::string& out_right)
{
    const size_t begin = stringToSplit.find_first_of(splitChar);
    GEP_ASSERT(begin != std::string::npos,
               "There is no splitChar in stringToSplit!",
               splitChar,
               stringToSplit);

    const size_t end = stringToSplit.find_first_of(splitChar, begin + 1);

    if (end == std::string::npos)
    {
        // out_left is the rest
        out_left = stringToSplit.substr(begin + 1, end);
        out_right = "";
        return;
    }
    
    out_left = stringToSplit.substr(begin + 1, end - 1);
    out_right = stringToSplit.substr(end);
}
