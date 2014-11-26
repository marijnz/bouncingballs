#pragma once

#include "gep/interfaces/subsystem.h"
#include "gep/weakPtr.h"

namespace gep
{
    //forward reference
    class IResource;
    class IResourceLoader;
    template <class T>
    struct ResourcePtr;

    struct ResourceFinalize
    {
        enum Enum
        {
            NotRequired  = 0,
            NotYet       = 0x00000001,
            FromRenderer = 0x00000002,
            FromTest     = 0x00000004
        };
    };

    enum class LoadAsync
    {
        No,
        Yes
    };

    class IResource
        : public WeakReferenced<IResource, WeakReferencedExport>
    {
        friend class IResourceManager;
    protected:
        static IResource** begin()
        {
            return s_weakTable;
        }

        static IResource** end()
        {
            return s_weakTable + s_weakTableSize;
        }
    public:
        /// \brief returns the loader which created this resource
        virtual IResourceLoader* getLoader() = 0;
        /// \brief stes the loader which created this resource
        virtual void setLoader(IResourceLoader* loader) = 0;
        /// \brief unloads the resource
        virtual void unload() = 0;
        /// \brief finalizes the resource (e.g. upload a texture to vram)
        virtual void finalize() = 0;
        /// \brief returns the finalize options \see ResourceFinalize
        virtual uint32 getFinalizeOptions() = 0;
        /// \brief checks if this resource is loaded or not
        virtual bool isLoaded() = 0;
        /// \brief returns the resource this subresource is part of
        virtual IResource* getSuperResource() = 0;
        /// \brief returns the type of the resource
        virtual const char* getResourceType() = 0;
        /// \brief creates a resource pointer from this resource
        template <class T>
        ResourcePtr<T> makeResourcePtrFromThis()
        {
            #ifdef _DEBUG
            auto ptr = dynamic_cast<T*>(this);
            #else
            auto ptr = static_cast<T*>(this);
            #endif
            return ResourcePtr<T>(ptr, false);
        }
    };

    struct GEP_API ResourcePtrBase
    {
    protected:
        WeakPtr<IResource, WeakReferencedExport> m_ptr;

    public:
        uint32 getWeakRefIndex()
        {
            return m_ptr.getWeakRefIndex();
        }
    };

    /// \brief points to a resource
    template <class T>
    struct ResourcePtr : public ResourcePtrBase
    {
        friend class IResourceManager;
        friend class IResource;

        template <class U>
        friend struct ResourcePtr;
    protected:
        ResourcePtr(T* ptr, bool isDummy)
        {
            GEP_ASSERT(ptr != nullptr, "can not create resource pointer from null");
            if(isDummy)
                m_ptr.setWithNewIndex(ptr);
            else
                m_ptr = ptr;
        }

        ResourcePtr(WeakPtr<IResource, WeakReferencedExport> ptr)
        {
            m_ptr = ptr;
        }

        void invalidateAndReplace(T* ptr)
        {
            m_ptr.invalidateAndReplace(ptr);
        }

    public:
        ResourcePtr() {}

        template <class U>
        ResourcePtr(ResourcePtr<U>& other)
        {
            static_assert(std::is_base_of<T, U>::value, "U is not a base of T");
            m_ptr = other.m_ptr;
        }

        inline operator T*()
        {
            GEP_ASSERT(m_ptr.get() != nullptr, "accessing invalid resource pointer");
            return static_cast<T*>(m_ptr.get());
        }

        inline T* operator ->()
        {
            GEP_ASSERT(m_ptr.get() != nullptr, "accessing invalid resource pointer");
            return static_cast<T*>(m_ptr.get());
        }

        inline bool isValid() const
        {
            return m_ptr.get() != nullptr;
        }

        inline T* get()
        {
            return static_cast<T*>(m_ptr.get());
        }

        template <class U>
        ResourcePtr<T>& operator = (ResourcePtr<U>& other)
        {
            static_assert(std::is_base_of<T, U>::value, "U is not a base of T");
            m_ptr = other.operator->();
            return *this;
        }

        template <class TO>
        ResourcePtr<TO> castTo()
        {
            #ifdef _DEBUG
            auto* ptr = dynamic_cast<TO*>(m_ptr.get());
            GEP_ASSERT(ptr != nullptr, "casting to wrong type");
            #endif
            return ResourcePtr<TO>(m_ptr);
        }
    };

    /// \brief class which is responsible for loading resources
    class IResourceLoader
    {
    public:
        virtual ~IResourceLoader() {}
        /// \brief loads a resource
        /// \param pInPlace, if the resource should be loaded in place (may be null)
        virtual IResource* loadResource(IResource* pInPlace) = 0;
        /// \brief returns the type of resources this loader loads
        virtual const char* getResourceType() = 0;
        /// \brief returns the id of the resource. Resources with the same id are only loaded once
        virtual const char* getResourceId() = 0;
        /// \brief deletes a resource
        virtual void deleteResource(IResource* pResource) = 0;
        /// \brief creates a duplicate of this resource loader
        virtual IResourceLoader* moveToHeap() = 0;
        /// \brief deletes this resource loader (previoulsy created with moveToHeap)
        virtual void release() = 0;
        /// \brief gets called after each load operation
        virtual void postLoad(ResourcePtr<IResource> pResource) = 0;
    };

    /// \brief interface for the resource manager
    class IResourceManager : public ISubsystem
    {
    protected:
        virtual ResourcePtr<IResource> doLoadResource(IResourceLoader& loader, LoadAsync loadAsync) = 0;

        inline static IResource** begin() { return IResource::begin(); }
        inline static IResource** end() { return IResource::end(); }
        inline static void invalidateAndReplace(ResourcePtr<IResource>& ptr, IResource* replaceWith)
        {
            ptr.invalidateAndReplace(replaceWith);
        }

        inline static ResourcePtr<IResource> makeResourcePtr(IResource* pResource, bool isDummy)
        {
            GEP_ASSERT(pResource != nullptr);
            return ResourcePtr<IResource>(pResource, isDummy);
        }
    public:
        virtual ~IResourceManager() {}

        virtual void initializeInGameThread() = 0;
        virtual void destroyInGameThread() = 0;

        /// \brief reloads a resource
        virtual void reloadResource(ResourcePtr<IResource> pResource) = 0;

        /// \brief loads a resource
        template <class T>
        inline ResourcePtr<T> loadResource(IResourceLoader& loader, LoadAsync loadAsync = LoadAsync::Yes)
        {
            return doLoadResource(loader, loadAsync).castTo<T>();
        }

        /// \brief deletes a resource
        virtual void deleteResource(IResource* pResource) = 0;

        /// \brief registeres a new resource type
        /// \param name
        ///   the name of the resource type
        /// \param pDummy
        ///   a pointer to the dummy resource which should be used for this type
        virtual void registerResourceType(const char* name, IResource* pDummy) = 0;

        /// \brief finalizes resources with a given set of flags
        virtual void finalizeResourcesWithFlags(uint32 flags) = 0;

        /// \brief registers a resource loader for a file changed event so that the assoicated resource will be
        ///  be reloaded as soon as the file changes
        virtual void registerLoaderForReload(const std::string& filename, IResourceLoader* pLoader, ResourcePtr<IResource> pResource) = 0;

        /// \brief deregisters a previously registered resource loader
        virtual void deregisterLoaderForReload(const std::string& filename, IResourceLoader* pLoader) = 0;
    };
}
