#pragma once

#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/ReferenceCounting.h"
#include "gep/exception.h"

#include "gep/interfaces/events/eventId.h"
#include "gep/interfaces/events/eventUpdateFramework.h"

#include "gep/interfaces/logging.h"
#include "gep/interfaces/scripting.h"
#include "gep/math3d/algorithm.h"

namespace gep
{
    struct EventResult
    {
        enum Enum
        {
            Handled,
            Ignored,
            Cancel
        };

        GEP_DISALLOW_CONSTRUCTION(EventResult);
    };

    class GEP_API EventBase
    {
    public:
        ILogging* getLogging() const;
    };

    template< typename T_EventData>
    class Event : public EventBase
    {
    public:
        typedef std::function<EventResult::Enum(T_EventData)> EventType;
        typedef Event<T_EventData> OwnType;
        typedef EventListenerId<OwnType> ListenerIdType;
        typedef DelayedEventId<OwnType> DelayedEventIdType;
        typedef std::function<void(OwnType&)> InitializerType;
        typedef std::function<void(OwnType&)> DestroyerType;

        struct CInfo
        {
            InitializerType initializer;
            DestroyerType destroyer;
            IUpdateFramework* updateFramework;
            IScriptingManager* scriptingManager;
            IAllocator* allocator;

            CInfo() :
                initializer(nullptr),
                destroyer(nullptr),
                updateFramework(nullptr),
                scriptingManager(nullptr),
                allocator(&g_stdAllocator)
            {
            }

            explicit CInfo(IAllocator* pAllocator) :
                initializer(nullptr),
                destroyer(nullptr),
                updateFramework(nullptr),
                scriptingManager(nullptr),
                allocator(pAllocator)
            {
            }
        };

    public:

        explicit Event(const CInfo& cinfo = CInfo()) :
            m_pAllocator(cinfo.allocator),
            m_id(EventId::generate()),
            m_triggerLevel(0),
            m_listeners(m_pAllocator),
            m_delayedEvents(m_pAllocator),
            m_callbackId_update(-1),
            m_onDestroy(cinfo.destroyer),
            m_pUpdateFramework(cinfo.updateFramework),
            m_pScriptingManager(cinfo.scriptingManager)
        {
            if (m_pUpdateFramework == nullptr) { m_pUpdateFramework = &EventUpdateFramework::instance(); }
            if (m_pScriptingManager == nullptr) { m_pScriptingManager = &EventScriptingManager::instance(); }

            if(cinfo.initializer) { cinfo.initializer(*this); }

            startUpdating();
        }

        ~Event()
        {
            GEP_ASSERT(m_triggerLevel == 0,
                "This event is being destroyed even though it is still triggering!",
                m_triggerLevel);

            if(m_onDestroy) { m_onDestroy(*this); }

            m_onDestroy = nullptr;
            stopUpdating();
            m_pUpdateFramework = nullptr;
            m_pScriptingManager = nullptr;
            m_delayedEvents.clear();
            m_listeners.clear();
        }

        inline ListenerIdType registerListener(const EventType& listener)
        {
            return registerListener(0, listener);
        }

        inline ListenerIdType registerListener(int16 priority, const EventType& listener)
        {
            ListenerWrapper wrapper;
            wrapper.id = ListenerIdType::generate();
            wrapper.priority = priority;
            wrapper.listener = listener;
            insertListener(wrapper);
            return wrapper.id;
        }

        inline ListenerIdType registerListener(ScriptFunctionWrapper funcRef)
        {
            return registerListener(0, funcRef);
        }

        inline ListenerIdType registerListener(int16 priority, ScriptFunctionWrapper funcRef)
        {
            // Wrap the call to the script function in an actual listener.
            auto actualListener = [&, funcRef](T_EventData data) {
                auto eventResult = EventResult::Handled;
                auto callResult = ScriptCallerType::call(m_pScriptingManager,
                                                         funcRef,
                                                         data,
                                                         eventResult);

                if(callResult.failedReturn())
                {
                    getLogging()->logError("An error occurred when trying to get the return value from the event listener. "
                                           "Will use the default return value EventResult.Handled. "
                                           "Did you forget to return an EventResult?");
                    eventResult = EventResult::Handled;
                }

                return eventResult;
            };
            return registerListener(priority, actualListener);
        }

        inline Result deregisterListener(ListenerIdType id)
        {
            if (m_triggerLevel > 0)
            {
                throw EventException("Cannot deregister listener while triggering!");
            }
            return removeListener(id);
        }

        inline EventResult::Enum trigger(T_EventData data)
        {
            TriggerCounter counter(m_triggerLevel);
            EventResult::Enum callResult = EventResult::Ignored;

            for (auto listener : m_listeners)
            {
                if(!listener) { continue; }

                callResult = call(listener, data);
                if (callResult == EventResult::Cancel)
                {
                    break;
                }
            }
            return callResult;
        }

        inline DelayedEventIdType delayedTrigger(float delayInSeconds, T_EventData data)
        {
            if (delayInSeconds <= 0.0f)
            {
                trigger(data);
                return DelayedEventIdType::invalidValue();
            }

            DelayedEvent delayedEvent;
            delayedEvent.remainingSeconds = delayInSeconds;
            delayedEvent.data = data;
            DelayedEventIdType delayedEventId(DelayedEventIdType::generate());
            m_delayedEvents[delayedEventId] = delayedEvent;
            return delayedEventId;
        }

        inline Result modifyDelayedEventTime(DelayedEventIdType id, float newTime)
        {
            Result result = FAILURE;

            m_delayedEvents.ifExists(id, [&](DelayedEvent& delayedEvent){
                delayedEvent.remainingSeconds = newTime;
                processDelayedEvent(delayedEvent);
                result = SUCCESS;
            });

            return result;
        }

        inline Result modifyDelayedEventData(DelayedEventIdType id, T_EventData newData)
        {
            Result result = FAILURE;
            m_delayedEvents.ifExists(id, [&](DelayedEvent& delayedEvent){
                delayedEvent.data = newData;
                result = SUCCESS;
            });

            return result;
        }

        inline Result removeDelayedEvent(DelayedEventIdType id)
        {
            return m_delayedEvents.remove(id);
        }

        inline EventId getId() const { return m_id; }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_PTR(static_cast<ListenerIdType(OwnType::*)(ScriptFunctionWrapper)>(&registerListener), "registerListener")
            LUA_BIND_FUNCTION_PTR(static_cast<ListenerIdType(OwnType::*)(int16, ScriptFunctionWrapper)>(&registerListener), "registerListenerPriority")
            LUA_BIND_FUNCTION(deregisterListener)
            LUA_BIND_FUNCTION(trigger)
            LUA_BIND_FUNCTION(delayedTrigger)
            LUA_BIND_FUNCTION(modifyDelayedEventTime)
            LUA_BIND_FUNCTION(modifyDelayedEventData)
            LUA_BIND_FUNCTION(removeDelayedEvent)
            LUA_BIND_FUNCTION(getId)
        LUA_BIND_REFERENCE_TYPE_END

    private:

        struct ScriptCallerResult
        {
            enum Enum
            {
                Invalid = 0,

                // Basics
                Ok = 0x1,
                Failed = 0x2,

                // Reasons
                Begin = 0x4,
                ArgsPush = 0x8,
                Call = 0x10,
                Return = 0x20,
            };

            int32 result;

            ScriptCallerResult(int32 result) :
                result(result)
            {
            }

            bool ok() const             { return result & Ok; }
            bool failed() const           { return result & Failed; }
            bool failedBegin() const    { return result & Failed && result & Begin; }
            bool failedArgsPush() const { return result & Failed && result & ArgsPush; }
            bool failedCall() const     { return result & Failed && result & Call; }
            bool failedReturn() const   { return result & Failed && result & Return; }
        };

        template<bool B_isUsableInScript>
        struct ScriptCaller
        {
            inline static ScriptCallerResult call(IScriptingManager*,
                                                 ScriptFunctionWrapper,
                                                 T_EventData&,
                                                 EventResult::Enum& out_result)
            {
                GEP_ASSERT(false, "Attempt to call a script function "
                    "with event data that is not usable in a script!",
                    typeid(T_EventData).name());
                return ScriptCallerResult::Invalid;
            }
        };
        template<>
        struct ScriptCaller<true>
        {
            inline static ScriptCallerResult call(IScriptingManager* scripting,
                                                 ScriptFunctionWrapper funcRef,
                                                 T_EventData& data,
                                                 EventResult::Enum& out_result)
            {
                ScriptCallerResult result = ScriptCallerResult::Ok;
                auto L = scripting->getState();

                scripting->callFunctionBegin(funcRef);
                lua::push(L, data);
                scripting->actualCall(1, 1);

                if (lua_isnil(L, -1))
                {
                    lua_pop(L, 1);
                    out_result = EventResult::Handled;
                    return ScriptCallerResult::Ok;
                }

                try
                {
                    out_result = lua::pop<EventResult::Enum>(L, -1);
                }
                catch(ScriptExecutionException& ex)
                {
                    g_globalManager.getLogging()->logError(
                        "Hint: You did not return an EventResult! "
                        "Note that you do not have to return anything, "
                        "in which case EventResult.Handled is assumed.");
                    throw ex;
                }

                return result;
            }
        };

        typedef ScriptCaller<IsBoundToScript<T_EventData>::value> ScriptCallerType;

        struct ListenerWrapper
        {
            ListenerIdType id;
            int16 priority;
            EventType listener;

            operator bool() const { return id != ListenerIdType::invalidValue(); }
        };

        struct DelayedEvent
        {
            float remainingSeconds;
            T_EventData data;

            DelayedEvent() :
                remainingSeconds(-1.0f),
                data()
            {
            }
        };

        struct TriggerCounter
        {
            uint16& count;
            TriggerCounter(uint16& count) : count(count) { ++count; }
            ~TriggerCounter() { --count; }
        };

    private:

        IAllocator* m_pAllocator;
        EventId m_id;
        uint16 m_triggerLevel;
        DynamicArray<ListenerWrapper> m_listeners;
        Hashmap<DelayedEventIdType, DelayedEvent> m_delayedEvents;
        CallbackId m_callbackId_update;
        DestroyerType m_onDestroy;
        IUpdateFramework* m_pUpdateFramework;
        IScriptingManager* m_pScriptingManager;

        inline void startUpdating()
        {
            if (m_callbackId_update.id == -1)
            {
                auto callback = std::bind(&OwnType::updateDelayedEvents, this, std::placeholders::_1);
                m_callbackId_update = m_pUpdateFramework->registerUpdateCallback(callback);
            }
        }

        inline void stopUpdating()
        {
            if (m_callbackId_update.id != -1)
            {
                m_pUpdateFramework->deregisterUpdateCallback(m_callbackId_update);
                m_callbackId_update.id = -1;
            }
        }

        inline void updateDelayedEvents(float elapsedSeconds)
        {
            // If there are no delayed events, there is no need to update
            if(m_delayedEvents.count() == 0){ return; }

            m_delayedEvents.removeWhere([&](DelayedEventIdType& id, DelayedEvent& delayedEvent){
                delayedEvent.remainingSeconds -= elapsedSeconds;
                return processDelayedEvent(delayedEvent);
            });
        }

        /// \brief returns \c true if the delayed event is finished, \c false otherwise.
        inline bool processDelayedEvent(DelayedEvent& delayedEvent)
        {
            bool result = false;
            if (delayedEvent.remainingSeconds <= 0.0f)
            {
                trigger(delayedEvent.data);
                result = true;
            }
            return result;
        }

        inline void insertListener(ListenerWrapper& wrapper)
        {
            if (m_listeners.length() == 0)
            {
                m_listeners.append(wrapper);
                return;
            }

            // reverse iteration
            for (size_t index = m_listeners.length(); index > 0; --index)
            {
                auto& listener = m_listeners[index - 1];
                if (listener.priority <= wrapper.priority)
                {
                    m_listeners.insertAtIndex(index, wrapper);
                    return;
                }
            }
            m_listeners.insertAtIndex(0, wrapper);
        }

        inline Result removeListener(ListenerIdType id)
        {
            for (size_t index = 0; index < m_listeners.length(); ++index)
            {
                if (m_listeners[index].id == id)
                {
                    m_listeners.removeAtIndex(index);
                    return SUCCESS;
                }
            }
            return FAILURE;
        }

        inline EventResult::Enum call(ListenerWrapper& wrapper, T_EventData& data)
        {
            auto result = EventResult::Cancel;
            if (wrapper.listener)
            {
                result = wrapper.listener(data);
            }
            else
            {
                GEP_ASSERT(false, "Unexpected listener that has no valid actual listener.");
            }
            return result;
        }
    };

    template<>
    class Event<void>
    {
        // TODO Events of type 'void' are currently not supported!
    };
}
