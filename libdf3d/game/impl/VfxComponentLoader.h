#pragma once

#include <game/EntityComponentLoader.h>
#include <particlesys/ParticleSystemComponentProcessor.h>

namespace df3d {

class VfxComponentLoader : public EntityComponentLoader
{
public:
    void loadComponent(const Json::Value &root, Entity e, World &w) const override
    {
        w.vfx().add(e, root["path"].asString());
    }
};

}
