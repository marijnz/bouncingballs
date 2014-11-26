#pragma once

#include "gep/interfaces/sound.h"
#include <fmod.hpp>
#include <fmod_studio.hpp>

namespace gep
{
    //forward declarations
    class FmodDummySound;
    class FmodDummySoundInstance;

    class FmodSoundSystem : public ISoundSystem
    {
    private:
        FMOD::Studio::System m_system;
        FMOD_3D_ATTRIBUTES m_listenerAttributes;
        ISoundParameter* m_pDummyParameter;
        FmodDummySound* m_pDummySound;
        FmodDummySoundInstance* m_pDummySoundInstance;

        void updateListenerAttributes();

    public:
        FmodSoundSystem();

        /// \brief returns the fmod handle of the sound system
        inline FMOD::Studio::System& getFmodHandle()
        {
            return m_system;
        }

        /// \brief returns the dummy sound parameter
        inline ISoundParameter* getDummyParameter() { return m_pDummyParameter; }

        /// \brief returns the dummy sound
        inline FmodDummySound* getDummySound() { return m_pDummySound; }

        /// \brief returns the dummy sound instance
        inline FmodDummySoundInstance* getDummySoundInstance() { return m_pDummySoundInstance; }

        // ISoundSystem interface
        virtual ResourcePtr<ISoundLibrary> loadLibrary(const char* filename) override;
        virtual ResourcePtr<ISound> getSound(const char* path) override;
        virtual void setListenerPosition(const vec3& pos) override;
        virtual void setListenerOrientation(const Quaternion& orientation) override;
        virtual void setListenerVelocity(const vec3& velocity) override;

        // ISubSystem interface
        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        virtual void loadLibraryFromLua( const char* filename );

    };
}
