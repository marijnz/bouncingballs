#include "stdafx.h"
#include "gep/unittest/UnittestManager.h"
#include <sstream>
#include <stdarg.h>
#include <Windows.h>
#include "gep/timer.h"

gep::UnittestManager* gep::UnittestManager::s_globalInstance = nullptr;

gep::UnittestManager::UnittestManager()
{
    m_doDebugBreaks = false;
    setFailedAssertHandler(this);
}

gep::UnittestManager::~UnittestManager()
{
    setFailedAssertHandler(nullptr);
}

gep::UnittestManager& gep::UnittestManager::instance()
{
    if(s_globalInstance == nullptr)
    {
        s_globalInstance = new UnittestManager();
    }
    return *s_globalInstance;
}

void gep::UnittestManager::destoryGlobalInstance()
{
    if(s_globalInstance != nullptr)
    {
        delete s_globalInstance;
        s_globalInstance = nullptr;
    }
}

void gep::UnittestManager::registerGroup(gep::UnittestGroup* group)
{
    m_groups.push_back(group);
}

int gep::UnittestManager::runAllTests()
{
    int numFailedTests = 0;
    UnittestLog log;
    Timer timer;
    PointInTime testsBegin(timer);
    for(auto it = m_groups.begin(); it < m_groups.end(); ++it)
    {
        numFailedTests += (*it)->runAllSubtests(log);
    }
    PointInTime testsEnd(timer);
    if(numFailedTests > 0)
    {
		log.logFailure("\n%d tests failed\n", numFailedTests);
    }
    else
    {
		log.logSuccess("All tests passed in %f s\n", testsEnd - testsBegin);
    }
    return numFailedTests;
}

gep::AssertCallbackResult gep::UnittestManager::failedAssert(const char* sourceFile, unsigned int line, const char* function, const char* expression, const char* msg, const char* additional)
{
    std::ostringstream message;
    message << msg << std::endl << "Expression: " << expression << std::endl << "Function: " << function << std::endl;
    message << additional;
    if(m_doDebugBreaks)
        GEP_DEBUG_BREAK
    throw UnittestFailedException(sourceFile, line, std::move(message.str()));
}

gep::UnittestFailedException::UnittestFailedException(const char* file, unsigned int line, std::string&& message) : std::exception(),
  m_message(message)
{
  m_file = file;
  m_line = line;
}

const char* gep::UnittestFailedException::what() const
{
  return m_message.c_str();
}

gep::UnittestGroup::UnittestGroup(const char* name) :
    m_name(name)
{
    UnittestManager::instance().registerGroup(this);
}

int gep::UnittestGroup::runAllSubtests(UnittestLog& log)
{
    int numTestsFailed = 0;
    log.logMessage("Starting Test Group %s\n", m_name);
    Timer timer;
    PointInTime testsBegin(timer);
    for(auto it = m_tests.begin(); it != m_tests.end(); ++it)
    {
        try
        {
            if((*it)->Initialize(log) != SUCCESS)
            {
                log.logFailure("  Subtest %s failed to initialize\n", (*it)->getName());
                numTestsFailed++;
            }
            else
            {
                PointInTime beforeTest(timer);
                (*it)->Run(log);
                PointInTime afterTest(timer);
                if((*it)->Deinitialize(log) != SUCCESS)
                {
                    log.logFailure("  Subtest %s failed to deinitialize\n", (*it)->getName());
                    numTestsFailed++;
                }
                else
                {
                    log.logSuccess("  Subtest %s passed in %f s\n", (*it)->getName(), afterTest - beforeTest);
                }
            }
        }
        catch(UnittestFailedException& ex)
        {
            log.logFailure("  Subtest %s failed in file '%s' line %d\n%s\n", (*it)->getName(), ex.getFile(), ex.getLine(), ex.what());
            numTestsFailed++;
        }
        catch(std::exception& ex)
        {
            log.logFailure("  Subtest %s failed due to an exception '%s'\n", (*it)->getName(), ex.what());
            numTestsFailed++;
        }
        catch(...)
        {
            log.logFailure("  Subtest %s failed due to an unkown exception\n", (*it)->getName());
            numTestsFailed++;
        }
    }
    PointInTime testsEnd(timer);
    if(numTestsFailed > 0)
    {
        log.logFailure("%d Tests failed in group %s\n\n", numTestsFailed, m_name);
    }
    else
    {
        log.logSuccess("All Tests in group %s passed in %f s\n\n", m_name, testsEnd - testsBegin);
    }
    return numTestsFailed;
}

void gep::UnittestGroup::registerTest(IUnittest* test)
{
    m_tests.push_back(test);
}

void gep::UnittestLog::logFailure(const char* fmt, ...)
{
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), 0x0C);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), 0x07);
}

void gep::UnittestLog::logSuccess(const char* fmt, ...)
{
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), 0x0A);
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
    SetConsoleTextAttribute (GetStdHandle (STD_OUTPUT_HANDLE), 0x07);
}

void gep::UnittestLog::logMessage(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);
}

gep::SimpleUnittest::SimpleUnittest(const char* name, UnittestGroup& group) :
    m_name(name)
{
    group.registerTest(this);
}

gep::Result gep::SimpleUnittest::Initialize(UnittestLog& log)
{
    return SUCCESS;
}

gep::Result gep::SimpleUnittest::Deinitialize(UnittestLog& log)
{
    return SUCCESS;
}
