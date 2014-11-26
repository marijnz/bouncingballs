#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gep/weakPtr.h"
#include "gep/interfaces/resourceManager.h"

namespace gep
{
    // forward declarations
    class ISoundLibrary;
    class ISound;
    class ISoundInstance;

    /// \brief the sound system interface
    class ISoundSystem : public ISubsystem
    {
    public:
        virtual ~ISoundSystem(){}
        virtual ResourcePtr<ISoundLibrary> loadLibrary(const char* filename) = 0;
        virtual void loadLibraryFromLua(const char* filename) = 0;
        virtual ResourcePtr<ISound> getSound(const char* path) = 0;
        virtual void setListenerPosition(const vec3& pos) = 0;
        virtual void setListenerOrientation(const Quaternion& orientation) = 0;
        virtual void setListenerVelocity(const vec3& velocity) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION_NAMED(loadLibraryFromLua, "loadLibrary")
        LUA_BIND_REFERENCE_TYPE_END;
    };

    /// \brief a library containing multiple sounds
    class ISoundLibrary : public IResource
    {
    public:
        virtual ~ISoundLibrary(){}
    };

    /// \brief interface for a sound parameter
    class ISoundParameter : public WeakReferenced<ISoundParameter, WeakReferencedExport>
    {
    public:
        virtual ~ISoundParameter(){}

        virtual Result setValue(float value) = 0;
        virtual Result getValue(float* value) const = 0;
    };

    /// \brief sound parameter
    ///   can be placed on the stack or inside classes
    class SoundParameter : public WeakPtr<ISoundParameter, WeakReferencedExport>
    {
    public:
        SoundParameter(){}
        SoundParameter(ISoundParameter* pImpl) : WeakPtr(pImpl){}

        inline Result setValue(float value)
        {
            ISoundParameter* pImpl = get();
            GEP_ASSERT(pImpl != nullptr, "instance does no longer exist");
            return pImpl->setValue(value);
        }

        inline Result getValue(float* value) const
        {
            const ISoundParameter* pImpl = get();
            GEP_ASSERT(pImpl != nullptr, "instance does no longer exist");
            return pImpl->getValue(value);
        }
    };

    /// \brief interface for a sound which holds audio data
    class ISound : public IResource
    {
    public:
        virtual ~ISound(){}

        virtual ResourcePtr<ISoundInstance> createInstance() = 0;
    };

    /// \brief
    ///    interface of a sound instance. Multiple sound instances can share the same audio data from a single sound.
    class ISoundInstance : public IResource
    {
    public:
        virtual ~ISoundInstance(){}

        virtual SoundParameter getParameter(const char* name) = 0;

        virtual void setPosition(const vec3& position) = 0;
        virtual void setOrientation(const Quaternion& orientation) = 0;
        virtual void setVelocity(const vec3& velocity) = 0;
        virtual void setVolume(float volume) = 0;
        virtual float getVolume() = 0;
        virtual void play() = 0;
        virtual void stop() = 0;
        virtual void setPaused(bool paused) = 0;
        virtual bool getPaused() = 0;
    };
}
