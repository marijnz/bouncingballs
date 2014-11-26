#include "stdafx.h"
#include "gpp/gameComponents/audioComponent.h"
#include "gep/interfaces/sound.h"
#include "gep/globalManager.h"
#include "gep/exception.h"
#include "gep/interfaces/logging.h"

gpp::AudioComponent::AudioComponent():
    m_sounds(),
    m_soundInstances()
{
    
}

gpp::AudioComponent::~AudioComponent()
{
}

void gpp::AudioComponent::initalize()
{
    setState(State::Active);
}

void gpp::AudioComponent::update( float elapsedS )
{
   auto pos = m_pParentGameObject->getWorldPosition();
   auto rot = m_pParentGameObject->getWorldRotation();

   for (auto sound : m_soundInstances.values())
   {
       sound->setPosition(pos);
       sound->setOrientation(rot);
   }
}

void gpp::AudioComponent::destroy()
{
    for(auto sound : m_soundInstances.values())
    {
        delete sound;
    }
    m_soundInstances.clear();
    m_sounds.clear();
}

gpp::SoundInstanceWrapper* gpp::AudioComponent::createSoundInstance( const std::string& soundName, const std::string& path )
{
    GEP_ASSERT(!soundName.empty());
    GEP_ASSERT(!path.empty());

    if(!m_sounds.exists(path))
    {
        try
        {
            m_sounds[path] = g_globalManager.getSoundSystem()->getSound(path.c_str()); 
        }
        catch(gep::LoadingError e)
        {
            g_globalManager.getLogging()->logError("Could not create SoundInstance for path %s. Maybe the path does not exist?",path.c_str());
            return nullptr;
        }
    }
    
    GEP_ASSERT(m_soundInstances.exists(soundName) == false, "A soundInstance with the name provided alredy exists!", soundName.c_str(), m_pParentGameObject->getGuid().c_str())
    auto sound = m_sounds[path]->createInstance();
    GEP_ASSERT(sound.isValid(), "Invalid sound instance.");
    auto instance = new SoundInstanceWrapper();
    instance->setInstance(sound);
    m_soundInstances[soundName] = instance;
    return instance;
}

gpp::SoundInstanceWrapper* gpp::AudioComponent::getSoundInstance( const std::string& soundName )
{
    GEP_ASSERT(m_soundInstances.exists(soundName) == true, "A soundInstance with the name provided doesn't exist!", soundName.c_str(), m_pParentGameObject->getGuid().c_str())
    
    return m_soundInstances[soundName];
}
