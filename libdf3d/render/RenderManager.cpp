#include "RenderManager.h"

#include <libdf3d/base/EngineController.h>
#include <libdf3d/base/FrameStats.h>
#include <libdf3d/base/DebugConsole.h>
#include <libdf3d/resources/ResourceManager.h>
#include <libdf3d/resources/ResourceFactory.h>
#include <libdf3d/3d/Camera.h>
#include <libdf3d/game/World.h>
#include <libdf3d/gui/GuiManager.h>
#include <libdf3d/particlesys/ParticleSystemComponentProcessor.h>
#include "RenderOperation.h"
#include "MeshData.h"
#include "Texture.h"
#include "Material.h"
#include "Technique.h"
#include "RenderPass.h"
#include "GpuProgram.h"
#include "Viewport.h"
#include "RenderQueue.h"
#include "IRenderBackend.h"
#include "GpuProgramSharedState.h"

namespace df3d {

void RenderManager::loadEmbedResources()
{
    m_renderBackend->createEmbedResources();

    m_ambientPassProps.setGpuProgram(svc().resourceManager().getFactory().createAmbientPassProgram());
    m_ambientMtlParam = m_ambientPassProps.getPassParamHandle("material_ambient");
}

void RenderManager::onFrameBegin()
{
    m_blendModeOverriden = false;
    m_depthTestOverriden = false;
    m_depthWriteOverriden = false;

    m_sharedState->clear();
    m_renderBackend->frameBegin();

    m_renderBackend->clearColorBuffer();
    m_renderBackend->clearDepthBuffer();
    m_renderBackend->clearStencilBuffer();
    m_renderBackend->enableDepthTest(true);
    m_renderBackend->enableDepthWrite(true);
    m_renderBackend->enableScissorTest(false);
    m_renderBackend->setBlendingMode(BlendingMode::NONE);
}

void RenderManager::onFrameEnd()
{
    m_renderBackend->frameEnd();
}

void RenderManager::doRenderWorld(World &world)
{
    // TODO_ecs: what if render queue becomes big???
    m_renderQueue->clear();

    world.collectRenderOperations(m_renderQueue.get());

    m_renderBackend->setViewport(m_viewport.x(), m_viewport.y(), m_viewport.width(), m_viewport.height());
    m_sharedState->setViewPort(m_viewport);
    m_sharedState->setProjectionMatrix(world.getCamera()->getProjectionMatrix());

    m_renderBackend->clearColorBuffer();
    m_renderBackend->clearDepthBuffer();

    m_sharedState->setAmbientColor(world.getRenderingParams().getAmbientLight());
    m_sharedState->setFog(world.getRenderingParams().getFogDensity(), world.getRenderingParams().getFogColor());

    m_sharedState->setViewMatrix(world.getCamera()->getViewMatrix());

    m_renderQueue->sort();

    // Ambient pass.
    m_blendModeOverriden = true;
    m_depthTestOverriden = true;
    m_depthWriteOverriden = true;
    m_renderBackend->setBlendingMode(BlendingMode::NONE);
    m_renderBackend->enableDepthTest(true);
    m_renderBackend->enableDepthWrite(true);

    for (const auto &op : m_renderQueue->litOpaqueOperations)
    {
        m_sharedState->setWorldMatrix(op.worldTransform);

        m_ambientPassProps.getPassParam(m_ambientMtlParam)->setValue(op.passProps->getAmbientColor());

        bindPass(&m_ambientPassProps);

        m_renderBackend->bindVertexBuffer(op.vertexBuffer);
        if (op.indexBuffer.valid())
            m_renderBackend->bindIndexBuffer(op.indexBuffer);

        m_renderBackend->draw(op.type, op.numberOfElements);
    }

    // Opaque pass with lights on.
    m_renderBackend->setBlendingMode(BlendingMode::ADD);
    m_renderBackend->enableDepthWrite(false);

    for (const auto &op : m_renderQueue->litOpaqueOperations)
    {
        m_sharedState->setWorldMatrix(op.worldTransform);
        // TODO: bind pass only once

        for (const auto &light : m_renderQueue->lights)
        {
            m_sharedState->setLight(*light);

            // TODO_render: update ONLY light uniforms.
            bindPass(op.passProps);

            m_renderBackend->bindVertexBuffer(op.vertexBuffer);
            if (op.indexBuffer.valid())
                m_renderBackend->bindIndexBuffer(op.indexBuffer);
            m_renderBackend->draw(op.type, op.numberOfElements);
        }
    }

    // Rendering others as usual.
    m_blendModeOverriden = false;
    m_depthTestOverriden = false;
    m_depthWriteOverriden = false;

    // Opaque pass without lights.
    for (const auto &op : m_renderQueue->notLitOpaqueOperations)
        drawRenderOperation(op);

    // VFX pass.
    world.vfx().draw();

    // Transparent pass.
    for (const auto &op : m_renderQueue->transparentOperations)
        drawRenderOperation(op);

    // Debug draw pass.
    if (auto console = svc().debugConsole())
    {
        if (console->getCVars().get<bool>(CVAR_DEBUG_DRAW))
        {
            for (const auto &op : m_renderQueue->debugDrawOperations)
                drawRenderOperation(op);
        }
    }

    m_sharedState->setProjectionMatrix(glm::ortho(0.0f, (float)m_viewport.width(), (float)m_viewport.height(), 0.0f));
    m_sharedState->setViewMatrix(glm::mat4(1.0f));

    // 2D ops pass.
    for (const auto &op : m_renderQueue->sprite2DOperations)
        drawRenderOperation(op);

    // Draw GUI.
    m_sharedState->setProjectionMatrix(glm::ortho(0.0f, (float)m_viewport.width(), (float)m_viewport.height(), 0.0f));
    m_sharedState->setViewMatrix(glm::mat4(1.0f));

    svc().guiManager().getContext()->Render();
}

void RenderManager::bindPass(RenderPass *pass)
{
    if (!pass)
        return;

    auto gpuProgram = pass->getGpuProgram();
    if (!gpuProgram)
    {
        glog << "Failed to bind pass. No GPU program" << logwarn;
        return;
    }

    m_renderBackend->bindGpuProgram(gpuProgram->getDescriptor());

    // Update pass uniforms.
    {
        // Shared uniforms.
        m_sharedState->updateSharedUniforms(*gpuProgram);

        int textureUnit = 0;
        // Custom uniforms.
        auto &passParams = pass->getPassParams();
        for (auto &passParam : passParams)
        {
            if (passParam.getTexture())
            {
                passParam.setValue(textureUnit);

                m_renderBackend->bindTexture(passParam.getTexture()->getDescriptor(), textureUnit);

                textureUnit++;
            }

            passParam.updateToProgram(*m_renderBackend, *gpuProgram);
        }
    }

    if (!m_depthTestOverriden)
        m_renderBackend->enableDepthTest(pass->isDepthTestEnabled());
    if (!m_depthWriteOverriden)
        m_renderBackend->enableDepthWrite(pass->isDepthWriteEnabled());
    if (!m_blendModeOverriden)
        m_renderBackend->setBlendingMode(pass->getBlendingMode());
    m_renderBackend->setCullFaceMode(pass->getFaceCullMode());
}

RenderManager::RenderManager(EngineInitParams params)
    : m_renderQueue(make_unique<RenderQueue>()),
    m_initParams(params)
{
    m_viewport = Viewport(0, 0, params.windowWidth, params.windowHeight);

    m_renderBackend = IRenderBackend::create();
    m_sharedState = make_unique<GpuProgramSharedState>();
}

RenderManager::~RenderManager()
{

}

void RenderManager::drawWorld(World &world)
{
    onFrameBegin();

    doRenderWorld(world);

    onFrameEnd();
}

void RenderManager::drawRenderOperation(const RenderOperation &op)
{
    if (op.numberOfElements == 0)
    {
        assert(false);
        return;
    }

    m_sharedState->setWorldMatrix(op.worldTransform);
    m_renderBackend->bindVertexBuffer(op.vertexBuffer);
    if (op.indexBuffer.valid())
        m_renderBackend->bindIndexBuffer(op.indexBuffer);

    bindPass(op.passProps);

    m_renderBackend->draw(op.type, op.numberOfElements);
}

const Viewport& RenderManager::getViewport() const
{
    return m_viewport;
}

const RenderingCapabilities& RenderManager::getRenderingCapabilities() const
{
    return m_initParams.renderingCaps;
}

IRenderBackend& RenderManager::getBackend()
{
    return *m_renderBackend;
}

}
