#include "stdafx.h"
#include "Exporter.h"

namespace Utils
{
    namespace Loader
    {

        Exporter::Exporter(std::string outputFilePath, const SceneData& sceneData) :
            m_sceneData(sceneData), m_outputFile(outputFilePath, std::ios::out | std::ios::binary)
        {
            // Write File Header
            WriteHeaderToFile();
            // Write SceneNode Data
            WriteSceneNodesToFile();
            // Write Material Data
            WriteMaterialsToFile();
            // Write Texture Data
            WriteTexturesToFile();

            m_outputFile.close();
        }

        void Exporter::WriteHeaderToFile()
        {
            m_outputFile.write((char*)&kVersionNumber, sizeof(uint32_t));
        }

        void Exporter::WriteSceneNodesToFile()
        {
            for (auto sceneNode : m_sceneData.SceneNodes)
            {
                // Scene Node Data ID
                WriteToFile(&SceneNodeDataID);

                // Node IDs
                WriteToFile(&sceneNode.NodeID);
                WriteToFile(&sceneNode.ParentID);

                // Transform.
                WriteToFile(&sceneNode.Transform);

                // Vertex and index count
                WriteToFile(&sceneNode.VertexCount);
                WriteToFile(&sceneNode.IndexCount);

                // Vertex property flags
                WriteToFile(&sceneNode.HasPositions);
                WriteToFile(&sceneNode.HasNormals);
                WriteToFile(&sceneNode.HasTangents);
                WriteToFile(&sceneNode.HasUVs);

                // Write the bounding box
                WriteToFile(&sceneNode.Bounds);

                // Now write vertex data depending on what the scene node contains
                if (sceneNode.HasPositions)
                {
                    WriteToFile(sceneNode.Positions.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasNormals)
                {
                    WriteToFile(sceneNode.Normals.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasTangents)
                {
                    WriteToFile(sceneNode.Tangents.data(), sceneNode.VertexCount);
                    WriteToFile(sceneNode.Bitangents.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasUVs)
                {
                    WriteToFile(sceneNode.UVs.data(), sceneNode.VertexCount);
                }
                if (sceneNode.IndexCount > 0)
                {
                    WriteToFile(sceneNode.Indices.data(), sceneNode.IndexCount);
                }
            }
        }

        void Exporter::WriteMaterialsToFile()
        {
            for (auto material : m_sceneData.Materials)
            {
                // ID for material data so the loader knows what it is receiving next.
                WriteToFile(&MaterialDataID);

                // Material ID
                WriteToFile(&material.ID);

                // Diffuse color
                WriteToFile(&material.DiffuseColor);

                // Specular color
                WriteToFile(&material.SpecularColor);

                // Emissive color
                WriteToFile(&material.EmissiveColor);

                // Now write all of the material texture IDs
                // Diffuse
                WriteToFile(&material.DiffuseTextureID);

                // Specular
                WriteToFile(&material.SpecularTextureID);

                // Emissive
                WriteToFile(&material.EmissiveTextureID);

                // Normal
                WriteToFile(&material.NormalTextureID);

                // Shininess
                WriteToFile(&material.ShininessTextureID);

                // Opacity
                WriteToFile(&material.OpacityTextureID);

                // AO
                WriteToFile(&material.AOTextureID);
            }
        }

        void Exporter::WriteTexturesToFile()
        {
            for (auto texture : m_sceneData.Textures)
            {
                // Write ID for texture data so the loader knows what it is receiving next.
                WriteToFile(&TextureDataID);

                // Texture ID
                WriteToFile(&texture.ID);

                // Size of texture file in bytes
                WriteToFile(&texture.dataSize);

                // Raw .dds texture memory
                WriteToFile(texture.data.data(), texture.dataSize);
            }
        }
    } // End namespace Loader
} // End namespace Utils