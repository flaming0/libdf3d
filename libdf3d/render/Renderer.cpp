#include "df3d_pch.h"
#include "Renderer.h"

#include <base/SystemsMacro.h>
#include <components/LightComponent.h>
#include "OpenGLCommon.h"
#include "GpuProgramState.h"
#include "RenderPass.h"
#include "Shader.h"
#include "GpuProgram.h"
#include "VertexIndexBuffer.h"
#include "GpuProgramUniform.h"
#include "Texture2D.h"
#include "Viewport.h"

namespace df3d { namespace render {

void Renderer::createWhiteTexture()
{
    auto w = 32;
    auto h = 32;
    auto pf = PixelFormat::RGBA;

    auto data = new unsigned char[w * h * 4];
    memset(data, 255, w * h * 4);

    m_whiteTexture = g_resourceManager->createEmptyTexture("__white_texture");
    m_whiteTexture->setFilteringMode(TextureFiltering::NEAREST);
    m_whiteTexture->setMipmapped(false);
    m_whiteTexture->setWrapMode(Texture::WrapMode::WRAP);
    m_whiteTexture->setWithData(w, h, pf, data);
    m_whiteTexture->setResident();

    delete [] data;
}

void Renderer::setBlendMode(RenderPass::BlendingMode bm)
{
    if (m_blendModeOverriden)
        return;

    switch (bm)
    {
    case RenderPass::BlendingMode::NONE:
        glDisable(GL_BLEND);
        break;
    case RenderPass::BlendingMode::ADDALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        break;
    case RenderPass::BlendingMode::ALPHA:
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        break;
    case RenderPass::BlendingMode::ADD:
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        break;
    default:
        break;
    }
}

void Renderer::setFrontFace(RenderPass::WindingOrder wo)
{
    if (m_frontFaceOverriden)
        return;

    switch (wo)
    {
    case RenderPass::WindingOrder::CW:
        glFrontFace(GL_CW);
        break;
    case RenderPass::WindingOrder::CCW:
        glFrontFace(GL_CCW);
    default:
        break;
    }
}

void Renderer::setCullFace(RenderPass::FaceCullMode cm)
{
    if (m_cullFaceOverriden)
        return;

    switch (cm)
    {
    case RenderPass::FaceCullMode::NONE:
        glDisable(GL_CULL_FACE);
        break;
    case RenderPass::FaceCullMode::FRONT:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
        break;
    case RenderPass::FaceCullMode::BACK:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        break;
    case RenderPass::FaceCullMode::FRONT_AND_BACK:
        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT_AND_BACK);
        break;
    default:
        break;
    }
}

void Renderer::setPolygonDrawMode(RenderPass::PolygonMode pm)
{
    if (m_polygonDrawModeOverriden)
        return;

    // Doesn't work in OpenGL ES 2.x
#if defined(DF3D_WINDOWS) || defined(DF3D_LINUX)
    switch (pm)
    {
    case RenderPass::PolygonMode::FILL:
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
    case RenderPass::PolygonMode::WIRE:
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        break;
    default:
        break;
    }
#endif
}

void Renderer::updateProgramUniformValues(GpuProgram *program, RenderPass *pass)
{
    // Update shader uniforms.
    size_t uniCount = program->getSharedUniformsCount();

    for (size_t i = 0; i < uniCount; i++)
        m_programState->updateSharedUniform(program->getSharedUniform(i));

    // Update custom uniforms.
    auto &passParams = pass->getPassParams();
    uniCount = passParams.size();

    for (size_t i = 0; i < uniCount; i++)
        passParams[i].updateTo(program);
}

void Renderer::updateTextureSamplers()
{
    // Bind textures to samplers.
    auto &samplers = m_programState->m_currentPass->getSamplers();
    size_t textureUnit = 0;
    for (size_t i = 0; i < samplers.size(); i++)
    {
        shared_ptr<Texture> texture = samplers[i].texture;
        if (!texture)
            texture = m_whiteTexture;

        auto bound = texture->bind(textureUnit);
        if (!bound) 
        {
            texture = m_whiteTexture;
            texture->bind(textureUnit);
        }

        // FIXME:
        auto location = glGetUniformLocation(m_programState->m_currentShader->descriptor(), samplers[i].name.c_str());
        if (location != -1)
            glUniform1i(location, textureUnit++);
    }
}

Renderer::Renderer()
    : m_programState(new GpuProgramState())
{
    const char *ver = (const char *)glGetString(GL_VERSION);
    base::glog << "OpenGL version" << ver << base::logmess;

    const char *card = (const char *)glGetString(GL_RENDERER);
    const char *vendor = (const char *)glGetString(GL_VENDOR);
    base::glog << "Using" << card << vendor << base::logmess;

    const char *shaderVer = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    base::glog << "Shaders version" << shaderVer << base::logmess;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    createWhiteTexture();

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_maxTextureSize);
    // TODO:
    // Check extension supported.
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &m_maxAnisotropyLevel);

    printOpenGLError();

    m_initialized = true;
}

Renderer::~Renderer()
{
}

void Renderer::setRenderStatsLocation(RenderStats *renderStats)
{
    m_renderStats = renderStats;
}

void Renderer::beginFrame()
{
    clearColorBuffer();
    clearDepthBuffer();
    clearStencilBuffer();
    enableDepthTest(true);
    enableDepthWrite(true);
    enableScissorTest(false);
    setBlendMode(RenderPass::BlendingMode::NONE);

    m_programState->onFrameBegin();

    // Clear previous frame GL error.
    printOpenGLError();
}

void Renderer::endFrame()
{
    m_programState->onFrameEnd();

    glFlush();
}

void Renderer::setViewport(const Viewport &viewport)
{
    glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());

    m_programState->m_pixelSize = glm::vec2(1.0f / (float)viewport.width(), 1.0f / (float)viewport.height());
}

void Renderer::enableDepthTest(bool enable)
{
    if (m_depthTestOverriden)
        return;

    if (enable)
        glEnable(GL_DEPTH_TEST);
    else
        glDisable(GL_DEPTH_TEST);
}

void Renderer::enableDepthWrite(bool enable)
{
    if (m_depthWriteOverriden)
        return;

    glDepthMask(enable);
}

void Renderer::enableScissorTest(bool enable)
{
    if (enable)
        glEnable(GL_SCISSOR_TEST);
    else
        glDisable(GL_SCISSOR_TEST);
}

void Renderer::setScissorRegion(int x, int y, int width, int height)
{
    glScissor(x, y, width, height);
}

void Renderer::enableFog(float density, const glm::vec3 &fogColor)
{
    m_programState->m_fogDensity = density;
    m_programState->m_fogColor = fogColor;
}

void Renderer::enableBlendModeOverride(RenderPass::BlendingMode bm)
{
    m_blendModeOverriden = false;
    setBlendMode(bm);
    m_blendModeOverriden = true;
}

void Renderer::enableFrontFaceOverride(RenderPass::WindingOrder wo)
{
    m_frontFaceOverriden = false;
    setFrontFace(wo);
    m_frontFaceOverriden = true;
}

void Renderer::enableCullFaceOverride(RenderPass::FaceCullMode cm)
{
    m_cullFaceOverriden = false;
    setCullFace(cm);
    m_cullFaceOverriden = true;
}

void Renderer::enablePolygonDrawModeOverride(RenderPass::PolygonMode pm)
{
    m_polygonDrawModeOverriden = false;
    setPolygonDrawMode(pm);
    m_polygonDrawModeOverriden = true;
}

void Renderer::enableDepthTestOverride(bool enable)
{
    m_depthTestOverriden = false;
    enableDepthTest(enable);
    m_depthTestOverriden = true;
}

void Renderer::enableDepthWriteOverride(bool enable)
{
    m_depthWriteOverriden = false;
    enableDepthWrite(enable);
    m_depthWriteOverriden = true;
}

void Renderer::clearColorBuffer(const glm::vec4 &color)
{
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Renderer::clearDepthBuffer()
{
    glClear(GL_DEPTH_BUFFER_BIT);
}

void Renderer::clearStencilBuffer()
{
    glClear(GL_STENCIL_BUFFER_BIT);
}

float Renderer::readDepthBuffer(int x, int y)
{
    float z = 0.0f;
    glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
    return z;
}

void Renderer::setWorldMatrix(const glm::mat4 &worldm)
{
    m_programState->setWorldMatrix(worldm);
}

void Renderer::setCameraMatrix(const glm::mat4 &viewm)
{
    m_programState->setViewMatrix(viewm);
}

void Renderer::setProjectionMatrix(const glm::mat4 &projm)
{
    m_programState->setProjectionMatrix(projm);
}

void Renderer::setAmbientLight(const glm::vec3 &ambient)
{
    m_programState->m_globalAmbient = glm::vec4(ambient, 1.0f);
}

void Renderer::setAmbientLight(float ra, float ga, float ba)
{
    setAmbientLight(glm::vec3(ra, ga, ba));
}

void Renderer::setLight(const components::LightComponent *light)
{
    auto &glslLight = m_programState->m_currentLight;

    // Update light params.
    glslLight.diffuseParam = light->getDiffuseColor();
    glslLight.specularParam = light->getSpecularColor();
    glslLight.k0Param = light->getConstantAttenuation();
    glslLight.k1Param = light->getLinearAttenuation();
    glslLight.k2Param = light->getQuadraticAttenuation();

    // Since we calculate lighting in the view space we should translate position and direction.
    if (light->type() == components::LightComponent::Type::DIRECTIONAL)
    {
        auto dir = light->getDirection();
        glslLight.positionParam = m_programState->getViewMatrix() * glm::vec4(dir, 0.0f);
    }
    else if (light->type() == components::LightComponent::Type::POINT)
    {
        auto pos = light->getPosition();
        glslLight.positionParam = m_programState->getViewMatrix() * glm::vec4(pos, 1.0f);
    }
}

void Renderer::bindPass(shared_ptr<RenderPass> pass)
{
    if (!pass)
        return;

    if (pass.get() == m_programState->m_currentPass)
    {
        updateProgramUniformValues(m_programState->m_currentShader, m_programState->m_currentPass);
        updateTextureSamplers();
        return;
    }

    m_programState->m_currentPass = pass.get();

    // FIXME:
    // Cache state.
    enableDepthTest(pass->depthTestEnabled());
    enableDepthWrite(pass->depthWriteEnabled());
    setBlendMode(pass->getBlendingMode());
    setFrontFace(pass->getFrontFaceWinding());
    setCullFace(pass->getFaceCullMode());
    setPolygonDrawMode(pass->getPolygonDrawMode());

    auto glprogram = pass->getGpuProgram();
    // Sanity check.
    if (!glprogram)
        return;

    // Bind GPU program.
    if (!m_programState->m_currentShader || m_programState->m_currentShader != glprogram.get())
    {
        m_programState->m_currentShader = glprogram.get();
        m_programState->m_currentShader->bind();
    }

    updateProgramUniformValues(m_programState->m_currentShader, m_programState->m_currentPass);
    updateTextureSamplers();

    printOpenGLError();
}

void Renderer::drawVertexBuffer(shared_ptr<VertexBuffer> vb, shared_ptr<IndexBuffer> ib, RenderOperation::Type type)
{
    if (!vb)
        return;

    bool indexed = ib.get() != nullptr;

    vb->bind();
    if (indexed)
        ib->bind();

    auto vbUsed = vb->getElementsUsed();

    switch (type)
    {
    case RenderOperation::Type::LINES:
        if (vbUsed > 0)
            glDrawArrays(GL_LINES, 0, vbUsed);
        break;
    case RenderOperation::Type::TRIANGLES:
        if (indexed)
            glDrawElements(GL_TRIANGLES, ib->getElementsUsed(), GL_UNSIGNED_INT, nullptr);
        else if (vbUsed > 0)
            glDrawArrays(GL_TRIANGLES, 0, vbUsed);
        break;
    default:
        break;
    }

    //vb->unbind();
    //if (indexed)
    //    ib->unbind();

    if (m_renderStats)
    {
        m_renderStats->drawCalls++;

        // FIXME:
        if (type != RenderOperation::Type::LINES)
            m_renderStats->totalTriangles += indexed ? ib->getElementsUsed() / 3 : vb->getElementsUsed() / 3;
    }

    printOpenGLError();
}

int Renderer::getMaxTextureSize()
{
    if (!m_initialized)
    {
        base::glog << "Failed to get max texture size. Renderer is not initialized" << base::logwarn;
        return -1;
    }

    return m_maxTextureSize;
}

float Renderer::getMaxAnisotropy()
{
    if (!m_initialized)
    {
        base::glog << "Failed to get max anisotropy level. Renderer is not initialized" << base::logwarn;
        return 1.0f;
    }

    return m_maxAnisotropyLevel;
}

} }
