#include "pch.h"
#include "Loader.h"
#include <Resources\Material.h>
#include <Resources\Mesh.h>
#include <Scene\Components\MeshInstance.h>
#include <Scene\Components\BoundingBox.h>

using namespace Utils::Loader;

namespace Engine
{
    Loader::Loader(D3DClass::Ptr d3dClass, Scene::Ptr scene, ShaderPipeline::Ptr shaderPipeline) :
        m_d3dClass(d3dClass), m_scene(scene), m_shaderPipeline(shaderPipeline)
    {
    }

    Loader::~Loader()
    {

    }

    void Loader::LoadFile(const std::string& filePath)
    {
        // Eventually we can add additional flags to calculate normals and tangents
        // cache optimisations, etc...
        MikeLoader importer;
        auto importedScene = importer.LoadFile(filePath);
        LoadScene(importedScene);
    }

    void Loader::LoadFile(const uint8_t* data, uint64_t byteCount)
    {
        MikeLoader importer;
        auto importedScene = importer.LoadFile(data, byteCount);
        LoadScene(importedScene);
    }

    void Loader::LoadScene(const SceneData& importedScene)
    {
        LoadTextures(importedScene.Textures);
        LoadMaterials(importedScene.Materials);
        LoadSceneNodes(importedScene.SceneNodes);

        m_textureMap.clear();
        m_materialMap.clear();
        m_nodeMap.clear();
    }


    void Loader::LoadSceneNodes(const std::vector<Utils::Loader::SceneNodeData>& importedSceneNodes)
    {
        for (auto importedNode : importedSceneNodes)
        {
            LoadNode(importedNode);
        }
        // Get the root node of the model
        // The root node of the model should always have the ID 1
        EngineAssert(m_nodeMap.find(1) != m_nodeMap.end());
        auto rootNode = m_nodeMap[1];

        // Calculate the bounding box for the root of the model
        auto rootBounds = Scene::CalculateBoundingBoxForSceneNode(rootNode);
        auto boundsComponent = rootNode->AddComponent<BoundingBox>(m_d3dClass->GetDevice());
        boundsComponent->SetBounds(rootBounds);
        auto size = rootBounds.GetSize();
        auto maxAxis = std::max(size.x, std::max(size.y, size.z));
        auto scale = 1.0f / maxAxis;
        //rootNode->SetScale(scale); // TODO: Allow for enabling or disabling mesh unit scaling
    }

    void Loader::LoadNode(const SceneNodeData& importedNode)
    {
        // TODO: Need to add bounds component.
        // Add a new node to the scene to represent the imported node
        SceneNode::Ptr sceneNode;
        if (importedNode.ParentID == 0)
        {
            sceneNode = m_scene->AddNode();
        }
        else
        {
            bool parentExists = (m_nodeMap.find(importedNode.ParentID) != m_nodeMap.end());
            EngineAssert(parentExists);
            auto parentNode = m_nodeMap[importedNode.ParentID];
            sceneNode = m_scene->AddNode(parentNode);
        }

        // Now that we have created our new node add it to the map so
        // other nodes can parent to it.
        m_nodeMap[importedNode.NodeID] = sceneNode;
        sceneNode->SetTransform(importedNode.Transform);

        // If the node has vertices assigned then load the mesh data into a mesh object
        // and attach a MeshInstance to the node
        if (importedNode.VertexCount > 0)
        {
            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->Load(importedNode, m_d3dClass->GetDevice());

            MeshInstance::Ptr meshInstance = sceneNode->AddComponent<MeshInstance>(m_d3dClass->GetDevice());
            meshInstance->SetMesh(mesh);

            // Retrieve the material for this node. All materials and SceneNodes
            // should share the same ID
            EngineAssert(m_materialMap.find(importedNode.NodeID) != m_materialMap.end());
            Material::Ptr material = m_materialMap[importedNode.NodeID];
            meshInstance->SetMaterial(material);

            // Add bounding box to node
            auto bBox = sceneNode->AddComponent<BoundingBox>(m_d3dClass->GetDevice());
            bBox->SetBounds(importedNode.Bounds);
        }

    }

    void Loader::LoadMaterials(const std::vector<Utils::Loader::MaterialData>& importedMaterials)
    {
        for (auto importedMaterial : importedMaterials)
        {
            auto engineMaterial = std::make_shared<Material>(m_d3dClass->GetDevice(), m_shaderPipeline);
            engineMaterial->SetDiffuseColor(importedMaterial.DiffuseColor);
            engineMaterial->SetOpacity(importedMaterial.DiffuseColor.w);
            engineMaterial->SetSpecularColor(importedMaterial.SpecularColor);
            engineMaterial->SetSmoothness(importedMaterial.SpecularColor.w);
            engineMaterial->SetEmissiveColor(importedMaterial.EmissiveColor);

            // Now check to see any textures that are bound.
            // Diffuse
            if (importedMaterial.DiffuseTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.DiffuseTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Diffuse, it->second);
                }
            }

            // Specular
            if (importedMaterial.SpecularTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.SpecularTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Specular, it->second);
                }
            }

            // Emissive
            if (importedMaterial.EmissiveTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.EmissiveTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Emissive, it->second);
                }
            }

            // Normal
            if (importedMaterial.NormalTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.NormalTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Normal, it->second);
                }
            }

            // Smoothness
            if (importedMaterial.SmoothnessTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.SmoothnessTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Smoothness, it->second);
                }
            }

            // Opacity
            if (importedMaterial.OpacityTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.OpacityTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::Opacity, it->second);
                }
            }

            // Ambient Oclussion
            if (importedMaterial.AOTextureID != Utils::Loader::kInvalidID)
            {
                auto it = m_textureMap.find(importedMaterial.AOTextureID);
                if (it != m_textureMap.end())
                {
                    engineMaterial->SetTexture(Material::TextureType::BakedAO, it->second);
                }
            }
            // This material ID must not exist in the map already
            EngineAssert(m_materialMap.find(importedMaterial.ID) == m_materialMap.end());
            m_materialMap[importedMaterial.ID] = engineMaterial;
        }
    }

    void Loader::LoadTextures(const std::vector<Utils::Loader::TextureData>& importedTextures)
    {
        for (auto importedTexture : importedTextures)
        {
            // Need to ensure texture loaded correctly
            auto engineTexture = Texture::CreateImportedTexture(importedTexture, m_d3dClass->GetDevice());
            // This value must not exist in the map already
            EngineAssert(m_textureMap.find(importedTexture.ID) == m_textureMap.end());
            m_textureMap[importedTexture.ID] = engineTexture;
        }
    }
}