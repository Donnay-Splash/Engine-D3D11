#pragma once

#include "SceneNode.h"

class Scene : public std::enable_shared_from_this<Scene>
{
public:
    using Ptr = std::shared_ptr<Scene>;
    Scene();
    Scene(const Scene&) = delete;
    ~Scene() {}

    void Initialize();

    SceneNode::Ptr AddNode(SceneNode::Ptr parentNode = nullptr);
    SceneNode::Ptr GetRootNode() const { return m_rootNode; }

    void Update(float frameTime);
    void Render(ID3D11DeviceContext* deviceContext);

private:
    SceneNode::Ptr m_rootNode;
};