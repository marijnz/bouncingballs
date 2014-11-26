#pragma once
#include "gep/container/DynamicArray.h"

namespace gep
{
    class HavokErrorHandler : public hkDefaultError
    {
    public:
        HavokErrorHandler() :
            hkDefaultError(nullptr) {}

        virtual int message(hkError::Message msg, int id, const char* description, const char* file, int line) override;
    };

    namespace hk
    {
        void initialize();
        void shutdown();
    }
}
