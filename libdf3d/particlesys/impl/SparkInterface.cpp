#include "SparkInterface.h"

#include <libdf3d/render/RenderPass.h>
#include <libdf3d/base/EngineController.h>
#include <libdf3d/3d/Camera.h>
#include <libdf3d/game/World.h>
#include <libdf3d/resources/ResourceManager.h>
#include <libdf3d/resources/ResourceFactory.h>
#include <libdf3d/render/VertexIndexBuffer.h>
#include <libdf3d/render/GpuProgram.h>
#include <libdf3d/render/RenderOperation.h>
#include <libdf3d/render/RenderQueue.h>

namespace df3d { namespace particlesys_impl {

static const size_t QUAD_VERTICES_PER_PARTICLE = 4;
static const size_t QUAD_INDICES_PER_PARTICLE = 6;

class MyRenderBuffer : public SPK::RenderBuffer
{
public:
    ParticleSystemBuffers_Quad *m_buffers = nullptr;

    MyRenderBuffer(size_t nbParticles, ParticleSystemBuffers_Quad *buffers)
        : m_buffers(buffers)
    {
        if (nbParticles > m_buffers->getParticlesAllocated())
            m_buffers->realloc(nbParticles);
    }
};

ParticleSystemBuffers_Quad::ParticleSystemBuffers_Quad()
{
    m_vb = make_unique<VertexBuffer>(VERTEX_FORMAT);
    m_ib = make_unique<IndexBuffer>();
}

ParticleSystemBuffers_Quad::~ParticleSystemBuffers_Quad()
{

}

void ParticleSystemBuffers_Quad::realloc(size_t nbParticles)
{
    nbParticles = std::max(nbParticles, INITIAL_CAPACITY);

    m_vertexData.reset(new VertexData(VERTEX_FORMAT));

    // Allocate GPU storage.
    m_vb->alloc(nbParticles * QUAD_VERTICES_PER_PARTICLE, nullptr, GpuBufferUsageType::DYNAMIC);
    m_ib->alloc(nbParticles * QUAD_INDICES_PER_PARTICLE, nullptr, GpuBufferUsageType::STATIC);

    // Allocate main memory storage copy (no glMapBuffer on ES2.0)
    m_vertexData->allocVertices(nbParticles * QUAD_VERTICES_PER_PARTICLE);
    m_indexData.resize(nbParticles * QUAD_INDICES_PER_PARTICLE);

    positionAtStart();

    // Initialize the index array.
    for (size_t i = 0; i < nbParticles; ++i)
    {
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 0);
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 1);
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 2);
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 0);
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 2);
        setNextIndex(QUAD_VERTICES_PER_PARTICLE * i + 3);
    }

    // Initialize GPU storage of index array.
    m_ib->update(nbParticles * QUAD_INDICES_PER_PARTICLE, m_indexData.data());
    // Clear main storage indices copy.
    m_indexData.clear();

    // Initialize the texture array (CCW order).
    for (size_t i = 0; i < nbParticles; ++i)
    {
        // FIXME: inverted UV's Y because of OpenGL.
        setNextTexCoords(1.0f, 0.0f);
        setNextTexCoords(0.0f, 0.0f);
        setNextTexCoords(0.0f, 1.0f);
        setNextTexCoords(1.0f, 1.0f);
    }

    m_particlesAllocated = nbParticles;
}

void ParticleSystemBuffers_Quad::positionAtStart()
{
    // Repositions all the buffer pointers at the start.
    m_currentIndexIndex = 0;
    m_currentVertexIndex = 0;
    m_currentColorIndex = 0;
    m_currentTexCoordIndex = 0;
}

void ParticleSystemBuffers_Quad::setNextIndex(int index)
{
    m_indexData[m_currentIndexIndex++] = index;
}

void ParticleSystemBuffers_Quad::setNextVertex(const SPK::Vector3D &vertex)
{
    auto vert = m_vertexData->getVertex(m_currentVertexIndex++);
    vert.setPosition({ vertex.x, vertex.y, vertex.z });
}

void ParticleSystemBuffers_Quad::setNextColor(const SPK::Color &color)
{
    auto vert = m_vertexData->getVertex(m_currentColorIndex++);
    vert.setColor({ color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f });
}

void ParticleSystemBuffers_Quad::setNextTexCoords(float u, float v)
{
    auto vert = m_vertexData->getVertex(m_currentTexCoordIndex++);
    vert.setTx({ u, v });
}

void ParticleSystemBuffers_Quad::addToRenderQueue(size_t nbOfParticles, RenderPass *passProps, const glm::mat4 &m, RenderQueue *rqueue)
{
    assert(nbOfParticles <= m_particlesAllocated);

    // Refill GPU with new data (only vertices have been changed).
    m_vb->update(nbOfParticles * QUAD_VERTICES_PER_PARTICLE, m_vertexData->getRawData());

    m_vb->setVerticesUsed(nbOfParticles * QUAD_VERTICES_PER_PARTICLE);
    m_ib->setIndicesUsed(nbOfParticles * QUAD_INDICES_PER_PARTICLE);

    RenderOperation op;
    op.type = RenderOperation::Type::TRIANGLES;
    op.indexData = m_ib.get();
    op.vertexData = m_vb.get();
    op.passProps = passProps;
    op.worldTransform = m;

    if (op.passProps->isTransparent())
        rqueue->transparentOperations.push_back(op);
    else
        rqueue->notLitOpaqueOperations.push_back(op);
}

ParticleSystemRenderer::ParticleSystemRenderer(bool NEEDS_DATASET)
    : SPK::Renderer(NEEDS_DATASET),
    m_pass(make_shared<RenderPass>())
{
    m_pass->setFaceCullMode(RenderPass::FaceCullMode::BACK);
    m_pass->setFrontFaceWinding(RenderPass::WindingOrder::CCW);
    m_pass->setDiffuseColor(1.0f, 1.0f, 1.0f);
    m_pass->setSampler("diffuseMap", nullptr);          // FIXME: force to use default white texture (as using colored program)

    m_pass->setGpuProgram(svc().resourceManager().getFactory().createColoredGpuProgram());
}

ParticleSystemRenderer::~ParticleSystemRenderer()
{

}

void ParticleSystemRenderer::setBlendMode(SPK::BlendMode blendMode)
{
    switch (blendMode)
    {
    case SPK::BLEND_MODE_NONE:
        m_pass->setBlendMode(RenderPass::BlendingMode::NONE);
        break;
    case SPK::BLEND_MODE_ADD:
        m_pass->setBlendMode(RenderPass::BlendingMode::ADDALPHA);
        break;
    case SPK::BLEND_MODE_ALPHA:
        m_pass->setBlendMode(RenderPass::BlendingMode::ALPHA);
        break;
    default:
        break;
    }
}

void ParticleSystemRenderer::setDiffuseMap(shared_ptr<Texture> texture)
{
    m_pass->setSampler("diffuseMap", texture);
}

void ParticleSystemRenderer::enableFaceCulling(bool enable)
{
    if (enable)
    {
        m_pass->setFaceCullMode(RenderPass::FaceCullMode::BACK);
        m_pass->setFrontFaceWinding(RenderPass::WindingOrder::CCW);
    }
    else
    {
        m_pass->setFaceCullMode(RenderPass::FaceCullMode::NONE);
    }
}

void QuadParticleSystemRenderer::render2D(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    scaleQuadVectors(particle, scaleX, scaleY);
    fillBufferColorAndVertex(particle, renderBuffer);
}

void QuadParticleSystemRenderer::render2DRot(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    rotateAndScaleQuadVectors(particle, scaleX, scaleY);
    fillBufferColorAndVertex(particle, renderBuffer);
}

void QuadParticleSystemRenderer::render2DAtlas(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    scaleQuadVectors(particle, scaleX, scaleY);
    fillBufferColorAndVertex(particle, renderBuffer);
    fillBufferTexture2DCoordsAtlas(particle, renderBuffer);
}

void QuadParticleSystemRenderer::render2DAtlasRot(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    rotateAndScaleQuadVectors(particle, scaleX, scaleY);
    fillBufferColorAndVertex(particle, renderBuffer);
    fillBufferTexture2DCoordsAtlas(particle, renderBuffer);
}

void QuadParticleSystemRenderer::fillBufferColorAndVertex(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    // Quads are drawn in a counter clockwise order.
    renderBuffer.m_buffers->setNextVertex(particle.position() + quadSide() + quadUp());    // top right vertex
    renderBuffer.m_buffers->setNextVertex(particle.position() - quadSide() + quadUp());    // top left vertex
    renderBuffer.m_buffers->setNextVertex(particle.position() - quadSide() - quadUp());    // bottom left vertex
    renderBuffer.m_buffers->setNextVertex(particle.position() + quadSide() - quadUp());    // bottom right vertex

    const auto &color = particle.getColor();
    renderBuffer.m_buffers->setNextColor(color);
    renderBuffer.m_buffers->setNextColor(color);
    renderBuffer.m_buffers->setNextColor(color);
    renderBuffer.m_buffers->setNextColor(color);
}

void QuadParticleSystemRenderer::fillBufferTexture2DCoordsAtlas(const SPK::Particle &particle, MyRenderBuffer &renderBuffer) const
{
    computeAtlasCoordinates(particle);

    // FIXME: inverted UV's Y because of OpenGL.
    renderBuffer.m_buffers->setNextTexCoords(textureAtlasU1(), textureAtlasV0());
    renderBuffer.m_buffers->setNextTexCoords(textureAtlasU0(), textureAtlasV0());
    renderBuffer.m_buffers->setNextTexCoords(textureAtlasU0(), textureAtlasV1());
    renderBuffer.m_buffers->setNextTexCoords(textureAtlasU1(), textureAtlasV1());
}

QuadParticleSystemRenderer::QuadParticleSystemRenderer(float scaleX, float scaleY)
    : ParticleSystemRenderer(false),
    SPK::QuadRenderBehavior(scaleX, scaleY)
{

}

QuadParticleSystemRenderer::~QuadParticleSystemRenderer()
{

}

SPK::RenderBuffer* QuadParticleSystemRenderer::attachRenderBuffer(const SPK::Group &group) const
{
    return SPK_NEW(MyRenderBuffer, group.getCapacity(), m_quadBuffers);
}

void QuadParticleSystemRenderer::render(const SPK::Group &group, const SPK::DataSet *dataSet, SPK::RenderBuffer *renderBuffer) const
{
    if (!isActive())
        return;

    auto &buffer = static_cast<MyRenderBuffer&>(*renderBuffer);
    buffer.m_buffers->positionAtStart(); // Repositions all the buffers at the start.

    m_pass->enableDepthWrite(isRenderingOptionEnabled(SPK::RENDERING_OPTION_DEPTH_WRITE));

    switch (texturingMode)
    {
    case SPK::TEXTURE_MODE_NONE:
        break;
    case SPK::TEXTURE_MODE_2D:
        if (!group.isEnabled(SPK::PARAM_TEXTURE_INDEX))
        {
            // FIXME: inverted UV's Y because of OpenGL.
            for (size_t i = 0; i < group.getCapacity(); ++i)
            {
                buffer.m_buffers->setNextTexCoords(1.0f, 0.0f);
                buffer.m_buffers->setNextTexCoords(0.0f, 0.0f);
                buffer.m_buffers->setNextTexCoords(0.0f, 1.0f);
                buffer.m_buffers->setNextTexCoords(1.0f, 1.0f);
            }
        }
        break;
    case SPK::TEXTURE_MODE_3D:
        glog << "3D texture for particle systems is not implemented" << logwarn;
        return;
    default:
        break;
    }

    if (group.isEnabled(SPK::PARAM_TEXTURE_INDEX))
    {
        if (group.isEnabled(SPK::PARAM_ANGLE))
            m_renderParticle = &QuadParticleSystemRenderer::render2DAtlasRot;
        else
            m_renderParticle = &QuadParticleSystemRenderer::render2DAtlas;
    }
    else
    {
        if (group.isEnabled(SPK::PARAM_ANGLE))
            m_renderParticle = &QuadParticleSystemRenderer::render2DRot;
        else
            m_renderParticle = &QuadParticleSystemRenderer::render2D;
    }

    auto camMatr = svc().world().getCamera()->getViewMatrix() * *m_currentTransformation;
    camMatr = glm::inverse(camMatr);

    bool globalOrientation = precomputeOrientation3D(group,
    { -camMatr[2][0], -camMatr[2][1], -camMatr[2][2] },
    { camMatr[1][0], camMatr[1][1], camMatr[1][2] },
    { camMatr[3][0], camMatr[3][1], camMatr[3][2] });

    if (globalOrientation)
    {
        computeGlobalOrientation3D(group);

        for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
            (this->*m_renderParticle)(*particleIt, buffer);
    }
    else
    {
        for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
        {
            computeSingleOrientation3D(*particleIt);
            (this->*m_renderParticle)(*particleIt, buffer);
        }
    }

    buffer.m_buffers->addToRenderQueue(group.getNbParticles(), m_pass.get(), *m_currentTransformation, m_currentRenderQueue);
}

void QuadParticleSystemRenderer::computeAABB(SPK::Vector3D &AABBMin, SPK::Vector3D &AABBMax, const SPK::Group &group, const SPK::DataSet *dataSet) const
{
    float diagonal = group.getGraphicalRadius() * std::sqrt(scaleX * scaleX + scaleY * scaleY);
    SPK::Vector3D diagV(diagonal, diagonal, diagonal);

    if (group.isEnabled(SPK::PARAM_SCALE))
    {
        for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
        {
            SPK::Vector3D scaledDiagV = diagV * particleIt->getParamNC(SPK::PARAM_SCALE);
            AABBMin.setMin(particleIt->position() - scaledDiagV);
            AABBMax.setMax(particleIt->position() + scaledDiagV);
        }
    }
    else
    {
        for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
        {
            AABBMin.setMin(particleIt->position());
            AABBMax.setMax(particleIt->position());
        }
        AABBMin -= diagV;
        AABBMax += diagV;
    }
}

/*
LineParticleSystemRenderer::LineParticleSystemRenderer(float length, float width)
    : ParticleSystemRenderer(false),
    SPK::LineRenderBehavior(length, width)
{

}

LineParticleSystemRenderer::~LineParticleSystemRenderer()
{

}

SPK::RenderBuffer* LineParticleSystemRenderer::attachRenderBuffer(const SPK::Group &group) const
{
    size_t totalParticles = group.getCapacity();
    auto buffer = SPK_NEW(MyRenderBuffer, totalParticles, LINE_VERTICES_PER_PARTICLE, LINE_INDICES_PER_PARTICLE);
    buffer->positionAtStart();

    // Initialize the index array.
    for (size_t i = 0; i < totalParticles * LINE_INDICES_PER_PARTICLE; ++i)
        buffer->setNextIndex(i);

    buffer->m_ib->update(totalParticles * LINE_INDICES_PER_PARTICLE, buffer->m_indexData.data());
    buffer->m_indexData.clear();

    return buffer;
}

void LineParticleSystemRenderer::render(const SPK::Group &group, const SPK::DataSet *dataSet, SPK::RenderBuffer *renderBuffer) const
{
    if (!isActive())
        return;

    auto &buffer = static_cast<MyRenderBuffer&>(*renderBuffer);
    buffer.positionAtStart(); // Repositions all the buffers at the start.

    m_pass->enableDepthWrite(isRenderingOptionEnabled(SPK::RENDERING_OPTION_DEPTH_WRITE));

    for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
    {
        buffer.setNextVertex(particleIt->position());
        buffer.setNextVertex(particleIt->position() + particleIt->velocity() * length);

        const auto &color = particleIt->getColor();
        buffer.setNextColor(color);
        buffer.setNextColor(color);
    }

    addToRenderQueue(buffer, group.getNbParticles(), LINE_VERTICES_PER_PARTICLE, LINE_INDICES_PER_PARTICLE, RenderOperation::Type::LINES);
}

void LineParticleSystemRenderer::computeAABB(SPK::Vector3D &AABBMin, SPK::Vector3D &AABBMax, const SPK::Group &group, const SPK::DataSet *dataSet) const
{
    for (SPK::ConstGroupIterator particleIt(group); !particleIt.end(); ++particleIt)
    {
        auto v = particleIt->position() + particleIt->velocity() * length;
        AABBMin.setMin(particleIt->position());
        AABBMin.setMin(v);
        AABBMax.setMax(particleIt->position());
        AABBMax.setMax(v);
    }
}
.*/

} }
