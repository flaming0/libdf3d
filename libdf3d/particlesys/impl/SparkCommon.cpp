#include "SparkCommon.h"

#include <libdf3d/base/EngineController.h>
#include <libdf3d/resources/ResourceManager.h>
#include <libdf3d/resources/ResourceFactory.h>
#include <libdf3d/render/RenderPass.h>

namespace df3d { namespace particlesys_impl {

ParticleSystemRenderer::ParticleSystemRenderer(bool NEEDS_DATASET)
    : SPK::Renderer(NEEDS_DATASET),
    m_pass(make_shared<RenderPass>())
{
    m_pass->setFaceCullMode(FaceCullMode::BACK);
    m_pass->setParam("material_diffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_pass->setParam("diffuseMap", nullptr);          // FIXME: force to use default white texture (as using colored program)

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
        m_pass->setBlendMode(BlendingMode::NONE);
        break;
    case SPK::BLEND_MODE_ADD:
        m_pass->setBlendMode(BlendingMode::ADDALPHA);
        break;
    case SPK::BLEND_MODE_ALPHA:
        m_pass->setBlendMode(BlendingMode::ALPHA);
        break;
    default:
        break;
    }
}

void ParticleSystemRenderer::setDiffuseMap(shared_ptr<Texture> texture)
{
    m_pass->setParam("diffuseMap", texture);
}

void ParticleSystemRenderer::enableFaceCulling(bool enable)
{
    if (enable)
        m_pass->setFaceCullMode(FaceCullMode::BACK);
    else
        m_pass->setFaceCullMode(FaceCullMode::NONE);
}

} }
