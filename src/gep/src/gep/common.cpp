#include "stdafx.h"

namespace gep
{
  IFailedAssertCallback* g_failedAssertHandler = nullptr;
};

bool gep::failedAssert(const char* sourceFile, unsigned int line, const char* function, const char* expression, const char* msg, const char* additional)
{
  if(g_failedAssertHandler != nullptr)
  {
    auto handlerResult = g_failedAssertHandler->failedAssert(sourceFile, line, function, expression, msg, additional);
    if(handlerResult == AssertCallbackResult::triggerDebugBreak)
      return true; //Debug break
    else if(handlerResult == AssertCallbackResult::ignore)
      return false;
    //AssertCallbackResult::continueWithNextHandler
  }

#ifdef _DEBUG
  int userResponse = _CrtDbgReport(_CRT_ASSERT, sourceFile, line, NULL, "%s\nExpression: %s\nFunction: %s\n%s", msg, expression, function, additional);
  if(userResponse != 0)
      return true; //Debug break
#endif

  return false; //Don't debug break
}

void gep::setFailedAssertHandler(gep::IFailedAssertCallback* handler)
{
  g_failedAssertHandler = handler;
}

gep::IFailedAssertCallback* gep::getFailedAssertHandler()
{
  return g_failedAssertHandler;
}
