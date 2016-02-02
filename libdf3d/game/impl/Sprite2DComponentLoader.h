#pragma once

#include <libdf3d/2d/Sprite2DComponentProcessor.h>
#include <libdf3d/game/EntityComponentLoader.h>

namespace df3d {

class Sprite2DComponentLoader : public EntityComponentLoader
{
public:
    void loadComponent(const Json::Value &root, Entity e, World &w) const override
    {
        std::string path;

        root["path"] >> path;

        w.sprite2d().add(e, path);
    }
};

}
