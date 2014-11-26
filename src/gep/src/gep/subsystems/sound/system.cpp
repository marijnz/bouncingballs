#include "stdafx.h"
#include "gep/globalManager.h"
#include "gep/interfaces/resourceManager.h"
#include "gep/interfaces/logging.h"
#include "gepimpl/subsystems/sound/system.h"
#include "gepimpl/subsystems/sound/library.h"
#include "gepimpl/subsystems/sound/sound.h"
#include "gep/exception.h"

#include <fmod_errors.h>

gep::FmodSoundSystem::FmodSoundSystem()
{
    memset(&m_listenerAttributes, 0, sizeof(m_listenerAttributes));
    m_listenerAttributes.forward.x = 1.0f;
    m_listenerAttributes.up.z = 1.0f;
}

void gep::FmodSoundSystem::initialize()
{
    if(FMOD::Studio::System::create(&m_system) != FMOD_OK)
    {
        throw Exception("Failed to create fmod system");
    }
    if( FMOD_OK !=
        m_system.initialize(32, FMOD_STUDIO_INIT_NORMAL,     FMOD_INIT_NORMAL, nullptr)            // normal initialization
//        m_system.initialize(32, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, nullptr)            // enable fmod studio live update
    ) {
        throw Exception("failed to initialize fmod system");
    }

    FmodDummySoundLibrary* pDummySoundLibrary = new FmodDummySoundLibrary();
    pDummySoundLibrary->setLoader(new FmodDummySoundLibraryLoader());

    m_pDummyParameter = new FmodDummySoundParameter();

    m_pDummySound = new FmodDummySound();
    m_pDummySound->setLoader(new FmodDummySoundLoader());

    m_pDummySoundInstance = new FmodDummySoundInstance();
    m_pDummySoundInstance->setLoader(new FmodDummySoundInstanceLoader());

    g_globalManager.getResourceManager()->registerResourceType("FmodSoundLibrary", pDummySoundLibrary);
    g_globalManager.getResourceManager()->registerResourceType("FmodSound", m_pDummySound);
    g_globalManager.getResourceManager()->registerResourceType("FmodSoundInstance", m_pDummySoundInstance);
}

void gep::FmodSoundSystem::destroy()
{
    if(m_system.isValid())
    {
        m_system.release();
    }


    delete m_pDummyParameter;
}

void gep::FmodSoundSystem::update(float elapsedTime)
{
    m_system.update();
}

gep::ResourcePtr<gep::ISoundLibrary> gep::FmodSoundSystem::loadLibrary(const char* filename)
{
    return g_globalManager.getResourceManager()->loadResource<FmodSoundLibrary>(FmodSoundLibraryFileLoader(filename), LoadAsync::No);
}
void gep::FmodSoundSystem::loadLibraryFromLua( const char* filename )
{
    loadLibrary(filename);
}

gep::ResourcePtr<gep::ISound> gep::FmodSoundSystem::getSound(const char* name)
{
    FmodSoundLoader loader(name);
    return g_globalManager.getResourceManager()->loadResource<ISound>(loader, LoadAsync::No);
}

void gep::FmodSoundSystem::updateListenerAttributes()
{
    auto result = m_system.setListenerAttributes(&m_listenerAttributes);
    if(result != FMOD_OK)
    {
        g_globalManager.getLogging()->logWarning("Failed to set fmod listener attributes with error: %s", FMOD_ErrorString(result));
    }
}

void gep::FmodSoundSystem::setListenerPosition(const vec3& pos)
{
    memcpy(&m_listenerAttributes.position, &pos, sizeof(float)*3);
    updateListenerAttributes();
}

void gep::FmodSoundSystem::setListenerOrientation(const Quaternion& orientation)
{

    /// TODO HAAAAACK
    if (!orientation.isValid())
    {
        return;
    }
    
    auto m = orientation.toMat3();
    vec3 forward(-m.data[3], -m.data[4], -m.data[5]);
    memcpy(&m_listenerAttributes.forward, &forward, sizeof(float) * 3);
    memcpy(&m_listenerAttributes.up, m.data + 6, sizeof(float) * 3);
    updateListenerAttributes();
}

void gep::FmodSoundSystem::setListenerVelocity(const vec3& velocity)
{
    memcpy(&m_listenerAttributes.velocity, &velocity, sizeof(float) * 3);
    updateListenerAttributes();
}
