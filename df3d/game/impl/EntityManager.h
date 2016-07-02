#pragma once

#include <df3d/game/Entity.h>
#include <df3d/lib/Handles.h>

namespace df3d { namespace game_impl {

class DF3D_DLL EntityManager : NonCopyable
{
    std::unordered_set<Entity> m_entities;

    HandleBag<Entity> m_bag;

public:
    EntityManager();
    ~EntityManager();

    Entity create();
    void destroy(Entity e);
    bool alive(Entity e) const;
    size_t size() const;

    void cleanStep();
};

} }
