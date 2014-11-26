#pragma once

#include "gep/gepmodule.h"
#include "gep/ArrayPtr.h"

#include <Windows.h>
#include <string>

namespace gep
{
    class GEP_API StackWalker
    {
    public:
      typedef size_t address_t; ///< type for a instruction address

      /// \brief
      ///  Get a given number of addresses from the callstack
      ///
      /// \param functionAddresses
      ///   where to save the results to (The array needs to be preallocated)
      ///
      /// \param pContext
      ///   the context to be used for capturing the stack frames. If null the current context is used
      ///
      /// \return the number of addresses written
      static size_t getCallstack(size_t framesToSkip, ArrayPtr<address_t> functionAddresses, CONTEXT* pContext = nullptr);

      /// \brief
      ///  resolves a number of given addresses to function names + file & line number
      ///
      /// \param functionAddresses
      ///  array of function addresses previously collected with GetCallstack
      ///
      /// \param pFunctionNames
      ///   array of function names which is uiMaxFuncNameLength * uiNumAddresses * sizeof(char) in length
      ///
      /// \param uiMaxFuncNameLength
      ///   maximum length of a function name string
      static void resolveCallstack(ArrayPtr<address_t> functionAddresses, char* pFunctionNames, size_t uiMaxFuncNameLength);

    private:
      static bool s_isInitialized;

      static std::string generateSearchPath();
    };

}
