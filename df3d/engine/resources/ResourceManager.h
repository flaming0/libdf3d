#pragma once

namespace df3d {

class ThreadPool;
class ResourceFileSystem;
class IResourceHolder;
struct LoadingState;

// TODO: refactor

using ResourcePackage = std::vector<ResourceID>;

class ResourceManager : NonCopyable
{
    friend struct LoadingState;

    mutable std::recursive_mutex m_lock;
    Allocator &m_allocator;
    unique_ptr<ResourceFileSystem> m_fs;
    unique_ptr<LoadingState> m_loadingState;

    struct Entry
    {
        shared_ptr<IResourceHolder> holder;
        int refCount = 0;
        bool valid = false;
    };

    std::unordered_map<ResourceID, Entry> m_cache;

    const void* getResourceData(const ResourceID &resourceID);
    void listDependencies(const ResourcePackage &input, ResourcePackage &output, LoadingState *loadingState);
    void unloadResource(const ResourceID &resource);

public:
    ResourceManager();
    ~ResourceManager();

    void initialize();
    void shutdown();
    void poll();

    void setDefaultFileSystem();
    void setFileSystem(unique_ptr<ResourceFileSystem> fs);

    // Can load only 1 package at a time.
    void loadPackageAsync(const ResourcePackage &resources);
    void unloadPackage(const ResourcePackage &resources);
    bool isLoading() const;
    void flush();

    ResourceFileSystem& getFS() { std::lock_guard<std::recursive_mutex> lock(m_lock); return *m_fs; }

    template<typename T>
    const T* getResource(const ResourceID &resourceID) { return static_cast<const T*>(getResourceData(resourceID)); }
};

}
