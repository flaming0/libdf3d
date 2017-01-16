#pragma once

#include "IResourceHolder.h"

namespace df3d {

struct EntityResource
{
    rapidjson::Document root;
};

class EntityHolder : public IResourceHolder
{
    EntityResource *m_resource = nullptr;

public:
    void listDependencies(ResourceDataSource &dataSource, std::vector<std::string> &outDeps) override;
    bool decodeStartup(ResourceDataSource &dataSource, Allocator &allocator) override;
    void decodeCleanup(Allocator &allocator) override;
    bool createResource(Allocator &allocator) override;
    void destroyResource(Allocator &allocator) override;

    void* getResource() override { return m_resource; }
};

}
