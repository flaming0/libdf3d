#include "BulletInterface.h"

#include <df3d/engine/EngineController.h>
#include <df3d/engine/render/RenderManager.h>
#include <df3d/engine/render/RenderPass.h>
#include <df3d/engine/render/IRenderBackend.h>
#include <df3d/engine/render/RenderQueue.h>
#include <df3d/engine/resources/ResourceManager.h>
#include <df3d/engine/resources/ResourceFactory.h>

namespace df3d { namespace physics_impl {

static unique_ptr<RenderPass> CreateDebugDrawPass()
{
    auto pass = make_unique<RenderPass>("bullet_debug_draw_pass");
    pass->setFaceCullMode(FaceCullMode::NONE);
    pass->setBlendMode(BlendingMode::ALPHA);
    pass->getPassParam("material_diffuse")->setValue(glm::vec4(1.0f, 1.0f, 1.0f, 0.7f));
    // FIXME: force to use default white texture because using colored shader.
    pass->getPassParam("diffuseMap")->setValue(nullptr);

    auto program = svc().resourceManager().getFactory().createColoredGpuProgram();
    pass->setGpuProgram(program);

    return pass;
}

BulletDebugDraw::BulletDebugDraw()
    : m_vertexData(vertex_formats::p3_tx2_c4)
{

}

BulletDebugDraw::~BulletDebugDraw()
{
    clean();
}

void BulletDebugDraw::drawLine(const btVector3 &from, const btVector3 &to, const btVector3 &color)
{
    if (m_vertexData.getVerticesCount() == 0)
        m_vertexData.allocVertices(300000);

    // FIXME: map directly to GPU.
    auto v1 = m_vertexData.getVertex(m_currentVertex++);
    v1.setColor({ color.x(), color.y(), color.z(), 1.0f });
    v1.setPosition({ from.x(), from.y(), from.z() });

    auto v2 = m_vertexData.getVertex(m_currentVertex++);
    v2.setColor({ color.x(), color.y(), color.z(), 1.0f });
    v2.setPosition({ to.x(), to.y(), to.z() });
}

void BulletDebugDraw::drawContactPoint(const btVector3 &PointOnB, const btVector3 &normalOnB, btScalar distance, int lifeTime, const btVector3 &color)
{

}

void BulletDebugDraw::reportErrorWarning(const char *warningString)
{
    DFLOG_WARN("Bullet physics warning: %s", warningString);
}

void BulletDebugDraw::draw3dText(const btVector3 &location, const char *textString)
{

}

void BulletDebugDraw::setDebugMode(int debugMode)
{

}

int BulletDebugDraw::getDebugMode() const
{
    return btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawWireframe;// | btIDebugDraw::DBG_DrawNormals | btIDebugDraw::DBG_DrawConstraints;
}

void BulletDebugDraw::clean()
{
    if (m_vertexBuffer.valid())
    {
        svc().renderManager().getBackend().destroyVertexBuffer(m_vertexBuffer);
        m_vertexBuffer.invalidate();
    }
    m_currentVertex = 0;
}

void BulletDebugDraw::flushRenderOperations(RenderQueue *ops)
{
    if (m_vertexData.getVerticesCount() == 0)
        return;

    if (!m_pass)
        m_pass = CreateDebugDrawPass();

    DF3D_ASSERT_MESS(!m_vertexBuffer.valid(), "bullet debug draw: invalid vertex buffer");

    m_vertexBuffer = svc().renderManager().getBackend().createVertexBuffer(m_vertexData, GpuBufferUsageType::STREAM);

    RenderOperation op;
    op.passProps = m_pass.get();
    op.vertexBuffer = m_vertexBuffer;
    op.numberOfElements = m_vertexData.getVerticesCount();
    op.topology = Topology::LINES;

    ops->debugDrawOperations.push_back(op);
}

} }