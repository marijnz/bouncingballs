#include "stdafx.h"
#include "gep/exit.h"

namespace gep
{
    struct ExitNode
    {
        exitFunc_t func;
        ExitNode* next;
    };
}

gep::ExitNode* g_firstExitNode;


gep::Result gep::atexit(gep::exitFunc_t func)
{
    if(func == nullptr)
        return FAILURE;
    auto newNode = (ExitNode*)malloc(sizeof(ExitNode));
    if(newNode == nullptr)
        return FAILURE;
    newNode->func = func;
    newNode->next = g_firstExitNode;
    g_firstExitNode = newNode;
    return SUCCESS;
}

void gep::destroy()
{
    while(g_firstExitNode != nullptr)
    {
        ExitNode* curNode = g_firstExitNode;
        g_firstExitNode = curNode->next;
        curNode->func();
        free(curNode);
    }
    g_firstExitNode = nullptr;
}
