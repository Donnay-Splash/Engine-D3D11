#include "Loader.h"

Loader::Loader()
{

}

SceneData Loader::LoadFile(std::string filePath)
{
    m_sceneData = SceneData();
    m_file.clear();
    m_file.open(filePath, std::ios::in | std::ios::binary);

    LoadHeaderFromFile();
    LoadSceneNodesFromFile();
    LoadMaterialsFromFile();
    LoadTexturesFromFile();

    return m_sceneData;
}

void Loader::LoadHeaderFromFile()
{
    ReadFromFile(&m_versionNumer);
    m_currentDataID = SceneNodeDataID;
}

void Loader::LoadSceneNodesFromFile()
{
    while (!AtEndOfFile())
    {
        ReadFromFile(&m_currentDataID);
        if (m_currentDataID != SceneNodeDataID)
        {
            break;
        }

        // Read standard scene node data in defined order.
        SceneNodeData sceneNode;
        // Node IDs
        ReadFromFile(&sceneNode.NodeID);
        ReadFromFile(&sceneNode.ParentID);

        // Transform.
        ReadFromFile(&sceneNode.Transform);

        // Vertex and index count
        ReadFromFile(&sceneNode.VertexCount);
        ReadFromFile(&sceneNode.IndexCount);

        // Vertex property flags
        ReadFromFile(&sceneNode.HasPositions);
        ReadFromFile(&sceneNode.HasNormals);
        ReadFromFile(&sceneNode.HasTangents);
        ReadFromFile(&sceneNode.HasUVs);

        // Now load vertex data
        if (sceneNode.HasPositions)
        {
            sceneNode.Positions.resize(sceneNode.VertexCount);
            ReadFromFile(sceneNode.Positions.data(), sceneNode.VertexCount);
        }
        if (sceneNode.HasNormals)
        {
            sceneNode.Normals.resize(sceneNode.VertexCount);
            ReadFromFile(sceneNode.Normals.data(), sceneNode.VertexCount);
        }
        if (sceneNode.HasTangents)
        {
            sceneNode.Tangents.resize(sceneNode.VertexCount);
            ReadFromFile(sceneNode.Tangents.data(), sceneNode.VertexCount);
            sceneNode.Bitangents.resize(sceneNode.VertexCount);
            ReadFromFile(sceneNode.Bitangents.data(), sceneNode.VertexCount);
        }
        if (sceneNode.HasUVs)
        {
            sceneNode.UVs.resize(sceneNode.VertexCount);
            ReadFromFile(sceneNode.UVs.data(), sceneNode.VertexCount);
        }
        if (sceneNode.IndexCount > 0)
        {
            sceneNode.Indices.resize(sceneNode.IndexCount);
            ReadFromFile(sceneNode.Indices.data(), sceneNode.IndexCount);
        }

        m_sceneData.SceneNodes.push_back(sceneNode);
    }
}

void Loader::LoadMaterialsFromFile()
{

}

void Loader::LoadTexturesFromFile()
{

}

bool Loader::AtEndOfFile()
{
    if (m_file.peek() == EOF)
    {
        if (m_file.eof())
        {
            return true;
        }
        else
        {
            throw std::exception("Error while reading file");
        }
    }

    return false;
}