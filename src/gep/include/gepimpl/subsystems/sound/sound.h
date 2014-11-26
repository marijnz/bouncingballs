#pragma once

#include "gep/interfaces/sound.h"
#include "gep/container/Hashmap.h"
#include "gep/container/DynamicArray.h"
#include <fmod.hpp>
#include <fmod_studio.hpp>

namespace gep
{
    // Forward declarations
    class FmodSoundInstance;
    class FmodSoundParameter;

    class FmodSoundLoader : public IResourceLoader
    {
    private:
        std::string m_path;
        FMOD::Studio::ID m_id;
        bool m_isIdResolved;

    public:
        FmodSoundLoader(const char* path);

        inline const char* getPath() const { return m_path.c_str(); }

        // IResourceLoader interface
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual void deleteResource(IResource* pResource) override;
        virtual const char* getResourceType() override;
        virtual void release() override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual FmodSoundLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    class FmodSound : public ISound
    {
        friend class FmodSoundInstanceLoader;
    private:
        uint32 m_numInstaces;
        FMOD::Studio::EventDescription m_eventDescription;
        FmodSoundLoader* m_pLoader;
        bool m_bLoaded;

    public:

        FmodSound();

        void load(const FMOD::Studio::ID& id);

        //ISound interface
        virtual ResourcePtr<ISoundInstance> createInstance() override;

        //IResource interface
        virtual FmodSoundLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
        virtual bool isLoaded() override;
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;
    };

    class FmodSoundInstanceLoader : public IResourceLoader
    {
    private:
        ResourcePtr<FmodSound> m_superResource;
        std::string m_resourceId;

    public:
        FmodSoundInstanceLoader(ResourcePtr<FmodSound> superResource);

        inline ResourcePtr<FmodSound> getSuperResource() { return m_superResource; }

        // IResourceLoader interface
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual void deleteResource(IResource* pResource) override;
        virtual const char* getResourceType() override;
        virtual void release() override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual FmodSoundInstanceLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    class FmodSoundInstance : public ISoundInstance
    {
        friend class FmodSoundInstanceLoader;
    private:
        Hashmap<std::string, FmodSoundParameter*, StringHashPolicy> m_parameter;
        FMOD::Studio::EventInstance m_eventInstance;
        FMOD_3D_ATTRIBUTES m_3dAttributes;
        FmodSoundInstanceLoader* m_pLoader;
        bool m_bLoaded;

        void update3dAttributes();

    public:
        FmodSoundInstance();

        //ISoundInstance interface
        SoundParameter getParameter(const char* name) override;
        virtual void setPosition(const vec3& position) override;
        virtual void setOrientation(const Quaternion& orientation) override;
        virtual void setVelocity(const vec3& velocity) override;
        virtual void setVolume(float volume) override;
        virtual void play() override;
        virtual void stop() override;
        virtual void setPaused(bool paused) override;
        virtual bool getPaused() override;

        //IResource interface
        virtual FmodSoundInstanceLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
        virtual bool isLoaded() override;
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;

        virtual float getVolume();

    };

    class FmodSoundParameter : public ISoundParameter
    {
        friend class FmodSoundInstance;
    private:
        FMOD::Studio::ParameterInstance m_fmodParameter;
    public:

        //ISoundParameter interface
        virtual Result setValue(float value) override;
        virtual Result getValue(float* value) const override;
    };

    class FmodDummySoundParameter : public ISoundParameter
    {
    public:
        //ISoundParameter interface
        virtual Result setValue(float value) override;
        virtual Result getValue(float* value) const override;
    };

    class FmodDummySoundLoader : public IResourceLoader
    {
    public:
        virtual IResource* loadResource(IResource* pInPlace) override;
        virtual void deleteResource(IResource* pResource) override;
        virtual const char* getResourceType() override;
        virtual void release() override;
        virtual void postLoad(ResourcePtr<IResource> pResource) override;
        virtual FmodDummySoundLoader* moveToHeap() override;
        virtual const char* getResourceId() override;
    };

    class FmodDummySoundInstanceLoader : public FmodDummySoundLoader
    {
    public:
        virtual const char* getResourceId() override;
        virtual FmodDummySoundInstanceLoader* moveToHeap() override;
        virtual const char* getResourceType() override;
    };

    class FmodDummySound : public ISound
    {
    private:
        FmodDummySoundLoader* m_pLoader;
    public:
        FmodDummySound();

        //ISound interface
        virtual ResourcePtr<ISoundInstance> createInstance() override;

        //IResource interface
        virtual FmodDummySoundLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
        virtual bool isLoaded() override;
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;
    };

    class FmodDummySoundInstance : public ISoundInstance
    {
    private:
        FmodDummySoundInstanceLoader* m_pLoader;
    public:
        FmodDummySoundInstance();

        //ISoundInstance interface
        SoundParameter getParameter(const char* name) override;
        virtual void setPosition(const vec3& position) override;
        virtual void setOrientation(const Quaternion& orientation) override;
        virtual void setVelocity(const vec3& velocity) override;
        virtual void setVolume(float volume) override;
        virtual void play() override;
        virtual void stop() override;
        virtual void setPaused(bool paused) override;
        virtual bool getPaused() override;

        //IResource interface
        virtual FmodDummySoundInstanceLoader* getLoader() override;
        virtual void setLoader(IResourceLoader* loader) override;
        virtual void unload() override;
        virtual void finalize() override;
        virtual uint32 getFinalizeOptions() override;
        virtual bool isLoaded() override;
        virtual IResource* getSuperResource() override;
        virtual const char* getResourceType() override;

        virtual float getVolume();

    };
};
