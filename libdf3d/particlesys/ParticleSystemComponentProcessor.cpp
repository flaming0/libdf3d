#include "ParticleSystemComponentProcessor.h"

#include "impl/ParticleSystemLoader.h"
#include "impl/SparkCommon.h"
#include "impl/SparkQuadRenderer.h"
#include "ParticleSystemUtils.h"
#include <libdf3d/game/ComponentDataHolder.h>
#include <libdf3d/game/World.h>
#include <libdf3d/3d/Camera.h>
#include <libdf3d/3d/SceneGraphComponentProcessor.h>
#include <libdf3d/base/EngineController.h>
#include <libdf3d/base/TimeManager.h>
#include <libdf3d/utils/JsonUtils.h>

namespace df3d {

struct ParticleSystemComponentProcessor::Impl
{
    struct Data
    {
        SPK::Ref<SPK::System> system;

        Entity holder;
        glm::mat4 holderTransform;
        bool paused = false;
        bool visible = true;
        bool worldTransformed = true;
        float systemLifeTime = -1.0f;
        float systemAge = 0.0f;
    };

    ComponentDataHolder<Data> data;
    particlesys_impl::ParticleSystemBuffers_Quad m_quadBuffers;

    static void updateCameraPosition(Data &compData, World *w)
    {
        auto spkSystem = compData.system;
        for (size_t i = 0; i < spkSystem->getNbGroups(); ++i)
        {
            if (spkSystem->getGroup(i)->isDistanceComputationEnabled())
            {
                auto pos = w->getCamera()->getPosition();
                if (!compData.worldTransformed)
                {
                    // Transform camera position into this node space.
                    auto invWorldTransform = glm::inverse(compData.holderTransform);

                    pos = glm::vec3((invWorldTransform * glm::vec4(pos, 1.0f)));
                }

                spkSystem->setCameraPosition(particlesys_impl::glmToSpk(pos));
                break;
            }
        }
    }
};

void ParticleSystemComponentProcessor::update()
{
    // Update the transform component.
    for (auto &compData : m_pimpl->data.rawData())
        compData.holderTransform = m_world->sceneGraph().getWorldTransform(compData.holder);

    auto dt = svc().timer().getFrameDelta(TimeChannel::GAME);
    for (auto &compData : m_pimpl->data.rawData())
    {
        if (compData.paused)
            continue;

        auto spkSystem = compData.system;

        if (compData.worldTransformed)
            spkSystem->getTransform().set(glm::value_ptr(compData.holderTransform));

        Impl::updateCameraPosition(compData, m_world);
        spkSystem->updateParticles(dt);

        if (compData.systemLifeTime > 0.0f)
        {
            compData.systemAge += dt;
            if (compData.systemAge > compData.systemLifeTime)
                compData.paused = true;
        }
    }
}

void ParticleSystemComponentProcessor::cleanStep(const std::list<Entity> &deleted)
{
    m_pimpl->data.cleanStep(deleted);
}

ParticleSystemComponentProcessor::ParticleSystemComponentProcessor(World *world)
    : m_pimpl(new Impl()),
    m_world(world)
{
    // Clamp the step to 100 ms.
    SPK::System::setClampStep(true, 0.1f);
    useRealStep();
}

ParticleSystemComponentProcessor::~ParticleSystemComponentProcessor()
{
    m_pimpl->data.clear();
    SPK_DUMP_MEMORY
}

void ParticleSystemComponentProcessor::useRealStep()
{
    SPK::System::useRealStep();
}

void ParticleSystemComponentProcessor::useConstantStep(float time)
{
    SPK::System::useConstantStep(time);
}

void ParticleSystemComponentProcessor::stop(Entity e)
{
    // TODO:
    DF3D_ASSERT(false, "not implemented");
}

void ParticleSystemComponentProcessor::pause(Entity e, bool paused)
{
    m_pimpl->data.getData(e).paused = paused;
}

void ParticleSystemComponentProcessor::setVisible(Entity e, bool visible)
{
    m_pimpl->data.getData(e).visible = visible;
}

void ParticleSystemComponentProcessor::setSystemLifeTime(Entity e, float lifeTime)
{
    m_pimpl->data.getData(e).systemLifeTime = lifeTime;
}

void ParticleSystemComponentProcessor::setWorldTransformed(Entity e, bool worldTransformed)
{
    m_pimpl->data.getData(e).worldTransformed = worldTransformed;
}

float ParticleSystemComponentProcessor::getSystemLifeTime(Entity e) const
{
    return m_pimpl->data.getData(e).systemLifeTime;
}

SPK::Ref<SPK::System> ParticleSystemComponentProcessor::getSystem(Entity e) const
{
    return m_pimpl->data.getData(e).system;
}

bool ParticleSystemComponentProcessor::isWorldTransformed(Entity e) const
{
    return m_pimpl->data.getData(e).worldTransformed;
}

bool ParticleSystemComponentProcessor::isPlaying(Entity e) const
{
    return !m_pimpl->data.getData(e).paused;
}

bool ParticleSystemComponentProcessor::isVisible(Entity e) const
{
    return !m_pimpl->data.getData(e).visible;
}

void ParticleSystemComponentProcessor::add(Entity e, const std::string &vfxResource)
{
    add(e, ParticleSystemUtils::parseVfx(vfxResource));
}

void ParticleSystemComponentProcessor::add(Entity e, const ParticleSystemCreationParams &params)
{
    if (m_pimpl->data.contains(e))
    {
        DFLOG_WARN("An entity already has a particle system component");
        return;
    }

    Impl::Data data;
    data.system = params.spkSystem;
    data.holder = e;
    data.systemLifeTime = params.systemLifeTime;
    data.worldTransformed = params.worldTransformed;
    data.holderTransform = m_world->sceneGraph().getWorldTransform(e);

    m_pimpl->data.add(e, data);
}

void ParticleSystemComponentProcessor::remove(Entity e)
{
    if (!m_pimpl->data.contains(e))
    {
        DFLOG_WARN("Failed to remove particle system component from an entity. Component is not attached");
        return;
    }

    m_pimpl->data.remove(e);
}

bool ParticleSystemComponentProcessor::has(Entity e)
{
    return m_pimpl->data.lookup(e).valid();
}

void ParticleSystemComponentProcessor::render()
{
    for (const auto &compData : m_pimpl->data.rawData())
    {
        if (compData.paused || !compData.visible)
            continue;

        glm::mat4 transf;
        if (!compData.worldTransformed)
            transf = compData.holderTransform;

        auto spkSystem = compData.system;
        // Prepare drawing of spark system.
        for (size_t i = 0; i < spkSystem->getNbGroups(); i++)
        {
            auto renderer = static_cast<particlesys_impl::ParticleSystemRenderer*>(spkSystem->getGroup(i)->getRenderer().get());
            renderer->m_currentTransformation = &transf;
            renderer->m_quadBuffers = &m_pimpl->m_quadBuffers;
        }

        spkSystem->renderParticles();
    }
}

}
