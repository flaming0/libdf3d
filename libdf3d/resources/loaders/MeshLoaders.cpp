#include "df3d_pch.h"
#include "MeshLoaders.h"

#include <base/Service.h>
#include <resources/FileDataSource.h>
#include <render/MaterialLib.h>
#include <utils/MeshUtils.h>

#include "MeshLoader_obj.h"
#include "MeshLoader_dfmesh.h"

namespace df3d { namespace resources {

MeshDataManualLoader::MeshDataManualLoader(std::vector<render::SubMesh> &&geometry)
    : m_geometry(std::move(geometry))
{

}

render::MeshData* MeshDataManualLoader::load()
{
    auto result = new render::MeshData(m_geometry);
    result->m_aabb.constructFromGeometry(m_geometry);
    result->m_obb.constructFromGeometry(m_geometry);
    result->m_sphere.constructFromGeometry(m_geometry);

    for (auto &s : m_geometry)
        utils::mesh::computeTangentBasis(s);

    return new render::MeshData(m_geometry);
}

MeshDataFSLoader::MeshDataFSLoader(const std::string &path, ResourceLoadingMode lm)
    : FSResourceLoader(lm),
    m_path(path)
{

}

render::MeshData* MeshDataFSLoader::createDummy()
{
    return new render::MeshData();
}

void MeshDataFSLoader::decode(shared_ptr<FileDataSource> source)
{
    auto extension = gsvc().filesystem.getFileExtension(source->getPath());

    if (extension == ".obj")
        m_mesh = MeshLoader_obj().load(source);
    else if (extension == ".dfmesh")
        m_mesh = MeshLoader_dfmesh().load(source);
}

void MeshDataFSLoader::onDecoded(Resource *resource)
{
    auto meshdata = static_cast<render::MeshData*>(resource);

    assert(m_mesh);

    meshdata->m_aabb = m_mesh->aabb;
    meshdata->m_obb = m_mesh->obb;
    meshdata->m_sphere = m_mesh->sphere;

    // Load all the materials.
    auto mtlLib = gsvc().resourceMgr.getFactory().createMaterialLib(m_mesh->materialLibName);

    if (mtlLib)     // Leaving null material in submesh, do not set default as it will be set later. mb it's wrong design?
    {
        assert(m_mesh->submeshes.size() == m_mesh->materialNames.size());

        for (size_t i = 0; i < m_mesh->submeshes.size(); i++)
        {
            auto &submesh = m_mesh->submeshes[i];

            if (m_mesh->materialNames[i])
                submesh.setMaterial(mtlLib->getMaterial(*m_mesh->materialNames[i]));
        }
    }

    meshdata->doInitMesh(m_mesh->submeshes);

    /*
    size_t meshTotalBytes = 0;
    for (const auto &s : m_mesh->submeshes)
        meshTotalBytes += s.getVertexData().getVerticesCount() * s.getVertexData().getFormat().getVertexSize();
    base::glog << "Cleaning up" << meshTotalBytes / 1024.0f << "KB of CPU copy of mesh data" << base::logdebug;
    */

    m_mesh.reset();     // Cleanup main memory.
}

} }