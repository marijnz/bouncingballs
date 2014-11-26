#include "stdafx.h"
#include "gepimpl/subsystems/sound/sound.h"
#include "gepimpl/subsystems/sound/system.h"
#include "gep/globalManager.h"
#include <fmod_errors.h>
#include "gep/exception.h"
#include "gep/interfaces/logging.h"

DefineWeakRefStaticMembersExport(gep::ISoundParameter)

gep::FmodSoundLoader::FmodSoundLoader(const char* path)
    : m_path(path)
    , m_isIdResolved(false)
{

}

gep::IResource* gep::FmodSoundLoader::loadResource(IResource* pInst)
{
    if(!m_isIdResolved)
    {
        FmodSoundSystem* pSystem = static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem());
        auto result = pSystem->getFmodHandle().lookupEventID(m_path.c_str(), &m_id);
        if(result != FMOD_OK)
        {
            std::ostringstream msg;
            msg << "Failed to find event '" << m_path << "' with error: " << FMOD_ErrorString(result);
            throw LoadingError(msg.str());
        }
        m_isIdResolved = true;
    }
    FmodSound* pInstance = nullptr;
    if(pInst != nullptr)
    {
        pInstance = dynamic_cast<FmodSound*>(pInst);
        GEP_ASSERT(pInstance != nullptr, "passed resource is not a FmodSound");
    }
    if(pInstance == nullptr)
    {
        pInstance = new FmodSound();
    }
    pInstance->unload();
    pInstance->load(m_id);
    return pInstance;
}

void gep::FmodSoundLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<FmodSound*>(pResource);
    GEP_ASSERT(pResource == nullptr || res != nullptr);
    delete res;
}

const char* gep::FmodSoundLoader::getResourceType()
{
    return "FmodSound";
}

void gep::FmodSoundLoader::postLoad(ResourcePtr<IResource> pResource)
{
}

gep::FmodSoundLoader* gep::FmodSoundLoader::moveToHeap()
{
    return new FmodSoundLoader(*this);
}

void gep::FmodSoundLoader::release()
{
    delete this;
}

const char* gep::FmodSoundLoader::getResourceId()
{
    return m_path.c_str();
}

gep::FmodSound::FmodSound()
    : m_pLoader(nullptr)
    , m_bLoaded(false)
    , m_numInstaces(0)
{
}

void gep::FmodSound::load(const FMOD::Studio::ID& id)
{
    FmodSoundSystem* pSystem = static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem());
    auto result = pSystem->getFmodHandle().getEvent(&id, FMOD_STUDIO_LOAD_BEGIN_NOW, &m_eventDescription);
    if(result != FMOD_OK)
    {
        std::ostringstream msg;
        msg << "Error loading event: " << FMOD_ErrorString(result);
        throw LoadingError(msg.str());
    }
    m_bLoaded = true;
}

gep::ResourcePtr<gep::ISoundInstance> gep::FmodSound::createInstance()
{
    FmodSoundInstanceLoader loader(makeResourcePtrFromThis<FmodSound>());
    return g_globalManager.getResourceManager()->loadResource<FmodSoundInstance>(loader, gep::LoadAsync::No);
}

gep::FmodSoundLoader* gep::FmodSound::getLoader()
{
    return m_pLoader;
}

void gep::FmodSound::setLoader(IResourceLoader* pLoader)
{
    auto loader = dynamic_cast<FmodSoundLoader*>(pLoader);
    GEP_ASSERT(pLoader == nullptr || loader != nullptr);
    m_pLoader = loader;
}

void gep::FmodSound::unload()
{
    if(m_bLoaded)
    {
        m_bLoaded = false;
    }
}

void gep::FmodSound::finalize()
{
}

gep::uint32 gep::FmodSound::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodSound::isLoaded()
{
    return m_bLoaded;
}

gep::IResource* gep::FmodSound::getSuperResource()
{
    return nullptr;
}

const char* gep::FmodSound::getResourceType()
{
    return "FmodSound";
}

gep::FmodSoundInstanceLoader::FmodSoundInstanceLoader(ResourcePtr<FmodSound> superResource)
    : m_superResource(superResource)
{
    std::ostringstream id;
    id << m_superResource->getLoader()->getPath() << ":" << m_superResource->m_numInstaces;
    m_resourceId = id.str();
    m_superResource->m_numInstaces++;
}

gep::IResource* gep::FmodSoundInstanceLoader::loadResource(IResource* pInPlace)
{
    auto pResource = dynamic_cast<FmodSoundInstance*>(pInPlace);
    GEP_ASSERT(pInPlace == nullptr || pResource != nullptr);
    if(pResource == nullptr)
    {
        pResource = new FmodSoundInstance();
    }
    auto result = m_superResource->m_eventDescription.createInstance(&pResource->m_eventInstance);
    if(result != FMOD_OK)
    {
        std::ostringstream msg;
        msg << "Failed to create event instance of sound '"
            << m_superResource->getLoader()->getPath()
            << "' with error: " << FMOD_ErrorString(result);
        throw LoadingError(msg.str());
    }
    pResource->m_bLoaded = true;

    return pResource;
}

void gep::FmodSoundInstanceLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<FmodSoundInstance*>(pResource);
    GEP_ASSERT(res != nullptr);
    delete res;
}

const char* gep::FmodSoundInstanceLoader::getResourceType()
{
    return "FmodSoundInstance";
}

void gep::FmodSoundInstanceLoader::release()
{
    delete this;
}

void gep::FmodSoundInstanceLoader::postLoad(ResourcePtr<IResource> pResource)
{
}

gep::FmodSoundInstanceLoader* gep::FmodSoundInstanceLoader::moveToHeap()
{
    return new FmodSoundInstanceLoader(*this);
}

const char* gep::FmodSoundInstanceLoader::getResourceId()
{
    return m_resourceId.c_str();
}

gep::FmodSoundInstance::FmodSoundInstance()
    : m_pLoader(nullptr)
    , m_bLoaded(false)
{
    memset(&m_3dAttributes, 0, sizeof(m_3dAttributes));
    m_3dAttributes.forward.x = 1.0f;
    m_3dAttributes.up.z = 1.0f;
}

gep::SoundParameter gep::FmodSoundInstance::getParameter(const char* name)
{
    std::string sName(name);
    gep::FmodSoundParameter* pParam = nullptr;
    if(!m_parameter.tryGet(sName, pParam))
    {
        FMOD::Studio::ParameterInstance fmodParam;
        auto status = m_eventInstance.getParameter(name, &fmodParam);
        if(status != FMOD_OK)
        {
            g_globalManager.getLogging()->logWarning("Couldn't find parameter '%s' for fmod event '%s'", name, getLoader()->getSuperResource()->getLoader()->getPath());
            return SoundParameter(static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem())->getDummyParameter());
        }
        else
        {
          pParam = new FmodSoundParameter();
          pParam->m_fmodParameter = fmodParam;
          m_parameter[sName] = pParam;
        }
    }
    return SoundParameter(pParam);
}

void gep::FmodSoundInstance::update3dAttributes()
{
    auto result = m_eventInstance.set3DAttributes(&m_3dAttributes);
    if(result != FMOD_OK)
    {
        g_globalManager.getLogging()->logWarning("Failed to update 3d attributes on sound instance of '%s' with error: %s", m_pLoader->getSuperResource()->getLoader()->getPath(), FMOD_ErrorString(result));
    }
}

void gep::FmodSoundInstance::setPosition(const vec3& position)
{
    memcpy(&m_3dAttributes.position, &position, sizeof(float) * 3);
    update3dAttributes();
}

void gep::FmodSoundInstance::setVelocity(const vec3& velocity)
{
    memcpy(&m_3dAttributes.velocity, &velocity, sizeof(float) * 3);
}

void gep::FmodSoundInstance::setOrientation(const Quaternion& orientation)
{
    auto m = orientation.toMat3();
    vec3 forward(-m.data[3], -m.data[4], -m.data[5]);
    memcpy(&m_3dAttributes.forward, &forward, sizeof(float) * 3);
    memcpy(&m_3dAttributes.up, m.data + 6, sizeof(float) * 3);
    update3dAttributes();
}

void gep::FmodSoundInstance::setVolume(float volume)
{
    m_eventInstance.setVolume(volume);
}

float gep::FmodSoundInstance::getVolume()
{
    float volume = 0.0f;
    auto result = m_eventInstance.getVolume(&volume);
    
    if(result != FMOD_OK)
    {
        g_globalManager.getLogging()->logWarning("Failed to get volume of sound instance of sound '%s' with error: %s", getLoader()->getSuperResource()->getLoader()->getPath(), FMOD_ErrorString(result));
    }
    return volume;
}

void gep::FmodSoundInstance::play()
{
    auto result = m_eventInstance.start();
    if(result != FMOD_OK)
    {
        g_globalManager.getLogging()->logWarning("Failed to play sound instance of sound '%s' with error: %s", getLoader()->getSuperResource()->getLoader()->getPath(), FMOD_ErrorString(result));
    }
}

void gep::FmodSoundInstance::stop()
{
    m_eventInstance.stop(FMOD_STUDIO_STOP_ALLOWFADEOUT);
}

void gep::FmodSoundInstance::setPaused(bool paused)
{
    m_eventInstance.setPaused(paused);
}

bool gep::FmodSoundInstance::getPaused()
{
    bool paused;
    m_eventInstance.getPaused(&paused);
    return paused;
}

gep::IResource* gep::FmodSoundInstance::getSuperResource()
{
    GEP_ASSERT(m_pLoader != nullptr);
    return m_pLoader->getSuperResource();
}

const char* gep::FmodSoundInstance::getResourceType()
{
    return "FmodSoundInstance";
}

gep::FmodSoundInstanceLoader* gep::FmodSoundInstance::getLoader()
{
    return m_pLoader;
}

void gep::FmodSoundInstance::setLoader(IResourceLoader* pLoader)
{
    auto res = dynamic_cast<FmodSoundInstanceLoader*>(pLoader);
    GEP_ASSERT(pLoader == nullptr || res != nullptr);
    m_pLoader = res;
}

void gep::FmodSoundInstance::unload()
{
    if(m_bLoaded)
    {
        m_bLoaded = false;
        for(FmodSoundParameter* param : m_parameter.values())
        {
            delete param;
        }
        m_parameter.clear();
        m_eventInstance.release();
    }
}

void gep::FmodSoundInstance::finalize()
{
}

gep::uint32 gep::FmodSoundInstance::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodSoundInstance::isLoaded()
{
    return m_bLoaded;
}

gep::Result gep::FmodSoundParameter::setValue(float value)
{
    if(!m_fmodParameter.isValid())
        return FAILURE;
    return (m_fmodParameter.setValue(value) == FMOD_OK) ? SUCCESS : FAILURE;
}

gep::Result gep::FmodSoundParameter::getValue(float* value) const
{
    if(!m_fmodParameter.isValid())
        return FAILURE;
    return (m_fmodParameter.getValue(value) == FMOD_OK) ? SUCCESS : FAILURE;
}

gep::Result gep::FmodDummySoundParameter::setValue(float value)
{
    return FAILURE;
}

gep::Result gep::FmodDummySoundParameter::getValue(float* value) const
{
    GEP_ASSERT(value != nullptr);
    return FAILURE;
}

gep::IResource* gep::FmodDummySoundLoader::loadResource(IResource* pInPlace)
{
    GEP_ASSERT(false, "should not be called");
    return nullptr;
}

void gep::FmodDummySoundLoader::deleteResource(IResource* pResource)
{
    delete pResource;
}

const char* gep::FmodDummySoundLoader::getResourceType()
{
    return "FmodSound";
}

void gep::FmodDummySoundLoader::release()
{
    delete this;
}

void gep::FmodDummySoundLoader::postLoad(ResourcePtr<IResource> resource)
{
}

gep::FmodDummySoundLoader* gep::FmodDummySoundLoader::moveToHeap()
{
    return new FmodDummySoundLoader(*this);
}

const char* gep::FmodDummySoundLoader::getResourceId()
{
    return "<Dummy FmodSound>";
}

const char* gep::FmodDummySoundInstanceLoader::getResourceType()
{
    return "FmodSoundInstance";
}

gep::FmodDummySoundInstanceLoader* gep::FmodDummySoundInstanceLoader::moveToHeap()
{
    return new FmodDummySoundInstanceLoader(*this);
}

const char* gep::FmodDummySoundInstanceLoader::getResourceId()
{
    return "<Dummy FmodSoundInstance>";
}

gep::FmodDummySound::FmodDummySound()
    : m_pLoader(nullptr)
{
}

gep::ResourcePtr<gep::ISoundInstance> gep::FmodDummySound::createInstance()
{
    return static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem())->getDummySoundInstance()->makeResourcePtrFromThis<ISoundInstance>();
}

gep::FmodDummySoundLoader* gep::FmodDummySound::getLoader()
{
    return m_pLoader;
}

void gep::FmodDummySound::setLoader(IResourceLoader* pLoader)
{
    auto loader = dynamic_cast<FmodDummySoundLoader*>(pLoader);
    GEP_ASSERT(pLoader == nullptr || loader != nullptr);
    m_pLoader = loader;
}

void gep::FmodDummySound::unload()
{
}

void gep::FmodDummySound::finalize()
{
}

gep::uint32 gep::FmodDummySound::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodDummySound::isLoaded()
{
    return true;
}

gep::IResource* gep::FmodDummySound::getSuperResource()
{
    return nullptr;
}

const char* gep::FmodDummySound::getResourceType()
{
    return "FmodSound";
}

gep::FmodDummySoundInstance::FmodDummySoundInstance()
    : m_pLoader(nullptr)
{
}

gep::SoundParameter gep::FmodDummySoundInstance::getParameter(const char* name)
{
    return SoundParameter(static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem())->getDummyParameter());
}

void gep::FmodDummySoundInstance::setPosition(const vec3& position)
{
}

void gep::FmodDummySoundInstance::setOrientation(const Quaternion& orientation)
{
}

void gep::FmodDummySoundInstance::setVelocity(const vec3& velocity)
{
}

void gep::FmodDummySoundInstance::setVolume(float volume)
{
}

void gep::FmodDummySoundInstance::play()
{
}

void gep::FmodDummySoundInstance::stop()
{
}

void gep::FmodDummySoundInstance::setPaused(bool)
{
}

bool gep::FmodDummySoundInstance::getPaused()
{
    return false;
}

gep::FmodDummySoundInstanceLoader* gep::FmodDummySoundInstance::getLoader()
{
    return m_pLoader;
}

void gep::FmodDummySoundInstance::setLoader(IResourceLoader* pLoader)
{
    auto loader = dynamic_cast<FmodDummySoundInstanceLoader*>(pLoader);
    GEP_ASSERT(pLoader == pLoader || loader != nullptr);
    m_pLoader = loader;
}

void gep::FmodDummySoundInstance::unload()
{
}

void gep::FmodDummySoundInstance::finalize()
{
}

gep::uint32 gep::FmodDummySoundInstance::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodDummySoundInstance::isLoaded()
{
    return true;
}

gep::IResource* gep::FmodDummySoundInstance::getSuperResource()
{
    return static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem())->getDummySound();
}

const char* gep::FmodDummySoundInstance::getResourceType()
{
    return "FmodSoundInstance";
}

float gep::FmodDummySoundInstance::getVolume()
{
    return 0.0f;
}

