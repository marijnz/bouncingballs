#pragma once
#include <sstream>
#include <functional>
/**
 * This headerfiles holds very common macros and helper functions like assert
 */

//Causes a debug break
#define GEP_DEBUG_BREAK { __debugbreak(); }

// printf format string annotation
#define GEP_PRINTF_FORMAT_STRING _Printf_format_string_

/// Concatenates two strings, even when the strings are macros themselves
#define GEP_CONCAT(x,y) GEP_CONCAT_HELPER(x,y)
#define GEP_CONCAT_HELPER(x,y) GEP_CONCAT_HELPER2(x,y)
#define GEP_CONCAT_HELPER2(x,y) x##y

/// Stringizes a string, even macros
#define GEP_STRINGIZE(str) GEP_STRINGIZE_HELPER(str)
#define GEP_STRINGIZE_HELPER(x) #x

// GEP_VA_NUM_ARGS() is a very nifty macro to retrieve the number of arguments handed to a variable-argument macro
// unfortunately, VS 2010 still has this compiler bug which treats a __VA_ARGS__ argument as being one single parameter:
// https://connect.microsoft.com/VisualStudio/feedback/details/521844/variadic-macro-treating-va-args-as-a-single-parameter-for-other-macros#details
#if _MSC_VER >= 1400
  #define GEP_VA_NUM_ARGS_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...)    N
  #define GEP_VA_NUM_ARGS_REVERSE_SEQUENCE            10, 9, 8, 7, 6, 5, 4, 3, 2, 1
  #define GEP_LEFT_PARENTHESIS (
  #define GEP_RIGHT_PARENTHESIS )
  #define GEP_VA_NUM_ARGS(...)                        GEP_VA_NUM_ARGS_HELPER GEP_LEFT_PARENTHESIS __VA_ARGS__, GEP_VA_NUM_ARGS_REVERSE_SEQUENCE GEP_RIGHT_PARENTHESIS
#endif

// GEP_PASS_VA passes __VA_ARGS__ as multiple parameters to another macro, working around the above-mentioned bug
#if _MSC_VER >= 1400
  #define GEP_PASS_VA(...)                            GEP_LEFT_PARENTHESIS __VA_ARGS__ GEP_RIGHT_PARENTHESIS
#endif

// Passes variadic preprocessor arguments to another macro
#ifndef GEP_PASS_VA
  #define GEP_PASS_VA(...)                            (__VA_ARGS__)
#endif

#define GEP_EXPAND_ARGS_1(op, a1)                 op(a1)
#define GEP_EXPAND_ARGS_2(op, a1, a2)             op(a1) op(a2)
#define GEP_EXPAND_ARGS_3(op, a1, a2, a3)         op(a1) op(a2) op(a3)
#define GEP_EXPAND_ARGS_4(op, a1, a2, a3, a4)     op(a1) op(a2) op(a3) op(a4)
#define GEP_EXPAND_ARGS_5(op, a1, a2, a3, a4, a5) op(a1) op(a2) op(a3) op(a4) op(a5)

#define GEP_EXPAND_ARGS_WITH_INDEX_1(op, a1)               op(a1, 1)
#define GEP_EXPAND_ARGS_WITH_INDEX_2(op, a1, a2)           op(a1, 1) op(a2, 2)
#define GEP_EXPAND_ARGS_WITH_INDEX_3(op, a1, a2, a3)       op(a1, 1) op(a2, 2) op(a3, 3)
#define GEP_EXPAND_ARGS_WITH_INDEX_4(op, a1, a2, a3, a4)   op(a1, 1) op(a2, 2) op(a3, 3) op(a4, 4)


// variadic macro "dispatching" the arguments to the correct macro.
// the number of arguments is found by using ME_PP_NUM_ARGS(__VA_ARGS__)
#define GEP_EXPAND_ARGS(op, ...)        GEP_CONCAT(GEP_EXPAND_ARGS_, GEP_VA_NUM_ARGS(__VA_ARGS__)) GEP_PASS_VA(op, __VA_ARGS__)
#define GEP_EXPAND_ARGS_WITH_INDEX(op, ...)        GEP_CONCAT(GEP_EXPAND_ARGS_WITH_INDEX_, GEP_VA_NUM_ARGS(__VA_ARGS__)) GEP_PASS_VA(op, __VA_ARGS__)

// The first argument to the GEP_ASSERT is the condition
// Only the first argument is needed, all others are optional
// The second argument is a string message to be displayed
// All remaining arguments are variables that give context for the assert
//
// Examples:
// GEP_ASSERT(a < b)
// GEP_ASSERT(a < b, "a is not less then b")
// GEP_ASSERT(a < b, "a is not less then b", a, b)

namespace gep
{
  GEP_API bool failedAssert(const char* sourceFile, unsigned int line, const char* function, const char* expression, const char* msg, const char* additional);

  enum class AssertCallbackResult
  {
    triggerDebugBreak,
    continueWithNextHandler,
    ignore
  };

  class GEP_API IFailedAssertCallback
  {
  public:
    virtual AssertCallbackResult failedAssert(const char* sourceFile, unsigned int line, const char* function, const char* expression, const char* msg, const char* additional) = 0;
  };
  GEP_API void setFailedAssertHandler(IFailedAssertCallback* handler);
  GEP_API IFailedAssertCallback* getFailedAssertHandler();
}

#define GEP_ASSERT_1(cond) \
  if(!(cond)) { if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, "", "")) GEP_DEBUG_BREAK }
#define GEP_ASSERT_2(cond, msg) \
  if(!(cond)) { if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, "")) GEP_DEBUG_BREAK }
#define GEP_ASSERT_3(cond, msg, var1) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }
#define GEP_ASSERT_4(cond, msg, var1, var2) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    info << std::endl << #var2 << " = " << var2; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }
#define GEP_ASSERT_5(cond, msg, var1, var2, var3) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    info << std::endl << #var2 << " = " << var2; \
    info << std::endl << #var3 << " = " << var3; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }
#define GEP_ASSERT_6(cond, msg, var1, var2, var3, var4) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    info << std::endl << #var2 << " = " << var2; \
    info << std::endl << #var3 << " = " << var3; \
    info << std::endl << #var4 << " = " << var4; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }
#define GEP_ASSERT_7(cond, msg, var1, var2, var3, var4, var5) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    info << std::endl << #var2 << " = " << var2; \
    info << std::endl << #var3 << " = " << var3; \
    info << std::endl << #var4 << " = " << var4; \
    info << std::endl << #var5 << " = " << var5; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }
#define GEP_ASSERT_8(cond, msg, var1, var2, var3, var4, var5, var6) \
  if(!(cond)) { \
  std::ostringstream info; \
    info << std::endl << #var1 << " = " << var1; \
    info << std::endl << #var2 << " = " << var2; \
    info << std::endl << #var3 << " = " << var3; \
    info << std::endl << #var4 << " = " << var4; \
    info << std::endl << #var5 << " = " << var5; \
    info << std::endl << #var6 << " = " << var6; \
    if(gep::failedAssert(__FILE__, __LINE__, __FUNCTION__, #cond, msg, info.str().c_str())) GEP_DEBUG_BREAK \
  }

#if defined(_DEBUG) || defined(GEP_UNITTESTS)
  #define GEP_ASSERT_ACTIVE
  #define GEP_ASSERT(...) GEP_CONCAT(GEP_ASSERT_, GEP_VA_NUM_ARGS(__VA_ARGS__)) GEP_PASS_VA(__VA_ARGS__)
#else
  #define GEP_ASSERT(...)
#endif

/// compile-time size of a static array
#define GEP_ARRAY_SIZE(arr) sizeof(ArraySizeHelper(arr))

namespace
{
    template<typename T, size_t N>
    char (&ArraySizeHelper(const T (&)[N]))[N];
}

//min & max macros
#define GEP_MIN(x, y) (((x) < (y)) ? (x) : (y))
#define GEP_MAX(x, y) (((x) > (y)) ? (x) : (y))

//SCOPE_EXIT macro
template <typename F>
struct ScopeExit {
    ScopeExit(F f) : f(f) {}
    ~ScopeExit() { f(); }
    F f;
};

template <typename F>
ScopeExit<F> MakeScopeExit(F f) {
    return ScopeExit<F>(f);
};

// Use it like this: SCOPE_EXIT{ freeSomeResource(1, 2, 3); });
#define SCOPE_EXIT auto GEP_CONCAT(scope_exit_, __LINE__) = MakeScopeExit([&]()
#define GEP_RELEASE_AND_NULL(x) if(x != nullptr) { x->Release(); x = nullptr; }

// Disallow copy constructor and assignment operator
#define GEP_DISALLOW_COPY_AND_ASSIGNMENT(type) private: type(const type&); void operator=(const type&)

// Disallow construction of a class or a struct, i.e. make the constructor private
#define GEP_DISALLOW_CONSTRUCTION(type) private: type(); ~type(); GEP_DISALLOW_COPY_AND_ASSIGNMENT(type)

// Signify the user (and the compiler) that a certain variable is not in use
#define GEP_UNUSED(var) (void)(var)

// Enable or disable variadic template arguments
#define GEP_VARIADIC_TEMPLATE_ARGUMENTS_ENABLED 0

#include "gep/types.h"
// disable the cross dll interface warnings
#pragma warning( disable : 4251 )

#include <lua.hpp>
