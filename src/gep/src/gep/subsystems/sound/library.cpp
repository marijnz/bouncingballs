#include "stdafx.h"
#include "gep/globalManager.h"
#include "gepimpl/subsystems/sound/system.h"
#include "gepimpl/subsystems/sound/library.h"
#include <fmod_errors.h>
#include "gep/exception.h"
#include "gep/file.h"

gep::IResource* gep::IFmodSoundLibraryLoader::loadResource(IResource* pInPlace)
{
    auto res = dynamic_cast<FmodSoundLibrary*>(pInPlace);
    GEP_ASSERT(pInPlace == nullptr || res != nullptr);
    return loadResource(res);
}

void gep::IFmodSoundLibraryLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<FmodSoundLibrary*>(pResource);
    GEP_ASSERT(pResource == nullptr || res != nullptr);
    delete res;
}

const char* gep::IFmodSoundLibraryLoader::getResourceType()
{
    return "FmodSoundLibrary";
}

void gep::IFmodSoundLibraryLoader::release()
{
    delete this;
}

void gep::IFmodSoundLibraryLoader::postLoad(ResourcePtr<IResource> resource)
{
}

gep::FmodSoundLibraryFileLoader::FmodSoundLibraryFileLoader(const char* filename) :
    m_filename(filename)
{
}

gep::FmodSoundLibrary* gep::FmodSoundLibraryFileLoader::loadResource(FmodSoundLibrary* pInPlace)
{
    if(pInPlace == nullptr)
    {
        pInPlace = new FmodSoundLibrary();
    }
    pInPlace->unload();
    pInPlace->load(m_filename.c_str());
    return pInPlace;
}

gep::FmodSoundLibraryFileLoader* gep::FmodSoundLibraryFileLoader::moveToHeap()
{
    return new FmodSoundLibraryFileLoader(*this);
}

const char* gep::FmodSoundLibraryFileLoader::getResourceId()
{
    return m_filename.c_str();
}



gep::FmodSoundLibrary::FmodSoundLibrary()
    : m_isLoaded(false)
{
}

void gep::FmodSoundLibrary::load(const char* filename)
{
    if(!fileExists(filename))
    {
        std::ostringstream msg;
        msg << "The file '" << filename << "' does not exist";
        throw LoadingError(msg.str());
    }

    FmodSoundSystem* pSystem = static_cast<FmodSoundSystem*>(g_globalManager.getSoundSystem());
    FMOD_RESULT result = pSystem->getFmodHandle().loadBankFile(filename, &m_bank);
    if(result != FMOD_OK)
    {
        std::ostringstream msg;
        msg << "Failed to load sound library '" << filename << "' with fmod error '" << FMOD_ErrorString(result) << "'";
        throw LoadingError(msg.str());
    }
    m_isLoaded = true;
}

gep::IResourceLoader* gep::FmodSoundLibrary::getLoader()
{
    return m_pLoader;
}

void gep::FmodSoundLibrary::setLoader(IResourceLoader* loader)
{
    m_pLoader = dynamic_cast<IFmodSoundLibraryLoader*>(loader);
}

void gep::FmodSoundLibrary::unload()
{
    if(m_isLoaded)
    {
        m_isLoaded = false;
        m_bank.unload();
    }
}

void gep::FmodSoundLibrary::finalize()
{

}

gep::uint32 gep::FmodSoundLibrary::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodSoundLibrary::isLoaded()
{
    return m_isLoaded;
}

gep::IResource* gep::FmodSoundLibrary::getSuperResource()
{
    return nullptr;
}

const char* gep::FmodSoundLibrary::getResourceType()
{
    return "FmodSoundLibrary";
}

gep::FmodSoundLibrary* gep::FmodDummySoundLibraryLoader::loadResource(FmodSoundLibrary* pInPlace)
{
    GEP_ASSERT(false, "should not be called");
    return nullptr;
}

gep::IResource* gep::FmodDummySoundLibraryLoader::loadResource(IResource* pInPlace)
{
    return pInPlace;
}

gep::FmodDummySoundLibraryLoader* gep::FmodDummySoundLibraryLoader::moveToHeap()
{
    return new FmodDummySoundLibraryLoader(*this);
}

const char* gep::FmodDummySoundLibraryLoader::getResourceId()
{
    return "<Dummy FmodSoundLibrary>";
}

void gep::FmodDummySoundLibraryLoader::deleteResource(IResource* pResource)
{
    auto res = dynamic_cast<FmodDummySoundLibrary*>(pResource);
    GEP_ASSERT(pResource == nullptr || res != nullptr);
    delete res;
}

gep::FmodDummySoundLibrary::FmodDummySoundLibrary()
    : m_pLoader(nullptr)
{
}

gep::FmodDummySoundLibraryLoader* gep::FmodDummySoundLibrary::getLoader()
{
    return m_pLoader;
}

void gep::FmodDummySoundLibrary::setLoader(IResourceLoader* pLoader)
{
    auto loader = dynamic_cast<FmodDummySoundLibraryLoader*>(pLoader);
    GEP_ASSERT(pLoader == nullptr || loader != nullptr);
    m_pLoader = loader;
}

void gep::FmodDummySoundLibrary::unload()
{
}

void gep::FmodDummySoundLibrary::finalize()
{
}

gep::uint32 gep::FmodDummySoundLibrary::getFinalizeOptions()
{
    return 0;
}

bool gep::FmodDummySoundLibrary::isLoaded()
{
    return true;
}

gep::IResource* gep::FmodDummySoundLibrary::getSuperResource()
{
    return nullptr;
}

const char* gep::FmodDummySoundLibrary::getResourceType()
{
    return "FmodSoundLibrary";
}
