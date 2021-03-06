#include "EntityLoader.h"

#include <df3d/game/World.h>
#include <df3d/engine/3d/SceneGraphComponentProcessor.h>
#include <df3d/engine/EngineController.h>
#include <df3d/engine/resources/ResourceManager.h>
#include <df3d/engine/resources/EntityResource.h>
#include <df3d/lib/JsonUtils.h>
#include <df3d/lib/Utils.h>
#include "MeshComponentLoader.h"
#include "PhysicsComponentLoader.h"
#include "Sprite2DComponentLoader.h"
#include "SceneGraphComponentLoader.h"
#include "VfxComponentLoader.h"
#include "TagComponentLoader.h"

namespace df3d { namespace game_impl {

EntityLoader::EntityLoader()
{
    registerEntityComponentLoader(Id("scenegraph"), make_unique<SceneGraphComponentLoader>());
    registerEntityComponentLoader(Id("mesh"), make_unique<MeshComponentLoader>());
    registerEntityComponentLoader(Id("vfx"), make_unique<VfxComponentLoader>());
    registerEntityComponentLoader(Id("physics"), make_unique<PhysicsComponentLoader>());
    registerEntityComponentLoader(Id("sprite_2d"), make_unique<Sprite2DComponentLoader>());
    registerEntityComponentLoader(Id("tags"), make_unique<TagComponentLoader>());
}

EntityLoader::~EntityLoader()
{

}

Entity EntityLoader::createEntityFromFile(const char *resourceFile, World &w)
{
    auto resource = svc().resourceManager().getResource<EntityResource>(Id(resourceFile));
    if (!resource)
        return {};

    return createEntityFromJson(resource->root, w);
}

Entity EntityLoader::createEntityFromJson(const Json::Value &root, World &w)
{
    if (root.isNull())
    {
        DFLOG_WARN("Failed to init an entity from Json node");
        return {};
    }

    Entity res = w.spawn();

    if (root.isMember("components"))
    {
        for (const auto &it : root["components"])
        {
            const auto &dataJson = it["data"];
            if (dataJson.isNull())
            {
                DFLOG_WARN("Failed to init a component. Empty \"data\" field");
                w.destroy(res);
                return{};
            }

            auto componentType = it["type"].asCString();
            auto foundLoader = m_loaders.find(Id(componentType));
            if (foundLoader != m_loaders.end())
                foundLoader->second->loadComponent(dataJson, res, w);
            else
                DFLOG_WARN("Failed to parse entity description, unknown component %s", componentType);
        }
    }

    if (root.isMember("children"))
    {
        for (const auto &it : root["children"])
            w.sceneGraph().attachChild(res, createEntityFromJson(it, w));
    }

    return res;
}

void EntityLoader::registerEntityComponentLoader(Id name, unique_ptr<EntityComponentLoader> loader)
{
    DF3D_ASSERT(!utils::contains_key(m_loaders, name));
    m_loaders.insert(std::make_pair(name, std::move(loader)));
}

} }
