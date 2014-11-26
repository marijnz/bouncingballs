#include "stdafx.h"
#include "gep/StackWalker.h"

#include <DbgHelp.h>
#include <Tlhelp32.h>

bool gep::StackWalker::s_isInitialized = false;


size_t gep::StackWalker::getCallstack(size_t framesToSkip, ArrayPtr<address_t> functionAddresses, CONTEXT* pContext)
{
    if(functionAddresses.length() < 63 && pContext == nullptr)
    {
        return RtlCaptureStackBackTrace((DWORD)framesToSkip, (DWORD)functionAddresses.length(), (void**)functionAddresses.getPtr(), nullptr);
    }

    HANDLE hThread = GetCurrentThread();
    HANDLE hProcess = GetCurrentProcess();
    STACKFRAME64 stackframe;
    DWORD imageType;
    CONTEXT c;

    if(pContext == nullptr)
    {
        c.ContextFlags = CONTEXT_FULL;
        RtlCaptureContext(&c);
    }
    else
        c = *pContext;

#ifdef _M_X64 //x64
    imageType = IMAGE_FILE_MACHINE_AMD64;
    stackframe.AddrPC.Offset = c.Rip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = c.Rbp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = c.Rsp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#else //x86
    imageType = IMAGE_FILE_MACHINE_I386;
    stackframe.AddrPC.Offset = (DWORD64)c.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
    stackframe.AddrFrame.Offset = (DWORD64)c.Ebp;
    stackframe.AddrFrame.Mode = AddrModeFlat;
    stackframe.AddrStack.Offset = (DWORD64)c.Esp;
    stackframe.AddrStack.Mode = AddrModeFlat;
#endif
    stackframe.AddrReturn.Offset = 0;

    size_t frameNum = 0;

    // do ... while so that we don't skip the first stackframe
    do
    {
        if( stackframe.AddrPC.Offset == stackframe.AddrReturn.Offset )
        {
            break; //endless callstack
        }
        if(frameNum >= framesToSkip)
        {
            functionAddresses[frameNum - framesToSkip] = (address_t)stackframe.AddrPC.Offset;
        }
        frameNum++;
    }
    while (StackWalk64(imageType, hProcess, hThread, &stackframe, &c, nullptr, nullptr, nullptr, nullptr));

    if(frameNum < framesToSkip)
        return 0;
    return frameNum - framesToSkip;
}

void gep::StackWalker::resolveCallstack(ArrayPtr<address_t> functionAddresses, char* pFunctionNames, size_t uiMaxFuncNameLength)
{
  HANDLE hProcess = GetCurrentProcess();
  if(!s_isInitialized)
  {
    DWORD symOptions = SymGetOptions();
    symOptions |= SYMOPT_LOAD_LINES;
    symOptions |= SYMOPT_FAIL_CRITICAL_ERRORS;
    symOptions |= SYMOPT_DEFERRED_LOADS;
    symOptions = SymSetOptions( symOptions );

    std::string searchPath = generateSearchPath();
    SymInitialize(hProcess, searchPath.c_str(), TRUE);

    s_isInitialized = true;
  }

  size_t symbolSize = sizeof(IMAGEHLP_SYMBOL64) + uiMaxFuncNameLength;
  IMAGEHLP_SYMBOL64* symbol = (IMAGEHLP_SYMBOL64*)calloc( symbolSize, 1 );

  symbol->SizeOfStruct = (DWORD)symbolSize;
  symbol->MaxNameLength = (DWORD)uiMaxFuncNameLength;

  IMAGEHLP_LINE64 line;
  line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

  IMAGEHLP_MODULE64 moduleInfo;
  moduleInfo.SizeOfStruct = sizeof(IMAGEHLP_MODULE64);

  for(size_t i=0; i<functionAddresses.length(); i++)
  {
    char* funcName = pFunctionNames + (uiMaxFuncNameLength * i);
    *funcName = '\0';

    DWORD64 offset = 0;
    DWORD displacement = 0;

    if(functionAddresses[i] != 0)
    {
      if( SymGetSymFromAddr64(hProcess, functionAddresses[i], &offset, symbol ) == TRUE )
      {
        char undecoratedName[512];
        char* symbolName = symbol->Name;
        if( SymUnDName64(symbol, undecoratedName, GEP_ARRAY_SIZE(undecoratedName)) == TRUE )
        {
          symbolName = undecoratedName;
        }

        if( SymGetLineFromAddr64( hProcess, functionAddresses[i], &displacement, &line ) == TRUE )
        {
          sprintf_s(funcName, uiMaxFuncNameLength, "%s(%d): %s", line.FileName, line.LineNumber, symbolName );
        }
        else
        {
          sprintf_s(funcName, uiMaxFuncNameLength, "%s", symbolName);
        }
      }
      else {
        sprintf_s(funcName, uiMaxFuncNameLength, "unknown %x", functionAddresses[i]);
      }
      funcName[uiMaxFuncNameLength-1] = '\0';
    }
    else {
      strcpy_s(funcName, uiMaxFuncNameLength, "unknown");
    }
  }

  free(symbol);
}

std::string gep::StackWalker::generateSearchPath()
{
  const char* defaultPathList[] = {"_NT_SYMBOL_PATH", "_NT_ALTERNATE_SYMBOL_PATH", "SYSTEMROOT"};

  std::string path;
  char temp[512] = {'\0'};
  DWORD len;

  for(int i=0; i<GEP_ARRAY_SIZE(defaultPathList); i++)
  {
    if( (len = GetEnvironmentVariableA( defaultPathList[i], temp, GEP_ARRAY_SIZE(temp) )) > 0 )
    {
      path += temp;
    }
  }
  return path;
}
