#pragma once

#include "gep/interfaces/events/eventManager.h"
#include "gep/interfaces/events/eventId.h"
#include "gep/interfaces/subsystem.h"

#include "gep/container/hashmap.h"

namespace gep
{
    class GlobalEventManager : public IEventManager
    {
        Event<float> m_update;
        Event<nullptr_t> m_postInit;
        Hashmap<EventId, Event<ScriptTableWrapper>*> m_scriptEvents;
    public:
        GlobalEventManager() :
            m_update(),
            m_postInit(),
            m_scriptEvents()
        {
        }

        virtual void initialize() override
        {
        }

        virtual void destroy() override
        {
            for (auto evt : m_scriptEvents.values())
            {
                delete evt;
            }
            m_scriptEvents.clear();
        }

        virtual Event<float>* getUpdateEvent() override
        {
            return &m_update;
        }

        virtual Event<nullptr_t>* getPostInitializationEvent() override
        {
            return &m_postInit;
        }

        virtual void update(float elapsedTime) override
        {
            m_update.trigger(elapsedTime);
        }

        virtual Event<ScriptTableWrapper>* createScriptTableEvent() override
        {
            auto evt = new Event<ScriptTableWrapper>();
            m_scriptEvents[evt->getId()] = evt;
            return evt;
        }

    };
}
