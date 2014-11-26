#pragma once
#include <vector>
#include <exception>

namespace gep
{
    /// \brief class for unittest logging
    class GEP_API UnittestLog
    {
    public:
        void logFailure(const char* fmt, ...);
        void logSuccess(const char* fmt, ...);
        void logMessage(const char* fmt, ...);
    };

    /// \brief interface for a single unittest
    class GEP_API IUnittest
    {
    public:
        virtual Result Initialize(UnittestLog& log) = 0;
        virtual void Run(UnittestLog& log) = 0;
        virtual Result Deinitialize(UnittestLog& log) = 0;
        virtual const char* getName() const = 0;
    };

    /// \brief a named group of unittests
    class GEP_API UnittestGroup
    {
    private:
        const char* m_name;
        std::vector<IUnittest*> m_tests;

    public:
        UnittestGroup(const char* name);

        /// \brief runs all tests in this testgroup
        /// \return the number of tests that failed
        int runAllSubtests(UnittestLog& log);

        /// \brief registers a new subtest
        void registerTest(IUnittest* test);
    };

    /// \brief singelton that manages all unittest
    class GEP_API UnittestManager : public IFailedAssertCallback
    {
    private:
        std::vector<UnittestGroup*> m_groups;
        static UnittestManager* s_globalInstance;
        bool m_doDebugBreaks;

        UnittestManager();
        ~UnittestManager();

    public:
        static UnittestManager& instance();

        void destoryGlobalInstance();
        void registerGroup(UnittestGroup* group);

        void setDoDebugBreaks(bool value) { m_doDebugBreaks = value; }

        virtual AssertCallbackResult failedAssert(const char* sourceFile, unsigned int line, const char* function, const char* expression, const char* msg, const char* additional) override;

        /// \brief runs all registered tests
        /// \return the number of tests that failed
        int runAllTests();
    };

    /// \brief exception that is thrown whenever a testing condition fails
    class GEP_API UnittestFailedException : public std::exception
    {
    private:
      const char* m_file;
      unsigned int m_line;
      std::string m_message;

    public:
      UnittestFailedException(const char* file, unsigned int line, std::string&& message);
      virtual const char* what() const override;
      inline const char* getFile() const { return m_file; }
      inline unsigned int getLine() const { return m_line; }
    };

    /// \brief a simple unittest implementation
    class GEP_API SimpleUnittest : public IUnittest
    {
    private:
        const char* m_name;
    public:
        SimpleUnittest(const char* name, UnittestGroup& group);
        virtual Result Initialize(UnittestLog& log) override;
        virtual Result Deinitialize(UnittestLog& log) override;
        virtual const char* getName() const override { return m_name; }
    };
};

#define GEP_EXTERN_UNITTEST_GROUP(groupId) extern gep::UnittestGroup g_unittest_##groupId;
#define GEP_UNITTEST_GROUP(groupId) inline gep::UnittestGroup& unittestGroup_##groupId() { \
    static gep::UnittestGroup instance(GEP_STRINGIZE(groupId));                            \
    return instance;                                                                       \
};
#define GEP_UNITTEST_TEST(groupId, testId) \
class Unittest_##groupId##testId : public gep::SimpleUnittest { \
public: \
    Unittest_##groupId##testId(const char* name, gep::UnittestGroup& group) : gep::SimpleUnittest(name,group) {} \
    virtual void Run(gep::UnittestLog& log) override; \
}; \
Unittest_##groupId##testId g_unittest_##groupId##testId(GEP_STRINGIZE(testId),unittestGroup_##groupId()); \
void Unittest_##groupId##testId::Run(gep::UnittestLog& log)
