#include "pch.h"
#include "Loader.h"
#include "Mesh.h"
#include "Material.h"

Loader::Loader()
{
    m_importer = std::make_unique<Assimp::Importer>();
}

Loader::~Loader()
{

}

void Loader::LoadFile(const std::string& filePath)
{
    auto loadedScene = m_importer->ReadFile(filePath, 0);
    auto rootNode = loadedScene->mRootNode;

    // TODO: Need to create the scene nodes
    auto rootModelNode = m_scene->AddNode();

    for (unsigned int i = 0; i < rootNode->mNumMeshes; i++)
    {
        // Load the mesh information.
        auto meshIndex = rootNode->mMeshes[i];
        auto mesh = loadedScene->mMeshes[meshIndex];
        auto engineMesh = std::make_shared<Mesh>();
        engineMesh->Load(mesh, m_d3dClass->GetDevice());

        // Load the material for the mesh
        auto materialIndex = mesh->mMaterialIndex;
        auto material = loadedScene->mMaterials[materialIndex];
        auto engineMaterial = std::make_shared<Material>();
        engineMaterial->Load();
    }

}