#pragma once

#include <Scene\Public Properties\SceneElement.h>
#include <Resources\ShaderPipeline.h>

namespace Engine
{
    // Forward declarations
    class SceneNode;


    class Component : public SceneElement
    {
    protected:
        using SceneNodePtr = std::shared_ptr<SceneNode>;

    public:
        using Ptr = std::shared_ptr<Component>;

        SceneNodePtr GetSceneNode() const { return m_sceneNode; }

        // Not required to be overidden by child classes. Can be hooked into if needed
        virtual void Update(float frameTime) {};

        // Not required to be overriden by child classes. Can be hooked into if needed.
        virtual void Render(ID3D11DeviceContext* deviceContext, ShaderPipeline::Ptr shaderOverride = nullptr) const {};

        virtual ~Component() {}

    protected:
        Component(SceneNodePtr sceneNode, std::wstring elementName);
        Component(const Component&) = delete;
        virtual void Initialize(ID3D11Device* device) = 0;
        virtual void OnSceneNodeTransformChanged(const Utils::Maths::Matrix& prevWorldTransform) {}

    private:
        SceneNodePtr m_sceneNode;

        friend class SceneNode;
    };

    using ComponentContainer = std::vector<Component::Ptr>;
}
