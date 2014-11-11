#include "df3d_pch.h"
#include "RenderPass.h"

#include "GpuProgram.h"
#include "Texture.h"
#include "RenderManager.h"
#include "Renderer.h"
#include <resources/ResourceManager.h>
#include <base/Controller.h>

namespace df3d { namespace render {

void RenderPassParam::updateTo(GpuProgram *program)
{
    if (!m_uniform)
    {
        m_uniform = program->getCustomUniform(name);
        if (!m_uniform)
        {
            base::glog << "Uniform" << name << "was not found in shader" << program->getGUID() << base::logwarn;
            return;
        }
    }

    m_uniform->update(&value);
}

RenderPass::RenderPass(const std::string &name)
    : m_name(name)
{
    setGpuProgram(GpuProgram::createSimpleLightingGpuProgram());
    setSampler("diffuseMap", nullptr);
}

RenderPass::~RenderPass()
{
}

void RenderPass::setGpuProgram(shared_ptr<GpuProgram> newProgram)
{
    if (!newProgram)
    {
        base::glog << "Failed to set empty gpu program to a render pass" << base::logwarn;
        return;
    }
    m_gpuProgram = newProgram;
}

shared_ptr<GpuProgram> RenderPass::getGpuProgram() const
{
    return m_gpuProgram;
}

void RenderPass::setSampler(const char *name, shared_ptr<Texture> texture)
{
    auto found = std::find_if(m_samplers.begin(), m_samplers.end(), [&](const Sampler &s) -> bool { return s.name == name; } );
    if (found == m_samplers.end())
    {
        Sampler newsampler;
        newsampler.name = name;
        newsampler.texture = texture;
        m_samplers.push_back(newsampler);
    }
    else
    {
        found->texture = texture;
    }
}

std::vector<Sampler> &RenderPass::getSamplers()
{
    return m_samplers;
}

shared_ptr<Texture> RenderPass::getSampler(const char *name)
{
    auto found = std::find_if(m_samplers.cbegin(), m_samplers.cend(), [&](const Sampler &s) -> bool { return s.name == name; });
    if (found == m_samplers.cend())
        return nullptr;
    return found->texture;
}

void RenderPass::addPassParam(const RenderPassParam &param)
{
    if (param.name.empty())
    {
        base::glog << "Invalid shader parameter name" << base::logwarn;
        return;
    }

    auto found = std::find_if(m_passParams.cbegin(), m_passParams.cend(), [&](const RenderPassParam &p) -> bool { return p.name == param.name; });
    if (found != m_passParams.cend())
    {
        base::glog << "Render pass" << m_name << "already have shader parameter" << param.name << base::logwarn;
        return;
    }

    m_passParams.push_back(param);
}

RenderPassParam *RenderPass::getPassParam(const char *name)
{
    auto found = std::find_if(m_passParams.begin(), m_passParams.end(), [&](const RenderPassParam &p) -> bool { return p.name == name; });
    if (found == m_passParams.end())
    {
        base::glog << "Couldn't find pass param" << name << "in render pass" << getName() << base::logwarn;
        return nullptr;
    }

    return &(*found);
}

void RenderPass::setAmbientColor(float ra, float ga, float ba, float aa)
{
    m_ambientColor.r = ra;
    m_ambientColor.g = ga;
    m_ambientColor.b = ba;
    m_ambientColor.a = aa;
}

void RenderPass::setAmbientColor(const glm::vec4 &ambient)
{
    m_ambientColor = ambient;
}

void RenderPass::setDiffuseColor(float rd, float gd, float bd, float ad)
{
    m_diffuseColor.r = rd;
    m_diffuseColor.g = gd;
    m_diffuseColor.b = bd;
    m_diffuseColor.a = ad;
}

void RenderPass::setDiffuseColor(const glm::vec4 &diffuse)
{
    m_diffuseColor = diffuse;
}

void RenderPass::setSpecularColor(float rs, float gs, float bs, float as)
{
    m_specularColor.r = rs;
    m_specularColor.g = gs;
    m_specularColor.b = bs;
    m_specularColor.a = as;
}

void RenderPass::setSpecularColor(const glm::vec4 &specular)
{
    m_specularColor = specular;
}

void RenderPass::setEmissiveColor(float re, float ge, float be, float ae)
{
    m_emissiveColor.r = re;
    m_emissiveColor.g = ge;
    m_emissiveColor.b = be;
    m_emissiveColor.a = ae;
}

void RenderPass::setEmissiveColor(const glm::vec4 &emissive)
{
    m_emissiveColor = emissive;
}

void RenderPass::setShininess(float sh)
{
    m_shininess = sh;
}

void RenderPass::setFrontFaceWinding(WindingOrder wo)
{
    m_frontFaceWo = wo;
}

void RenderPass::setFaceCullMode(FaceCullMode mode)
{
    m_faceCullMode = mode;
}

void RenderPass::setPolygonDrawMode(PolygonMode mode)
{
    m_polygonMode = mode;
}

void RenderPass::setBlendMode(BlendingMode mode)
{
    m_blendMode = mode;
    m_isTransparent = m_blendMode != BM_NONE;
}

void RenderPass::enableDepthTest(bool enable)
{
    m_depthTest = enable;
}

void RenderPass::enableDepthWrite(bool enable)
{
    m_depthWrite = enable;
}

void RenderPass::enableLighting(bool enable)
{
    m_lightingEnabled = enable;
}

shared_ptr<RenderPass> RenderPass::createDebugDrawPass()
{
    auto pass = make_shared<render::RenderPass>("debug_draw_pass");
    pass->setFrontFaceWinding(render::RenderPass::WO_CCW);
    pass->setFaceCullMode(render::RenderPass::FCM_NONE);
    pass->setPolygonDrawMode(render::RenderPass::PM_WIRE);
    pass->setDiffuseColor(1.0f, 1.0f, 1.0f, 0.2f);
    pass->setBlendMode(render::RenderPass::BM_ALPHA);

    auto program = g_resourceManager->getResource<GpuProgram>(COLORED_PROGRAM_EMBED_PATH);
    pass->setGpuProgram(program);

    return pass;
}

} }