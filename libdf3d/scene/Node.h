#pragma once

#include <components/NodeComponent.h>

FWD_MODULE_CLASS(components, TransformComponent)
FWD_MODULE_CLASS(components, MeshComponent)
FWD_MODULE_CLASS(components, LightComponent)
FWD_MODULE_CLASS(components, AudioComponent)
FWD_MODULE_CLASS(components, ParticleSystemComponent)
FWD_MODULE_CLASS(components, PhysicsComponent)
FWD_MODULE_CLASS(render, RenderQueue)

namespace df3d { namespace scene {

// A node in a scene graph.
// FIXME:
// Use more cache-friendly structure.

class DF3D_DLL Node : public boost::enable_shared_from_this<Node>, private boost::noncopyable
{
    friend class components::NodeComponent;

public:
    // Map instead of unordered_map because of Python.
    typedef std::map<std::string, shared_ptr<Node>> NodeChildren;

protected:
    std::string m_nodeName;
    bool m_visible = true;

    shared_ptr<components::NodeComponent> m_components[components::CT_COUNT];

    weak_ptr<Node> m_parent;
    NodeChildren m_children;

    void broadcastNodeEvent(components::Event ev);
    void broadcastComponentEvent(const components::NodeComponent *who, components::Event ev);

public:
    Node(const char *name = "");
    ~Node();

    void setName(const char *newName);
    const std::string &getName() const { return m_nodeName; }

    void setVisible(bool visible) { m_visible = visible; }
    bool isVisible() const { return m_visible; }

    void update(float dt);
    void draw(render::RenderQueue *ops);
    void traverse(std::function<void(shared_ptr<Node>)> fn);

    void addChild(shared_ptr<Node> child);
    void removeChild(shared_ptr<Node> child);
    void removeChild(const char *name);
    void removeAllChildren();

    NodeChildren::iterator begin() { return m_children.begin(); }
    NodeChildren::iterator end() { return m_children.end(); }

    NodeChildren::const_iterator cbegin() const { return m_children.cbegin(); }
    NodeChildren::const_iterator cend() const { return m_children.cend(); }

    shared_ptr<Node> getChildByName(const char *name) const;
    shared_ptr<Node> getParent() const { return m_parent.lock(); }

    shared_ptr<Node> clone() const;

    shared_ptr<components::NodeComponent> getComponent(components::ComponentType type) { return m_components[type]; }
    shared_ptr<components::TransformComponent> transform();
    shared_ptr<components::MeshComponent> mesh();
    shared_ptr<components::LightComponent> light();
    shared_ptr<components::AudioComponent> audio();
    shared_ptr<components::ParticleSystemComponent> vfx();
    shared_ptr<components::PhysicsComponent> physics();

    void attachComponent(shared_ptr<components::NodeComponent> component);
    void detachComponent(components::ComponentType type);
};

} }