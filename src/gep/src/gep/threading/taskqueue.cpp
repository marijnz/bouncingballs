#include "stdafx.h"
#include "gep/threading/taskQueue.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include <thread>

gep::TaskGroup::TaskGroup(TaskQueue* pTaskQueue) :
    m_isExecuting(false),
    m_pTaskQueue(pTaskQueue)
{

}

void gep::TaskGroup::addTask(ITask* pTask)
{
    GEP_ASSERT(!m_isExecuting, "can not add tasks while executing");
    m_tasks.append(pTask);
}

void gep::TaskGroup::reset()
{
    m_numRemainingTasks = 0;
    m_isExecuting = false;
    m_tasks.resize(0);
    m_finishedCallback = nullptr;
}

void gep::TaskGroup::taskFinished()
{
    uint32 tasksRemainingInGroup = InterlockedDecrement(&m_numRemainingTasks);
    if(tasksRemainingInGroup == 0)
    {
        m_isExecuting = false;
        if(m_finishedCallback)
            m_finishedCallback(m_tasks.toArray());
        m_pTaskQueue->scheduleNextGroup();
    }
}

gep::TaskWorker::TaskWorker(TaskQueue* pTaskQueue) :
    m_pTaskQueue(pTaskQueue),
    m_hasWorkSemaphore(0),
    m_pActiveGroup(nullptr)
{
}

void gep::TaskWorker::addTasks(ArrayPtr<ITask*> tasks)
{
    ScopedLock<Mutex> lock(m_tasksMutex);
    m_tasks.append(tasks);
}

void gep::TaskWorker::runTasks()
{
    do
    {
        while(m_pTaskQueue->m_isRunning && runSingleTask() == SUCCESS) {}
    }
    while(m_pTaskQueue->m_isRunning && stealTasks() == SUCCESS);
}

void gep::TaskWorker::run()
{
    try {
        while(m_pTaskQueue->m_isRunning)
        {
            m_hasWorkSemaphore.waitAndDecrement();
            if(!m_pTaskQueue->m_isRunning)
                break;
            runTasks();
        }
    }
    catch(std::exception& ex)
    {
        g_globalManager.getLogging()->logError("A task worker was killed due to a unhandeled exception:\n%s", ex.what());
    }
}

gep::Result gep::TaskWorker::runSingleTask()
{
    ITask* pTaskToExecute = nullptr;
    {
        // try to get a task from our internal task list
        ScopedLock<Mutex> lock(m_tasksMutex);
        if(m_tasks.length() == 0)
            return FAILURE;
        pTaskToExecute = m_tasks.lastElement();
        m_tasks.resize(m_tasks.length() - 1);
    }

    pTaskToExecute->execute();
    m_pActiveGroup->taskFinished();
    return SUCCESS;
}

gep::Result gep::TaskWorker::stealTasks()
{
    // try to steal tasks from all other task workers but ourself
    for(TaskWorker* pOtherWorker : m_pTaskQueue->m_worker)
    {
        if(pOtherWorker != this)
        {
            ScopedLock<Mutex> lock(pOtherWorker->m_tasksMutex);
            if(pOtherWorker->m_tasks.length() > 0)
            {
                // steal half of the tasks, but at least 1
                size_t tasksToSteal = pOtherWorker->m_tasks.length() / 2;
                if(tasksToSteal < 1)
                    tasksToSteal = 1;
                auto otherTasks = pOtherWorker->m_tasks.toArray();
                size_t otherNewLength = otherTasks.length() - tasksToSteal;
                // Append a sub array to our task queue
                m_tasks.append( otherTasks(otherNewLength, otherTasks.length()) );
                // remove the stolen tasks from the other task queue
                pOtherWorker->m_tasks.resize(otherNewLength);
                return SUCCESS;
            }
        }
    }
    return FAILURE;
}

gep::TaskQueue::TaskQueue()
    : m_localWorker(this),
    m_currentTaskGroup(nullptr),
    m_isRunning(true)
{
    TaskWorker* localWorker = &m_localWorker;
    m_worker.append(localWorker);
    for(size_t i=1; i < 8/*std::thread::hardware_concurrency()*/; i++)
    {
        TaskWorker* newWorker = new TaskWorker(this);
        newWorker->start();
        m_worker.append(newWorker);
    }
}

gep::TaskQueue::~TaskQueue()
{
    if(m_isRunning)
        stop();
    for(size_t i=1; i < m_worker.length(); i++)
    {
        m_worker[i]->join();
        delete m_worker[i];
    }
    for(auto group : m_unusedTaskGroups)
    {
        delete group;
    }
    while(m_remainingTaskGroups.count() > 0)
    {
        delete m_remainingTaskGroups.take();
    }
}

gep::TaskGroup* gep::TaskQueue::createGroup()
{
    TaskGroup* result = nullptr;
    if(m_unusedTaskGroups.length() > 0)
    {
        result = m_unusedTaskGroups.lastElement();
        m_unusedTaskGroups.resize(m_unusedTaskGroups.length() - 1);
        return result;
    }
    result = new TaskGroup(this);
    return result;
}

void gep::TaskQueue::deleteGroup(TaskGroup* pGroup)
{
    if(pGroup != nullptr)
        m_unusedTaskGroups.append(pGroup);
}

void gep::TaskQueue::scheduleForExecution(TaskGroup* pGroup)
{
    GEP_ASSERT(pGroup->m_tasks.length() > 0, "there are no tasks in the group");
    pGroup->m_isExecuting = true;
    pGroup->m_numRemainingTasks = (uint32)pGroup->m_tasks.length();

    ScopedLock<Mutex> lock(m_schedulingMutex);
    m_remainingTaskGroups.append(pGroup);
    if(m_currentTaskGroup == nullptr)
    {
        scheduleNextGroup();
    }
}

void gep::TaskQueue::scheduleNextGroup()
{
    ScopedLock<Mutex> lock(m_schedulingMutex);
    if(m_remainingTaskGroups.count() > 0)
    {
        m_currentTaskGroup = m_remainingTaskGroups.take();
        size_t numTasks = m_currentTaskGroup->m_tasks.length();
        size_t tasksPerWorker = numTasks / m_worker.length();
        if(tasksPerWorker < 1)
            tasksPerWorker = 1;
        size_t taskStart = 0;
        auto taskArray = m_currentTaskGroup->m_tasks.toArray();
        // set the active task group on all workers
        for(auto pWorker : m_worker)
        {
            pWorker->m_pActiveGroup = m_currentTaskGroup;
        }
        // distribute the tasks to the workers
        for(auto pWorker : m_worker)
        {
            if(taskStart >= numTasks)
                break;
            size_t end = taskStart + tasksPerWorker;
            if(end > numTasks)
                end = numTasks;
            pWorker->addTasks( taskArray(taskStart, end) );
            taskStart += tasksPerWorker;
        }

        // wakeup all the workers but the first (the first is the local worker)
        for(size_t i=1; i < m_worker.length(); i++)
            m_worker[i]->m_hasWorkSemaphore.increment();
    }
    else
    {
        m_currentTaskGroup = nullptr;
    }
}

gep::Result gep::TaskQueue::runSingleTask()
{
    return m_localWorker.runSingleTask();
}

void gep::TaskQueue::runTasks()
{
    m_localWorker.runTasks();
}

void gep::TaskQueue::stop()
{
    m_isRunning = false;
    // signal all the workers to wake up if needed
    for(size_t i=1; i < m_worker.length(); i++)
    {
        m_worker[i]->m_hasWorkSemaphore.increment();
        m_worker[i]->join();
    }
}
