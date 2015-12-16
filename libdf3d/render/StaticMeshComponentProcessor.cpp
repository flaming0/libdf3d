#include "df3d_pch.h"
#include "StaticMeshComponentProcessor.h"

#include <base/EngineController.h>
#include <scene/World.h>
#include <scene/TransformComponentProcessor.h>
#include <scene/impl/ComponentDataHolder.h>
#include <resources/ResourceManager.h>
#include <resources/ResourceFactory.h>
#include <render/MeshData.h>
#include <utils/MathUtils.h>

namespace df3d {

struct StaticMeshComponentProcessor::Impl
{
    // TODO_ecs: more cache friendly layout.
    struct Data
    {
        shared_ptr<MeshData> meshData;
        ComponentInstance transformComponent;

        bool visible = true;
        bool frustumCullingDisabled = false;
    };

    ComponentDataHolder<Data> data;
};

void StaticMeshComponentProcessor::draw(RenderQueue *ops)
{

}
void StaticMeshComponentProcessor::cleanStep(World &w)
{

}

StaticMeshComponentProcessor::StaticMeshComponentProcessor()
    : m_pimpl(new Impl())
{

}

StaticMeshComponentProcessor::~StaticMeshComponentProcessor()
{

}

shared_ptr<MeshData> StaticMeshComponentProcessor::getMeshData(ComponentInstance comp) const
{
    return m_pimpl->data.getData(comp).meshData;
}

AABB StaticMeshComponentProcessor::getAABB(ComponentInstance comp)
{
    // FIXME: mb cache if transformation hasn't been changed?
    const auto &compData = m_pimpl->data.getData(comp);

    AABB transformedAABB;

    auto modelSpaceAABB = compData.meshData->getAABB();
    if (!modelSpaceAABB || !modelSpaceAABB->isValid())
        return transformedAABB;

    // Get the corners of original AABB (model-space).
    std::vector<glm::vec3> aabbCorners(8);
    modelSpaceAABB->getCorners(aabbCorners);

    // Create new AABB from the corners of the original also applying world transformation.
    auto tr = world().transform().getTransformation(compData.transformComponent);
    for (auto &p : aabbCorners)
    {
        auto trp = tr * glm::vec4(p, 1.0f);
        transformedAABB.updateBounds(glm::vec3(trp));
    }

    return transformedAABB;
}

BoundingSphere StaticMeshComponentProcessor::getBoundingSphere(ComponentInstance comp)
{
    // FIXME: mb cache if transformation hasn't been changed?
    const auto &compData = m_pimpl->data.getData(comp);

    auto meshDataSphere = compData.meshData->getBoundingSphere();
    if (!meshDataSphere || !meshDataSphere->isValid())
        return BoundingSphere();

    BoundingSphere sphere = *meshDataSphere;

    // Update transformation.
    auto pos = world().transform().getPosition(compData.transformComponent, true);
    auto scale = world().transform().getScale(compData.transformComponent);
    sphere.setPosition(pos);

    // FIXME: absolutely incorrect!!! Should take into account children.
    // TODO_ecs: 

    // FIXME: wtf is this??? Why can't just scale radius?
    auto rad = sphere.getRadius() * utils::math::UnitVec3;
    rad.x *= scale.x;
    rad.y *= scale.y;
    rad.z *= scale.z;
    sphere.setRadius(glm::length(rad));

    return sphere;
}

OBB StaticMeshComponentProcessor::getOBB(ComponentInstance comp)
{
    // FIXME: mb cache if transformation hasn't been changed?
    assert(false && "Not implemented");
    return OBB();
}

void StaticMeshComponentProcessor::setVisible(ComponentInstance comp, bool visible)
{
    m_pimpl->data.getData(comp).visible = visible;
}

void StaticMeshComponentProcessor::disableFrustumCulling(ComponentInstance comp, bool disable)
{
    m_pimpl->data.getData(comp).frustumCullingDisabled = disable;
}

ComponentInstance StaticMeshComponentProcessor::add(Entity e, const std::string &meshFilePath, ResourceLoadingMode lm)
{
    if (m_pimpl->data.contains(e))
    {
        glog << "An entity already has a static mesh component" << logwarn;
        return ComponentInstance();
    }

    Impl::Data data;
    data.meshData = svc().resourceManager().getFactory().createMeshData(meshFilePath, lm);
    data.transformComponent = world().transform().lookup(e);
    assert(data.transformComponent.valid());

    return m_pimpl->data.add(e, data);
}

void StaticMeshComponentProcessor::remove(Entity e)
{
    if (!m_pimpl->data.contains(e))
    {
        glog << "Failed to remove static mesh component from an entity. Component is not attached" << logwarn;
        return;
    }

    m_pimpl->data.remove(e);
}

ComponentInstance StaticMeshComponentProcessor::lookup(Entity e)
{
    return m_pimpl->data.lookup(e);
}

}
