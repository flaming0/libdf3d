#pragma once

#include <components/NodeComponent.h>

FWD_MODULE_CLASS(components, TransformComponent)
FWD_MODULE_CLASS(components, MeshComponent)
FWD_MODULE_CLASS(components, LightComponent)
FWD_MODULE_CLASS(components, AudioComponent)
FWD_MODULE_CLASS(components, ParticleSystemComponent)
FWD_MODULE_CLASS(components, PhysicsComponent)
FWD_MODULE_CLASS(components, Sprite2DComponent)
FWD_MODULE_CLASS(render, RenderQueue)

namespace df3d { namespace scene {

// A node in a scene graph.
// FIXME:
// Use more cache-friendly structure.

class DF3D_DLL Node : public std::enable_shared_from_this<Node>, utils::NonCopyable
{
    friend class components::NodeComponent;

public:
    using NodeChildren = std::vector<shared_ptr<Node>>;
    using Component = shared_ptr<components::NodeComponent>;

protected:
    std::string m_nodeName;
    bool m_visible = true;
    void *m_userPointer = nullptr;
    double m_lifeTime = 0.0;
    double m_maxLifeTime = -1.0;

    Component m_components[components::COUNT];

    // TODO:
    // List of active components.

    weak_ptr<Node> m_parent;
    NodeChildren m_children;

    void broadcastNodeEvent(components::NodeEvent ev);
    void broadcastComponentEvent(const components::NodeComponent *who, components::ComponentEvent ev);

    virtual void onComponentEvent(const components::NodeComponent *who, components::ComponentEvent ev) { }

public:
    Node(const std::string &name = "");
    virtual ~Node();

    void setName(const std::string &newName);
    const std::string &getName() const { return m_nodeName; }

    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void setUserPointer(void *userPointer) { m_userPointer = userPointer; }
    void* getUserPointer() const { return m_userPointer; }

    void setMaxLifeTime(double lifeTime) { m_maxLifeTime = lifeTime; }
    double getMaxLifeTime() const { return m_maxLifeTime; }
    double getLifeTime() const { return m_lifeTime; }

    void update(float dt);
    void draw(render::RenderQueue *ops);
    void traverse(std::function<void(shared_ptr<Node>)> fn);

    void addChild(shared_ptr<Node> child);
    void removeChild(shared_ptr<Node> child);
    void removeChild(const std::string &name);
    void removeAllChildren();

    NodeChildren::iterator begin() { return m_children.begin(); }
    NodeChildren::iterator end() { return m_children.end(); }

    NodeChildren::const_iterator cbegin() const { return m_children.cbegin(); }
    NodeChildren::const_iterator cend() const { return m_children.cend(); }

    shared_ptr<Node> getChildByName(const std::string &name) const;
    shared_ptr<Node> getParent() const { return m_parent.lock(); }
    size_t getChildrenCount() const { return m_children.size(); }

    shared_ptr<Node> clone() const;

    shared_ptr<components::NodeComponent> getComponent(components::ComponentType type) const { return m_components[type]; }
    shared_ptr<components::TransformComponent> transform();
    shared_ptr<components::MeshComponent> mesh();
    shared_ptr<components::LightComponent> light();
    shared_ptr<components::AudioComponent> audio();
    shared_ptr<components::ParticleSystemComponent> vfx();
    shared_ptr<components::PhysicsComponent> physics();
    shared_ptr<components::Sprite2DComponent> sprite2d();
    size_t attachedComponentsCount() const;

    void attachComponent(shared_ptr<components::NodeComponent> component);
    void detachComponent(components::ComponentType type);

    template<typename T, typename... Args>
    void createComponent(Args&&... args)
    {
        attachComponent(make_shared<T>(std::forward<Args>(args)...));
    }
};

} }
