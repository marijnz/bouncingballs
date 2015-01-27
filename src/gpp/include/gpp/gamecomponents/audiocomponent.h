#pragma once
#include "gep/interfaces/scripting.h"
#include "gpp/gameObjectSystem.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/sound.h"
#include "gep/types.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

namespace gpp
{
    class SoundInstanceWrapper;
    class AudioComponent : public Component
    {
    public:
        AudioComponent();
        virtual ~AudioComponent();

        virtual void initalize();

        virtual void update( float elapsedMS );

        virtual void destroy();

        SoundInstanceWrapper* createSoundInstance(const std::string& soundName, const std::string& path);

        SoundInstanceWrapper* getSoundInstance(const std::string& soundName);

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(createSoundInstance)
            LUA_BIND_FUNCTION(getSoundInstance)
        LUA_BIND_REFERENCE_TYPE_END;

    private:
        gep::Hashmap<std::string, SoundInstanceWrapper*, gep::StringHashPolicy> m_soundInstances;
        gep::Hashmap<std::string, gep::ResourcePtr<gep::ISound>, gep::StringHashPolicy> m_sounds;

    };

    template<>
    struct ComponentMetaInfo<AudioComponent>
    {
        static const char* name(){ return "AudioCComponent"; }
        static const gep::int32 initializationPriority() { return 65; }
        static const gep::int32 updatePriority() { return 42; }
    };

    class SoundInstanceWrapper
    {
    public:
        SoundInstanceWrapper() : m_pSoundInstance(){}

        virtual ~SoundInstanceWrapper(){}

        void setInstance(gep::ResourcePtr<gep::ISoundInstance> instance){m_pSoundInstance = instance;}

        void play(){m_pSoundInstance->play();}

        bool getPaused(){return m_pSoundInstance->getPaused();}

        void setPaused(bool paused){m_pSoundInstance->setPaused(paused);}

        void setVolume(float volume){m_pSoundInstance->setVolume(volume);}

        float getVolume(){return m_pSoundInstance->getVolume();}

        void stop(){return m_pSoundInstance->stop();}

        void setPosition(const gep::vec3& position){m_pSoundInstance->setPosition((position));}

        void setOrientation(const gep::Quaternion& orientation){m_pSoundInstance->setOrientation(orientation);}

        float getParameter(const char* parameterName)
        {
            float value = 0.0;
            auto result = m_pSoundInstance->getParameter(parameterName).getValue(&value);
            if (result != gep::Result::SUCCESS)
            {
                g_globalManager.getLogging()->logError("The sound parameter %s does not exist!", parameterName);
                return 0.0;
            }
            return value;
        }

        void setParameter(const char* parameterName, float value)
        {
            auto result = m_pSoundInstance->getParameter(parameterName).setValue(value);
            if (result != gep::Result::SUCCESS)
            {
                g_globalManager.getLogging()->logError("The sound parameter %s does not exist!", parameterName);
            }
        }

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(play)
            LUA_BIND_FUNCTION(stop)
            LUA_BIND_FUNCTION(getPaused)
            LUA_BIND_FUNCTION(setPaused)
            LUA_BIND_FUNCTION(setVolume)
            LUA_BIND_FUNCTION(getVolume)
            LUA_BIND_FUNCTION(getParameter)
            LUA_BIND_FUNCTION(setParameter)
        LUA_BIND_REFERENCE_TYPE_END

    private:
        gep::ResourcePtr<gep::ISoundInstance> m_pSoundInstance;
    };
}
