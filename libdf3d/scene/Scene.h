#pragma once

FWD_MODULE_CLASS(render, Material)
FWD_MODULE_CLASS(render, RenderQueue)

namespace df3d { namespace scene {

class Node;
class SceneManager;

class DF3D_DLL Scene
{
    friend class SceneManager;

    shared_ptr<render::Material> m_postProcessMaterial;
    shared_ptr<Node> m_root;

    glm::vec3 m_ambientLight = glm::vec3(1.0f, 1.0f, 1.0f);
    float m_fogDensity = 0.0f;
    glm::vec3 m_fogColor;

public:
    Scene(const char *sceneDefinitionFile);
    ~Scene();

    void setAmbientLight(float ra, float ga, float ba);
    glm::vec3 getAmbientLight() const;

    void setFog(float density, const glm::vec3 &color);
    void setFog(float density, float r, float g, float b);
    float getFogDensity() const;
    glm::vec3 getFogColor() const;

    void setPostProcessMaterial(shared_ptr<render::Material> material);
    shared_ptr<render::Material> getPostProcessMaterial() { return m_postProcessMaterial; }

    shared_ptr<Node> getRoot() const { return m_root; }

    void collectRenderOperations(render::RenderQueue *ops);

    shared_ptr<Node> getChildByName(const char *name) const;
    void addChild(shared_ptr<Node> child);
    void removeChild(shared_ptr<Node> child);
    void removeChild(const char *name);
    void removeAllChildren();
};

} }