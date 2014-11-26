#pragma once
#include <exception>
#include <string>

namespace gep
{
    class Exception : public std::exception
    {
    private:
        std::string m_msg;

    public:
        Exception(const std::string& msg) : m_msg(msg) {}

        virtual const char* what() const override { return m_msg.c_str(); }
    };

    class LoadingError : public Exception
    {
    public:
        LoadingError(const std::string& msg) : Exception(msg) {}
    };

    class ScriptException : public Exception
    {
    public:
        ScriptException(const std::string& msg) : Exception(msg) {}
    };

    class ScriptLoadException : public ScriptException
    {
    public:
        ScriptLoadException(const std::string& msg) : ScriptException(msg) {}
    };

    class ScriptExecutionException : public ScriptException
    {
    public:
        ScriptExecutionException(const std::string& msg) : ScriptException(msg) {}
    };

    class EventException : public Exception
    {
    public:
        EventException(const std::string& msg) : Exception(msg) {}
    };
    
};
