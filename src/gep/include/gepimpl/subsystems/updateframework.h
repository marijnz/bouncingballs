#pragma once

#include "gep/interfaces/updateFramework.h"
#include "gep/ArrayPtr.h"
#include "gep/container/dynamicArray.h"
#include "gep/timer.h"
#include "gep/threading/thread.h"
#include "gep/threading/semaphore.h"



namespace gep
{
    // forward declarations
    class UpdateFramework;

    /// \brief thread which runs the game simulation
    class GameThread : public Thread
    {
        friend class UpdateFramework;
    private:
        Semaphore m_gameStartLock;
        Semaphore m_gameEndLock;
        Semaphore m_gameDestroyLock;
        bool m_execute;
        UpdateFramework* m_pUpdateFramework;
        float m_elapsedTime;

    public:
        GameThread(UpdateFramework* pUpdateFramework);

        virtual void run() override;
    };

    class UpdateFramework
        : public IUpdateFramework
    {
    private:
        float m_pFrameTimesArray[60];
        ArrayPtr<float> m_FrameTimesPtr;
        size_t m_frameIdx;
        bool m_running;
        GameThread m_gameThread;

        DynamicArray<std::function<void(float elapsedTime)>> m_toUpdate;

        DynamicArray<std::function<void()>> m_toInitialize;
        DynamicArray<std::function<void()>> m_toDestroy;

        PointInTime m_timeOfLastFrame;

    public:
        UpdateFramework();

        // IUpdateFramework interface
        virtual void stop() override;
        virtual void run() override;
        virtual float getElapsedTime() const override;
        virtual float calcElapsedTimeAverage(size_t numFrames) const override;
        virtual CallbackId registerUpdateCallback(std::function<void(float elapsedTime)> callback) override;
        virtual void deregisterUpdateCallback(CallbackId id) override;

        virtual CallbackId registerInitializeCallback(std::function<void()> callback) override;
        virtual CallbackId registerDestroyCallback(std::function<void()> callback) override;
        virtual void deregisterInitializeCallback(CallbackId id) override;
        virtual void deregisterDestroyCallback(CallbackId id) override;

        void runGame(float elapsedTime);


        void initializeGame();
        void destroyGame();



    };
}
