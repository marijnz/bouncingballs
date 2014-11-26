#include "stdafx.h"
#include "gepimpl/subsystems/renderer/extractor.h"
#include "gep/globalManager.h"


void* gep::RendererExtractor::doMakeCommand(size_t size, CommandType type)
{
    GEP_ASSERT(m_isExtracting == true, "calling extractor from outside of a extraction callback");
    void* mem = m_pCurrentAllocator->allocateMemory(size);
    memset(mem, 0, size);
    auto cmd = (CommandBase*)mem;
    cmd->type = type;
    GEP_ASSERT((char*)mem > (char*)m_pLastCommand);
    auto offset = (char*)mem - (char*)m_pLastCommand;
    GEP_ASSERT(offset < std::numeric_limits<uint16>::max(), "offset overflow");
    m_pLastCommand->offsetNext = (uint16)offset;
    m_pLastCommand = cmd;
    return mem;
}

gep::CallbackId gep::RendererExtractor::registerExtractionCallback(std::function<void(IRendererExtractor& extractor)> callback)
{
    for(size_t i=0; i <m_callbacks.length(); ++i)
    {
        if(!m_callbacks[i])
        {
            m_callbacks[i] = callback;
            return CallbackId(i);
        }
    }
    m_callbacks.append(callback);
    return CallbackId(m_callbacks.length() - 1);
}

void gep::RendererExtractor::deregisterExtractionCallback(CallbackId callbackId)
{
    GEP_ASSERT(callbackId.id < m_callbacks.length(), "callback id out of bounds");
    GEP_ASSERT(m_callbacks[callbackId.id], "callback was already deregistered");
    m_callbacks[callbackId.id] = nullptr;
}


gep::RendererExtractor::RendererExtractor()
    : m_isExtracting(false),
    m_pCurrentAllocator(nullptr),
    m_pLastCommand(nullptr),
    m_nextPoolToFill(0),
    m_nextPoolToRead(0),
    m_fullPoolSync(0),
    m_emptyPoolSync(NUM_POOLS),
    m_context2d(*this)
{
    m_pCurrentAllocator = m_pools[0].pAllocator;
}

gep::RendererExtractor::~RendererExtractor()
{
}

void gep::RendererExtractor::extract()
{
    m_emptyPoolSync.waitAndDecrement();
    m_isExtracting = true;

    auto& pool = m_pools[m_nextPoolToFill];
    m_nextPoolToFill = (m_nextPoolToFill + 1) % NUM_POOLS;
    m_pCurrentAllocator = pool.pAllocator;
    m_pLastCommand = (CommandBase*)m_pCurrentAllocator->allocateMemory(sizeof(CommandBase));
    m_pLastCommand->offsetNext = 0;
    m_pLastCommand->type = CommandType::FirstCommand;

    for(auto& callback : m_callbacks)
    {
        if (callback)
            callback(*this);
    }

    m_isExtracting = false;
    m_fullPoolSync.increment();
}

void gep::RendererExtractor::setCamera(ICamera* pCamera)
{
    auto& cmd = makeCommand<CommandCamera>();
    cmd.viewMatrix = pCamera->getViewMatrix();
    cmd.projectionMatrix = pCamera->getProjectionMatrix();
}

gep::CommandBase* gep::RendererExtractor::startReadCommands()
{
    m_fullPoolSync.waitAndDecrement();
    auto& pool = m_pools[m_nextPoolToRead];
    m_nextPoolToRead = (m_nextPoolToRead + 1) % NUM_POOLS;
    CommandBase* firstCommand = (CommandBase*)pool.pStart;
    GEP_ASSERT(firstCommand->type == CommandType::FirstCommand);
    return nextCommand(firstCommand);
}

void gep::RendererExtractor::endReadCommands()
{
    auto& pool = m_pools[(m_nextPoolToRead + NUM_POOLS - 1) % NUM_POOLS];
    pool.pAllocator->freeToMarker(pool.pStart);
    m_emptyPoolSync.increment();
}

void gep::RendererExtractor::beginDebugMarker(const char* name)
{
    auto& cmd = makeCommand<CommandDebugMarkerBegin>();
    const size_t len = strlen(name)+1;
    auto wc = (wchar_t*)getCurrentAllocator()->allocateMemory(sizeof(WCHAR) * len);
    mbstowcs (wc, name, len);
    cmd.name = wc;
}

void gep::RendererExtractor::endDebugMarker()
{
    auto& cmd = makeCommand<CommandDebugMarkerEnd>();
}

gep::CommandBase* gep::RendererExtractor::nextCommand(CommandBase* lastCommand)
{
    if(lastCommand->offsetNext == 0)
        return nullptr;
    return (CommandBase*)((char*)lastCommand + lastCommand->offsetNext);
}

gep::IContext2D& gep::RendererExtractor::getContext2D()
{
    return m_context2d;
}

gep::Context2D::Context2D(RendererExtractor& extractor) :
    m_extractor(extractor)
{
}

void gep::Context2D::printText(const vec2& screenPosition, const char* text, Color color)
{
    auto& cmd = m_extractor.makeCommand<CommandDrawText>();
    cmd.position = g_globalManager.getRenderer()->toAbsoluteScreenPosition(screenPosition);
    cmd.color = color;
    auto len = strlen(text);

#ifdef _DEBUG
    for (size_t i = 0; i < len; i++)
    {
        GEP_ASSERT(text[i] > 0, "ascii string contains non-ascii characters. Please use wide char version to print non ascii characters");
    }
#endif // _DEBUG

    cmd.text = GEP_NEW_ARRAY(m_extractor.getCurrentAllocator(), char, len + 1).getPtr();
    memcpy((void*)cmd.text, text, len + 1);
}
