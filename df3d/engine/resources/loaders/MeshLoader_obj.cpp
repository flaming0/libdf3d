#include "MeshLoader_obj.h"

#include <df3d/engine/EngineController.h>
#include <df3d/engine/io/DefaultFileSystem.h>
#include <df3d/engine/io/DataSource.h>
#include <df3d/lib/MeshUtils.h>
#include <df3d/lib/Utils.h>

namespace df3d { namespace resource_loaders {

bool MeshLoader_obj::hasNormals() const
{
    return m_normals.size() > 0;
}

bool MeshLoader_obj::hasTxCoords() const
{
    return m_txCoords.size() > 0;
}

unique_ptr<SubMesh> MeshLoader_obj::createSubmesh(const std::string &materialName)
{
    auto vertexFormat = vertex_formats::p3_n3_tx2_tan3_bitan3;
    auto submesh = make_unique<SubMesh>(vertexFormat);
    submesh->setVertexBufferUsageHint(GpuBufferUsageType::STATIC);
    submesh->setIndexBufferUsageHint(GpuBufferUsageType::STATIC);

    DF3D_ASSERT(m_materialNameLookup.find(submesh.get()) == m_materialNameLookup.end());

    m_materialNameLookup[submesh.get()] = materialName;     // Store the material name in order to load it on the next stage.

    return submesh;
}

void MeshLoader_obj::processLine_v(std::istream &is)
{
    glm::vec3 v;
    is >> v.x >> v.y >> v.z;

    m_vertices.push_back(v);
}

void MeshLoader_obj::processLine_vt(std::istream &is)
{
    glm::vec2 uv;
    is >> uv.x >> uv.y;

    // NOTE:
    // Invert for OpenGL
    // FIXME: if DirectX???
    uv.y = 1.0f - uv.y;

    m_txCoords.push_back(uv);
}

void MeshLoader_obj::processLine_vn(std::istream &is)
{
    glm::vec3 vertexNormal;
    is >> vertexNormal.x >> vertexNormal.y >> vertexNormal.z;

    m_normals.push_back(vertexNormal);
}

void MeshLoader_obj::processLine_vp(std::istream &is)
{
    utils::skip_line(is);
}

void MeshLoader_obj::processLine_f(std::istream &is)
{
    // If do not have submesh yet, then create default and set is as current.
    if (!m_currentSubmesh)
    {
        auto submesh = createSubmesh(m_meshDataFileName);   // Using filename as the material name.

        m_currentSubmesh = submesh.get();
        m_submeshes[m_meshDataFileName] = std::move(submesh);
    }

    size_t verticesCount = 0;
    // FIXME:
    // Only triangles.
    while (verticesCount < 3)
    {
        char temp;

        verticesCount++;
        int vertexidx = 0;
        int normalidx = 0;
        int uvidx = 0;

        // vertex
        if (!hasNormals() && !hasTxCoords())
        {
            is >> vertexidx;
            DF3D_ASSERT(vertexidx >= 1);
        }
        // vertex//normal
        else if (!hasTxCoords())
        {
            is >> vertexidx >> temp >> temp >> normalidx;
            DF3D_ASSERT(vertexidx >= 1 && normalidx >= 1);
        }
        // vertex/texture
        else if (!hasNormals())
        {
            is >> vertexidx >> temp >> uvidx;
            DF3D_ASSERT(vertexidx >= 1 && uvidx >= 1);
        }
        // vertex/texture/normal
        else
        {
            is >> vertexidx >> temp >> uvidx >> temp >> normalidx;
            DF3D_ASSERT(vertexidx >= 1 && uvidx >= 1 && normalidx >= 1);
        }

        auto v = m_currentSubmesh->getVertexData().allocVertex();
        v.setPosition(m_vertices[vertexidx - 1]);

        if (normalidx > 0)
            v.setNormal(m_normals[normalidx - 1]);
        else
            v.setNormal({ 0.0f, 0.0f, 0.0f });
        if (uvidx > 0)
            v.setTx(m_txCoords[uvidx - 1]);
        else
            v.setTx({ 0.0f, 0.0f });

        if (!is.good())
        {
            DF3D_ASSERT(false);
            break;
        }
    }
}

void MeshLoader_obj::processLine_mtl(std::istream &is)
{
    std::string material;
    is >> material;

    // Create new vertex cache if not found, set as current.
    auto found = m_submeshes.find(material);
    if (found == m_submeshes.end())
    {
        auto submesh = createSubmesh(material);

        m_currentSubmesh = submesh.get();
        m_submeshes[material] = std::move(submesh);
    }
    else
    {
        m_currentSubmesh = found->second.get();
    }
}

void MeshLoader_obj::processLine_o(std::istream &is)
{
    utils::skip_line(is);
}

void MeshLoader_obj::processLine_g(std::istream &is)
{
    utils::skip_line(is);
}

void MeshLoader_obj::processLine_s(std::istream &is)
{
    utils::skip_line(is);
}

MeshLoader_obj::MeshLoader_obj()
    : m_vertices(MemoryManager::allocDefault()),
    m_normals(MemoryManager::allocDefault()),
    m_txCoords(MemoryManager::allocDefault())
{

}

unique_ptr<MeshDataFSLoader::Mesh> MeshLoader_obj::load(shared_ptr<DataSource> source)
{
    m_meshDataFileName = source->getPath();

    // Parse obj. TODO: can use stream directly.
    std::string buffer(source->getSize(), 0);
    source->read(&buffer[0], buffer.size());

    std::istringstream input(std::move(buffer));
    std::string tok;
    while (input >> tok)
    {
        utils::trim_left(tok);

        if (tok.empty() || tok[0] == '#')
        {
            utils::skip_line(input);
            continue;
        }

        if (tok == "f")
            processLine_f(input);
        else if (tok == "v")
            processLine_v(input);
        else if (tok == "vt")
            processLine_vt(input);
        else if (tok == "vn")
            processLine_vn(input);
        else if (tok == "vp")
            processLine_vp(input);
        else if (tok == "usemtl")
            processLine_mtl(input);
        else if (tok == "o")
            processLine_o(input);
        else if (tok == "g")
            processLine_g(input);
        else if (tok == "s")
            processLine_s(input);
        else if (tok == "mtllib")
            input >> m_materialLibPath;
    }

    bool computeNormals = m_normals.size() > 0 ? false : true;

    // Do post init.
    auto result = make_unique<MeshDataFSLoader::Mesh>();

    for (auto &s : m_submeshes)
    {
        if (computeNormals)
        {
            DFLOG_DEBUG("Computing normals in %s", source->getPath().c_str());
            MeshUtils::computeNormals(*s.second);
        }

        MeshUtils::computeTangentBasis(*s.second);

        auto mtlFound = m_materialNameLookup.find(s.second.get());
        unique_ptr<std::string> materialName;
        if (mtlFound != m_materialNameLookup.end())
            materialName = make_unique<std::string>(mtlFound->second);

        result->submeshes.push_back(std::move(*s.second));
        result->materialNames.push_back(std::move(materialName));
    }

    // Compute all the bounding volumes.
    result->aabb.constructFromGeometry(result->submeshes);
    result->sphere.constructFromGeometry(result->submeshes);
    result->obb.constructFromGeometry(result->submeshes);
    result->convexHull.constructFromGeometry(result->submeshes);

    result->materialLibName = m_materialLibPath;

    // TODO: can also unload materiallib from resource manager.

    // Clear all the state.
    m_materialLibPath.clear();
    m_meshDataFileName.clear();
    m_submeshes.clear();
    m_materialNameLookup.clear();
    m_currentSubmesh = nullptr;

    return result;
}

} }