#pragma once

#include <functional>

namespace gep
{
    struct CallbackId
    {
        size_t id;

        inline CallbackId(size_t id) : id(id) {}
    };

    inline bool operator == (const CallbackId& lhs, const CallbackId& rhs)
    {
        return lhs.id == rhs.id;
    }

    class IUpdateFramework
    {
    public:
        virtual ~IUpdateFramework(){}

        virtual void stop() = 0;
        virtual void run() = 0;
        virtual float getElapsedTime() const = 0;
        virtual float calcElapsedTimeAverage(size_t numFrames) const = 0;
        virtual CallbackId registerUpdateCallback(std::function<void(float elapsedTime)> callback) = 0;
        virtual void deregisterUpdateCallback(CallbackId id) = 0;

        virtual CallbackId registerInitializeCallback(std::function<void()> callback) = 0;
        virtual CallbackId registerDestroyCallback(std::function<void()> callback) = 0;
        virtual void deregisterInitializeCallback(CallbackId id) = 0;
        virtual void deregisterDestroyCallback(CallbackId id) = 0;
    };
}
