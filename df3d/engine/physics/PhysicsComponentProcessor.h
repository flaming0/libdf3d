#pragma once

#include <df3d/game/Entity.h>
#include <df3d/game/EntityComponentProcessor.h>

class btRigidBody;
class btDynamicsWorld;
class btMotionState;

namespace df3d {

class RenderQueue;
class MeshData;
class World;
struct PhysicsComponentCreationParams;

class DF3D_DLL PhysicsComponentProcessor : public EntityComponentProcessor
{
    friend class World;

    struct Impl;
    unique_ptr<Impl> m_pimpl;

    void update() override;
    void draw(RenderQueue *ops) override;
    void cleanStep(const std::list<Entity> &deleted) override;

public:
    PhysicsComponentProcessor(World *w);
    ~PhysicsComponentProcessor();

    btRigidBody* getBody(Entity e);
    glm::vec3 getCenterOfMass(Entity e);

    void teleportPosition(Entity e, const glm::vec3 &pos);
    void teleportOrientation(Entity e, const glm::quat &orient);

    void add(Entity e, const PhysicsComponentCreationParams &params, shared_ptr<MeshData> mesh);
    // NOTE: body should not be added to the Physics World as it will be added via this processor.
    void add(Entity e, btRigidBody *body, short group = -1, short mask = -1);
    void remove(Entity e);
    bool has(Entity e);

    btDynamicsWorld* getPhysicsWorld();
    btMotionState* createMotionState(Entity e);
};

}