#pragma once

#include <scene/Entity.h>

namespace df3d {

// TODO_ecs : add test suits.
class DF3D_DLL EntityManager : utils::NonCopyable
{
    std::unordered_set<Entity::IdType> m_entities;
    Entity m_next;

public:
    EntityManager();
    ~EntityManager();

    Entity create();
    void destroy(Entity e);
    bool alive(Entity e);
};

}
