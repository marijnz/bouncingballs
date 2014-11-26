#include "stdafx.h"
#include "gepimpl/subsystems/resourceManager.h"
#include "gep/exception.h"
#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"
#include "gep/interfaces/updateFramework.h"
#include "gep/file.h"
#include <algorithm>

DefineWeakRefStaticMembersExport(gep::IResource)

gep::ResourceManager::ResourceManager()
  : m_dataDirWatcher("data", DirectoryWatcher::WatchSubdirs::yes, DirectoryWatcher::Watch::writes),
    m_timeSinceLastCheck(0.1f),
    m_updateNum(0)
    , m_pResourceLoader(nullptr)
    , m_pHkResourceLoader(nullptr)
{
}

void gep::ResourceManager::initialize()
{
    m_pResourceLoader = new ResourceLoaderThread(this);
    m_pResourceLoader->start();
}

void gep::ResourceManager::destroy()
{
    m_pResourceLoader->stop();
    m_pResourceLoader->join();
    delete m_pResourceLoader;

    // Make a copy to allow safe iterating
    auto resourceToPatchCopy = m_resourcesToPatch;
    Hashmap<IResource*, bool> alreadyDeleted;
    for(auto& info : resourceToPatchCopy)
    {
        if(!alreadyDeleted.exists(info.pResource))
        {
            if(info.pResource != nullptr)
            {
                deleteResource(info.pResource);
                alreadyDeleted[info.pResource] = true;
            }
            else
                info.pLoader->release();
        }
    }
    IResource** e = end();
    for(IResource** it = begin(); it < e; ++it)
    {
        auto ptr = *it;
        if(ptr != nullptr)
        {
            IResource* pResource = ptr;
            if(pResource != nullptr)
            {
                deleteResource(pResource);
            }
        }
    }
    for(auto pLoader : m_failedInitialLoad.keys())
    {
        pLoader->release();
    }
    for(IResource* pDummy : m_resourceDummies.values())
    {
        if(pDummy != nullptr)
        {
            IResourceLoader* pLoader = pDummy->getLoader();
            pDummy->unload();
            pLoader->deleteResource(pDummy);
            pLoader->release();
        }
    }
    m_failedInitialLoad.clear();
    m_resourceDummies.clear();
    m_fileChangedListener.clear();
    m_loadedResources.clear();
}

void gep::ResourceManager::update(float elapsedTime)
{
    {
        ScopedLock<Mutex> lock(m_patchResourceMutex);
        for(size_t i=0; i<m_resourcesToPatch.length();)
        {
            auto& info = m_resourcesToPatch[i];
            IResource* pDummyResource = nullptr;
            if(!m_resourceDummies.tryGet(std::string(info.pLoader->getResourceType()), pDummyResource))
            {
                GEP_ASSERT(false, "resource type not registered yet");
            }
            if(info.pResource == nullptr) // failed load
            {
                if(info.ptr == pDummyResource)
                    m_failedInitialLoad[info.pLoader] = true;

                info.pLoader->postLoad(info.ptr);
                m_resourcesToPatch.removeAtIndexUnordered(i); //instead of i++
            }
            else if(info.isFinalized) // load successfull and finalized
            {
                if(m_failedInitialLoad.exists(info.pLoader))
                    m_failedInitialLoad.remove(info.pLoader);

                if(info.ptr == pDummyResource)
                {
                    //replacing a dummy resource
                    invalidateAndReplace(info.ptr, info.pResource);
                    GEP_ASSERT(info.ptr == info.pResource);
                }
                else
                {
                    // replacing a valid resource
                    if(info.ptr != info.pResource) // do not swap on inplace reloading
                    {
                        IResource* pResourceToReplace = info.ptr;
                        pResourceToReplace->swapPlaces(*info.pResource);
                        info.pLoader->deleteResource(pResourceToReplace);
                        GEP_ASSERT(info.ptr == info.pResource);
                    }
                }

                info.pLoader->postLoad(info.ptr);
                m_resourcesToPatch.removeAtIndexUnordered(i); //instead of i++
            }
            else
            {
                i++;
            }
        }
    }

    m_timeSinceLastCheck -= elapsedTime;
    if(m_timeSinceLastCheck <= 0.0f)
    {
        m_updateNum++;
        ScopedLock<Mutex> lock(m_fileChangedLock);
        m_dataDirWatcher.enumerateChanges([=](const char* filename, DirectoryWatcher::Action::Enum action)
        {
            if(action != DirectoryWatcher::Action::modified)
                return;
            std::string path = "data\\" + std::string(filename);
            ReloadInfo info;
            if(m_fileChangedListener.tryGet(path, info))
            {
                // Some modified events come in twice. Make sure to not handle them twice
                if(info.updateNum == m_updateNum)
                    return;
                g_globalManager.getLogging()->logMessage("Reloading '%s' resource from file '%s'.", info.pLoader->getResourceType(), filename);
                m_fileChangedListener[path].updateNum = m_updateNum;
                // the modified file might still be be open for writing, wait until its possible to read it
                Sleep(10);
                while(true)
                {
                    RawFile file(path.c_str(), "r");
                    if(file.isOpen())
                        break;
                    Sleep(100);
                }
                try {
                    IResource* pDummyResource = nullptr;
                    IResource* pResourceToReload = info.pResource;
                    if(!m_resourceDummies.tryGet(std::string(info.pLoader->getResourceType()), pDummyResource))
                    {
                        GEP_ASSERT(false, "resource type not registered yet");
                    }
                    IResource* pNewResource = info.pLoader->loadResource((pDummyResource == pResourceToReload) ? nullptr : pResourceToReload);
                    if(pNewResource != nullptr)
                    {
                        if(m_failedInitialLoad.exists(info.pLoader))
                            m_failedInitialLoad.remove(info.pLoader);
                        if(pResourceToReload != nullptr && pResourceToReload != pDummyResource)
                        {
                            // swap if it did not reload inplace
                            if(pResourceToReload != pNewResource)
                            {
                                pNewResource->swapPlaces(*pResourceToReload);
                                removeFromNewList(pResourceToReload);
                                info.pLoader->deleteResource(pResourceToReload);
                            }
                        }
                        else
                        {
                            //replace the dummy resource
                            invalidateAndReplace(info.pResource, pNewResource);
                        }

                        if(pNewResource->getFinalizeOptions() > 0)
                        {
                            ScopedLock<Mutex> lock(m_newResourceMutex);
                            m_newResources.append(pNewResource);
                        }
                        pNewResource->setLoader(info.pLoader);
                    }
                    else
                    {
                        g_globalManager.getLogging()->logError("Failed to reload '%s' resource from file '%s'. Loader returned null.", info.pLoader->getResourceType(), filename);
                    }
                }
                catch(LoadingError& ex)
                {
                    g_globalManager.getLogging()->logError("Failed to reload '%s' resource from file '%s' error:\n%s", info.pLoader->getResourceType(), filename, ex.what());
                }
            }
        });
        m_timeSinceLastCheck = 0.1f;
    }
}

void gep::ResourceManager::initializeInGameThread()
{
    m_pHkResourceLoader = new hkLoader();
}

void gep::ResourceManager::destroyInGameThread()
{
    DELETE_AND_NULL(m_pHkResourceLoader);
}

gep::ResourcePtr<gep::IResource> gep::ResourceManager::doLoadResource(IResourceLoader& loader, LoadAsync loadAsync)
{
    // Because of the insert and lookup into m_loadedResources we have to lock the entire method
    // This is only going to be a problem for synchronous loads because they will block for a long time
    ScopedLock<Mutex> outerLock(m_loadingResourceMutex);
    ResourcePtr<IResource> alreadyLoaded;
    std::string resourceId(loader.getResourceId());
    if(m_loadedResources.tryGet(resourceId, alreadyLoaded))
    {
        g_globalManager.getLogging()->logMessage("Reusing already loaded resource '%s'", resourceId.c_str());
        return alreadyLoaded;
    }
    IResourceLoader* pLoader = loader.moveToHeap();
    GEP_ASSERT(pLoader != nullptr);
    if(loadAsync == LoadAsync::No)
    {
        try {
            IResource* pResult = pLoader->loadResource(nullptr);
            if(pResult != nullptr)
            {
                if(pResult->getFinalizeOptions() > 0)
                {
                    ScopedLock<Mutex> lock(m_newResourceMutex);
                    m_newResources.append(pResult);
                }
                pResult->setLoader(pLoader);

                auto result = makeResourcePtr(pResult, false);
                pLoader->postLoad(result);
                m_loadedResources[pLoader->getResourceId()] = result;
                GEP_ASSERT(result != nullptr);
                return result;
            }
        }
        catch(LoadingError& ex)
        {
            g_globalManager.getLogging()->logError("%s", ex.what());
        }
    }
    IResource* pResult = nullptr;
    if(loadAsync == LoadAsync::No)
    {
        m_failedInitialLoad[pLoader] = true;
    }
    m_resourceDummies.tryGet(std::string(pLoader->getResourceType()), pResult);
    GEP_ASSERT(pResult != nullptr, "Unkown resource type", pLoader->getResourceType());
    auto result = makeResourcePtr(pResult, true);
    m_loadedResources[pLoader->getResourceId()] = result;
    if(loadAsync == LoadAsync::Yes)
    {
        GEP_ASSERT(result == pResult);
        m_pResourceLoader->loadResource(result, pLoader);
    }
    else
    {
        pLoader->postLoad(result);
    }
    GEP_ASSERT(result != nullptr);
    return result;
}

void gep::ResourceManager::removeFromNewList(IResource* pResource)
{
    ScopedLock<Mutex> lock(m_newResourceMutex);
    for(size_t i=0; i < m_newResources.length(); i++)
    {
        if(m_newResources[i] == pResource)
        {
            m_newResources.removeAtIndexUnordered(i);
            break;
        }
    }
}

void gep::ResourceManager::reloadResource(ResourcePtr<IResource> ptr)
{
    IResource* pDummyResource = nullptr;
    if(!m_resourceDummies.tryGet(ptr->getResourceType(), pDummyResource))
    {
        GEP_ASSERT(false, "Resource type not registered yet", ptr->getResourceType());
    }
    IResource* result = nullptr;
    IResourceLoader* pLoader = ptr->getLoader();
    try {
        IResource* toReload = ptr;
        result = pLoader->loadResource((toReload == pDummyResource) ? nullptr : toReload);
        if(result != nullptr)
            result->setLoader(pLoader);
    }
    catch(LoadingError& ex)
    {
        g_globalManager.getLogging()->logError("Error reloading '%s':\n%s", ptr->getLoader()->getResourceId(), ex.what());
    }
    if(result != nullptr)
        resourceFinishedLoading(ptr, result, pLoader);
}

void gep::ResourceManager::deleteResource(IResource* pResource)
{
    if(pResource == nullptr)
        return;
    auto pLoader = pResource->getLoader();
    std::string resourceId = pLoader->getResourceId();
    if(m_loadedResources.exists(resourceId))
        m_loadedResources.remove(resourceId);
    IResource* pDummyResource = nullptr;
    if(!m_resourceDummies.tryGet(pLoader->getResourceType(), pDummyResource))
    {
        GEP_ASSERT(false, "resource type not registered yet");
    }
    if(pResource != pDummyResource)
    {
        removeFromNewList(pResource);
        {
            // remove the resource from the to patch table
            ScopedLock<Mutex> lock(m_patchResourceMutex);
            for(size_t i=0; i < m_resourcesToPatch.length(); )
            {
                auto& info = m_resourcesToPatch[i];
                if(info.ptr == pResource || info.pResource == pResource)
                {
                    if(info.ptr != info.pResource && info.pResource != pResource)
                        pLoader->deleteResource(info.pResource);
                    m_resourcesToPatch.removeAtIndexUnordered(i); // instead of i++
                }
                else
                    i++;
            }
        }
        pResource->unload();
        pLoader->deleteResource(pResource);
        pLoader->release();
    }
}

void gep::ResourceManager::registerResourceType(const char* name, IResource* pDummy)
{
    GEP_ASSERT(!m_resourceDummies.exists(std::string(name)), "resource type already exists");
    m_resourceDummies[std::string(name)] = pDummy;
}

void gep::ResourceManager::finalizeResourcesWithFlags(uint32 flags)
{
    {
    ScopedLock<Mutex> lock(m_newResourceMutex);
    for(size_t i=0; i < m_newResources.length(); )
    {
        IResource* pResource = m_newResources[i];
        uint32 resourceFlags = pResource->getFinalizeOptions();
        if((resourceFlags & ResourceFinalize::NotYet) == 0 && (resourceFlags & flags) > 0)
        {
            pResource->finalize();
            m_newResources.removeAtIndexUnordered(i); // this removes a element => we don't need to increment the index
        }
        else
        {
            i++;
        }
    }
    }

    {
        ScopedLock<Mutex> lock(m_patchResourceMutex);
        for(auto& toPatch : m_resourcesToPatch)
        {
            if(!toPatch.isFinalized)
            {
                uint32 resourceFlags = toPatch.pResource->getFinalizeOptions();
                if((resourceFlags & ResourceFinalize::NotYet) == 0 && (resourceFlags & flags) > 0)
                {
                    toPatch.pResource->finalize();
                    toPatch.isFinalized = true;
                }
            }
        }
    }
}

void gep::ResourceManager::registerLoaderForReload(const std::string& filename, IResourceLoader* pLoader, ResourcePtr<IResource> pResource)
{
    std::string fixedpath(filename);
    std::replace(fixedpath.begin(), fixedpath.end(), '/', '\\');
    {
      size_t pos = 0;
      while((pos = fixedpath.find("\\\\", pos)) != std::string::npos)
      {
         fixedpath.replace(pos, 2, "\\");
         pos += 1;
      }
    }
    if(fixedpath.substr(0, 2) == ".\\")
      fixedpath = fixedpath.substr(2);
    ScopedLock<Mutex> lock(m_fileChangedLock);
    ReloadInfo& info = m_fileChangedListener[fixedpath];
    info.pLoader = pLoader;
    info.pResource = pResource;
    info.updateNum = m_updateNum;
}

void gep::ResourceManager::deregisterLoaderForReload(const std::string& filename, IResourceLoader* pLoader)
{
    std::string fixedpath(filename);
    std::replace(fixedpath.begin(), fixedpath.end(), '/', '\\');
    ScopedLock<Mutex> lock(m_fileChangedLock);
    ReloadInfo info;
    if(m_fileChangedListener.tryGet(fixedpath, info))
    {
        if(info.pLoader == pLoader)
            m_fileChangedListener.remove(fixedpath);
    }
}

void gep::ResourceManager::resourceFinishedLoading(ResourcePtr<IResource> ptr, IResource* pResource, IResourceLoader* pLoader)
{
    PatchInfo info;
    info.ptr = ptr;
    info.pResource = pResource;
    info.pLoader = pLoader;
    info.isFinalized = (pResource->getFinalizeOptions() == 0);
    ScopedLock<Mutex> lock(m_patchResourceMutex);
    m_resourcesToPatch.append(info);
}

gep::ResourceLoaderThread::ResourceLoaderThread(ResourceManager* pResourceManager) :
    m_toLoadCounter(0),
    m_pResourceManager(pResourceManager),
    m_isRunning(false)
{

}

gep::ResourceLoaderThread::~ResourceLoaderThread()
{
    GEP_ASSERT(m_isRunning == false, "resource loader should not be running");
    while(m_resourcesToLoad.count() > 0)
    {
        m_resourcesToLoad.take().pLoader->release();
    }
}

void gep::ResourceLoaderThread::loadResource(ResourcePtr<IResource> ptr, IResourceLoader* pLoader)
{
    ToLoadInfo info;
    info.ptr = ptr;
    info.pLoader = pLoader;
    m_resourcesToLoad.append(info);
    m_toLoadCounter.increment();
}

void gep::ResourceLoaderThread::run()
{
    m_isRunning = true;
    SCOPE_EXIT{ m_isRunning = false; });
    while(m_isRunning)
    {
//        Sleep(100);
        m_toLoadCounter.waitAndDecrement();
        // We might got signaled to quit, check this
        if(!m_isRunning)
            break;
        auto info = m_resourcesToLoad.take();
        IResource* pResult = nullptr;
        try
        {
            pResult = info.pLoader->loadResource(nullptr);
            if(pResult != nullptr)
                pResult->setLoader(info.pLoader);
        }
        catch(LoadingError& ex)
        {
            g_globalManager.getLogging()->logError("%s", ex.what());
        }
        if(pResult != nullptr)
        {
            m_pResourceManager->resourceFinishedLoading(info.ptr, pResult, info.pLoader);
        }
        else
        {
            info.pLoader->release();
        }
    }
}

void gep::ResourceLoaderThread::stop()
{
    m_isRunning = false;
    m_toLoadCounter.increment();
}


