
#pragma once

#include "gep/gepmodule.h"
#include "gep/threading/thread.h"
#include "gep/threading/semaphore.h"
#include "gep/container/DynamicArray.h"
#include "gep/container/Queue.h"
#include "gep/types.h"
#include <functional>

namespace gep
{
    // forward declarations
    class TaskQueue;

    /// \brief interface for a task
    class ITask
    {
    public:
        /// \brief executes the task
        virtual void execute() = 0;
    };

    class StandardTask : public ITask
    {
        std::function<void()> m_executable;
    public:
        StandardTask(const std::function<void()>& executable) : m_executable(executable)
        {
            GEP_ASSERT(m_executable, "Executable object is invalid.");
        }

        virtual ~StandardTask() {}

        virtual void execute() override { if(m_executable) m_executable(); }
    };

    /// \brief Groups together tasks which can be executed in parallel
    class GEP_API TaskGroup
    {
        friend class TaskQueue;
        friend class TaskWorker;
    private:
        bool m_isExecuting;
        TaskQueue* m_pTaskQueue;
        std::function<void(ArrayPtr<ITask*>)> m_finishedCallback;
        volatile uint32 m_numRemainingTasks;
        DynamicArray<ITask*> m_tasks;

        TaskGroup(TaskQueue* pTaskQueue);

        void reset();
        void taskFinished();

    public:
        /// \brief adds a task to the task group
        void addTask(ITask* pTask);

        /// \brief sets a function which should be called when the task group finished
        inline void setOnFinished(std::function<void(ArrayPtr<ITask*>)> onFinished)
        {
            GEP_ASSERT(!m_isExecuting);
            m_finishedCallback = onFinished;
        }

    };

    /// \brief worker which executes a single task at a time
    /// each worker has its own task queue
    /// if the task queue is empty it tries to steal tasks from other workers
    class GEP_API TaskWorker
        : public Thread
    {
        friend class TaskQueue;
    private:
        TaskQueue* m_pTaskQueue;
        TaskGroup* m_pActiveGroup;
        Semaphore m_hasWorkSemaphore;
        DynamicArray<ITask*> m_tasks;
        Mutex m_tasksMutex;

        inline void setActiveGroup(TaskGroup* pGroup) { m_pActiveGroup = pGroup; }
        void addTasks(ArrayPtr<ITask*> tasks);

        // runs a single task
        Result runSingleTask();
        // tries to steal tasks from other task workers
        Result stealTasks();
        // runs tasks until there is no more work
        void runTasks();

    public:
        TaskWorker(TaskQueue* pTaskQueue);

        virtual void run() override;
    };

    /// \brief Manages tasks
    class GEP_API TaskQueue
    {
        friend class TaskWorker;
        friend class TaskGroup;
    private:
        bool m_isRunning;
        TaskGroup* m_currentTaskGroup;
        Mutex m_schedulingMutex;
        Queue<TaskGroup*> m_remainingTaskGroups;
        DynamicArray<TaskGroup*> m_unusedTaskGroups;
        DynamicArray<TaskWorker*> m_worker;
        TaskWorker m_localWorker;

        void scheduleNextGroup();
    public:
        TaskQueue();
        ~TaskQueue();

        /// \brief creates a new task group
        TaskGroup* createGroup();

        /// \brief deletes a task group (previously created with createGroup)
        void deleteGroup(TaskGroup*);

        /// \brief schedules a task group for execution, it should at least contain 1 task
        void scheduleForExecution(TaskGroup* group);

        /// \brief tries to run a single task from the queue
        /// \return SUCCESS if successfull, FAILURE otherwise
        Result runSingleTask();

        /// \brief runs tasks until there is no more work
        void runTasks();

        /// \brief stops execution of tasks, blocks until all tasks are stopped
        void stop();
    };
}
