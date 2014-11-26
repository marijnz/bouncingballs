#pragma once

#include "gep/interfaces/resourceManager.h"
#include "gep/container/hashmap.h"
#include "gep/container/DynamicArray.h"
#include "gep/directory.h"
#include "gep/threading/thread.h"
#include "gep/container/queue.h"
#include "gep/threading/semaphore.h"

namespace gep
{
    // forward declarations
    class ResourceLoaderThread;

    class ResourceManager
        : public IResourceManager
    {
    private:
        struct ReloadInfo
        {
            ResourcePtr<IResource> pResource;
            IResourceLoader* pLoader;
            uint32 updateNum;
        };

        struct PatchInfo
        {
            ResourcePtr<IResource> ptr;
            IResource* pResource;
            IResourceLoader* pLoader;
            bool isFinalized;
        };

        Hashmap<std::string, IResource*, StringHashPolicy> m_resourceDummies;
        Hashmap<std::string, ReloadInfo, StringHashPolicy> m_fileChangedListener;
        Hashmap<IResourceLoader*, bool, PointerHashPolicy> m_failedInitialLoad;
        Hashmap<std::string, ResourcePtr<IResource>, StringHashPolicy> m_loadedResources;
        Mutex m_fileChangedLock;
        DynamicArray<IResource*> m_newResources;
        Mutex m_newResourceMutex;
        DynamicArray<PatchInfo> m_resourcesToPatch;
        Mutex m_patchResourceMutex;
        ResourceLoaderThread* m_pResourceLoader;
        Mutex m_loadingResourceMutex;
        DirectoryWatcher m_dataDirWatcher;
        float m_timeSinceLastCheck;
        uint32 m_updateNum;

        void removeFromNewList(IResource* pResource);

        hkLoader* m_pHkResourceLoader;

    protected:
        virtual ResourcePtr<IResource> doLoadResource(IResourceLoader& loader, LoadAsync loadAsync) override;

    public:
        ResourceManager();

        // ISubsystem interface
        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        virtual void initializeInGameThread() override;
        virtual void destroyInGameThread() override;

        // IResourceManager interface
        virtual void deleteResource(IResource* pResource) override;
        virtual void registerResourceType(const char* name, IResource* pDummy) override;
        virtual void finalizeResourcesWithFlags(uint32 flags) override;
        virtual void registerLoaderForReload(const std::string& filename, IResourceLoader* pLoader, ResourcePtr<IResource> pResource) override;
        virtual void deregisterLoaderForReload(const std::string& filename, IResourceLoader* pLoader) override;
        virtual void reloadResource(ResourcePtr<IResource> pResource);

        void resourceFinishedLoading(ResourcePtr<IResource> ptr, IResource* pResource, IResourceLoader* pLoader);


        hkLoader* getHavokResourceLoader() const { return m_pHkResourceLoader; }
    };

#define g_resourceManager (*static_cast<ResourceManager*>(g_globalManager.getResourceManager()))

    class ResourceLoaderThread : public Thread
    {
    private:
        struct ToLoadInfo
        {
            ResourcePtr<IResource> ptr;
            IResourceLoader* pLoader;
        };
        Queue<ToLoadInfo, MutexLockPolicy> m_resourcesToLoad;
        Semaphore m_toLoadCounter;
        ResourceManager* m_pResourceManager;
        bool m_isRunning;

    public:

        ResourceLoaderThread(ResourceManager* pResourceManager);
        ~ResourceLoaderThread();

        void loadResource(ResourcePtr<IResource> ptr, IResourceLoader* pLoader);

        virtual void run() override;

        void stop(); // stops the resource loader thread
    };
}
