#pragma once
#include "gep/interfaces/scripting.h"
#include "gep/interfaces/updateFramework.h"

namespace gep
{
    class ICamera;
    class IRendererExtractor;

    namespace settings
    {
        struct General;
    }
}

namespace gpp { namespace sm {

    class StateMachineFactory;
    class StateMachine;

}} // namespace gpp::sm

namespace gpp
{
    class GameObjectManager;

    struct VSyncMode
    {
        enum Enum
        {
            EnumMin = -1,

            Off,
            On,
            Adaptive,

            EnumCount
        };

        VSyncMode(Enum value) : value(value){}

        inline bool isOff() const { return value == Off; }
        inline bool isOn() const { return value == On; }
        inline bool isAdaptive() const { return value == Adaptive; }

        inline void advanceToNext()
        {
            value = Enum(value + 1);
            if (value == EnumCount)
            {
                value = Off;
            }
        }

        inline void advanceToPrevious()
        {
            value = Enum(value - 1);
            if (value == EnumMin)
            {
                value = Adaptive;
            }
        }

        inline const char* toString() const
        {
            switch(value)
            {
            case gpp::VSyncMode::Off:      return "off";
            case gpp::VSyncMode::On:       return "on";
            case gpp::VSyncMode::Adaptive: return "adaptive";
            }

            return "<<INVALID>>";
        }

        Enum value;
    };

    class GPP_API Game
    {
        sm::StateMachineFactory* m_pStateMachineFactory;

        gep::ICamera* m_pDummyCam;
        sm::StateMachine* m_pStateMachine;
        bool m_continueRunningGame;
        gep::CallbackId m_memoryManagerUpdate;
        VSyncMode m_vsyncMode;
        bool m_printDebugInfo;

    public:
        Game();

        virtual ~Game();

        void initialize();
        void destroy();
        void update(float elapsedTime);
        void render(gep::IRendererExtractor& extractor);

        void setUpStateMachine();
        void makeScriptBindings();

        void bindEnums();
        void bindOther();

        inline sm::StateMachine* getStateMachine()
        {
            GEP_ASSERT(m_pStateMachine, "state machine not initialized!");
            return m_pStateMachine;
        }

        inline sm::StateMachineFactory* getStateMachineFactory()
        {
            GEP_ASSERT(m_pStateMachineFactory, "state machine factory not initialized!");
            return m_pStateMachineFactory;
        }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(getStateMachine)
            LUA_BIND_FUNCTION(getStateMachineFactory)
        LUA_BIND_REFERENCE_TYPE_END

    private:

        void toggleDisplayMemoryStatistics();

        void updateVSyncState();
    };
}
