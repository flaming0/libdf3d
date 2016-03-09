#pragma once

#include <btBulletCollisionCommon.h>
#include <btBulletDynamicsCommon.h>

#include <libdf3d/render/Vertex.h>

namespace df3d { 

class RenderPass;
class RenderQueue;

namespace physics_impl {

class BulletDebugDraw : public btIDebugDraw
{
    shared_ptr<RenderPass> m_pass;
    shared_ptr<VertexBuffer> m_vertexBuffer;

    VertexData m_vertexData;

public:
    BulletDebugDraw();
    ~BulletDebugDraw();

    void drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color) override;
    void drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color) override;
    void reportErrorWarning(const char *warningString) override;
    void draw3dText(const btVector3 &location, const char *textString) override;
    void setDebugMode(int debugMode) override;
    int getDebugMode() const override;

    void clean();
    void flushRenderOperations(RenderQueue *ops);
};

} }
