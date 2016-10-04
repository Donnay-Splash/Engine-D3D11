#include "stdafx.h"
#include "MikeLoader.h"
#include <iterator>

namespace Utils
{
    namespace Loader
    {
        MikeLoader::MikeLoader()
        {

        }

        SceneData MikeLoader::LoadFile(std::string filePath)
        {
            m_sceneData = SceneData();
            std::ifstream file(filePath, std::ios::in | std::ios::binary);
            LoadFileToBuffer(file);

            LoadDataFromBuffer();
            

            return m_sceneData;
        }

        SceneData MikeLoader::LoadFile(const uint8_t* data, uint64_t byteCount)
        {
            m_sceneData = SceneData();
            m_buffer = std::vector<uint8_t>(data, data + byteCount);
            m_readBytes = 0;

            LoadDataFromBuffer();

            return m_sceneData;
        }

        void MikeLoader::LoadFileToBuffer(std::ifstream& file)
        {
            file.unsetf(std::ios::skipws);
            std::streampos fileSize;

            file.seekg(0, std::ios::end);
            fileSize = file.tellg();
            file.seekg(0, std::ios::beg);

            m_readBytes = 0;
            m_buffer.clear();
            m_buffer.reserve(fileSize);
            m_buffer.insert(m_buffer.begin(),
                std::istream_iterator<uint8_t>(file),
                std::istream_iterator<uint8_t>());
        }

        void MikeLoader::LoadDataFromBuffer()
        {
            LoadHeaderFromBuffer();
            LoadSceneNodesFromBuffer();
            LoadMaterialsFromBuffer();
            LoadTexturesFromBuffer();
        }

        void MikeLoader::LoadHeaderFromBuffer()
        {
            ReadFromBuffer(&m_versionNumer);
            ReadFromBuffer(&m_currentDataID);
        }

        void MikeLoader::LoadSceneNodesFromBuffer()
        {
            while (true)
            {
                if (m_currentDataID != SceneNodeDataID)
                {
                    break;
                }

                // Read standard scene node data in defined order.
                SceneNodeData sceneNode;
                // Node IDs
                ReadFromBuffer(&sceneNode.NodeID);
                ReadFromBuffer(&sceneNode.ParentID);

                // Transform.
                ReadFromBuffer(&sceneNode.Transform);

                // Vertex and index count
                ReadFromBuffer(&sceneNode.VertexCount);
                ReadFromBuffer(&sceneNode.IndexCount);

                // Vertex property flags
                ReadFromBuffer(&sceneNode.HasPositions);
                ReadFromBuffer(&sceneNode.HasNormals);
                ReadFromBuffer(&sceneNode.HasTangents);
                ReadFromBuffer(&sceneNode.HasUVs);

                // Now load vertex data
                if (sceneNode.HasPositions)
                {
                    sceneNode.Positions.resize(sceneNode.VertexCount);
                    ReadFromBuffer(sceneNode.Positions.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasNormals)
                {
                    sceneNode.Normals.resize(sceneNode.VertexCount);
                    ReadFromBuffer(sceneNode.Normals.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasTangents)
                {
                    sceneNode.Tangents.resize(sceneNode.VertexCount);
                    ReadFromBuffer(sceneNode.Tangents.data(), sceneNode.VertexCount);
                    sceneNode.Bitangents.resize(sceneNode.VertexCount);
                    ReadFromBuffer(sceneNode.Bitangents.data(), sceneNode.VertexCount);
                }
                if (sceneNode.HasUVs)
                {
                    sceneNode.UVs.resize(sceneNode.VertexCount);
                    ReadFromBuffer(sceneNode.UVs.data(), sceneNode.VertexCount);
                }
                if (sceneNode.IndexCount > 0)
                {
                    sceneNode.Indices.resize(sceneNode.IndexCount);
                    ReadFromBuffer(sceneNode.Indices.data(), sceneNode.IndexCount);
                }

                m_sceneData.SceneNodes.push_back(sceneNode);

                // If we are yet to reach the end of the buffer
                // then read the next data type ID
                if (!AtEndOfBuffer())
                {
                    // Now we have finished loading this node.
                    // Check what the next data type is.
                    ReadFromBuffer(&m_currentDataID);
                }
                else
                {
                    break;
                }
            }
        }

        void MikeLoader::LoadMaterialsFromBuffer()
        {
            while (true)
            {
                if (m_currentDataID != MaterialDataID)
                {
                    break;
                }

                MaterialData material;

                // Material ID that matches the Node that uses it
                ReadFromBuffer(&material.ID);

                // Diffuse Color
                ReadFromBuffer(&material.DiffuseColor);

                // Specular Color
                ReadFromBuffer(&material.SpecularColor);

                // Emissive Color
                ReadFromBuffer(&material.EmissiveColor);

                // Now read all of the material texture IDs
                // Diffuse
                ReadFromBuffer(&material.DiffuseTextureID);

                // Specular
                ReadFromBuffer(&material.SpecularTextureID);

                // Emissive
                ReadFromBuffer(&material.EmissiveTextureID);

                // Normal
                ReadFromBuffer(&material.NormalTextureID);

                // Shininess
                ReadFromBuffer(&material.ShininessTextureID);

                // Opacity
                ReadFromBuffer(&material.OpacityTextureID);

                // AO
                ReadFromBuffer(&material.AOTextureID);

                m_sceneData.Materials.push_back(material);

                // If we are yet to reach the end of the buffer
                // then read the next data type ID
                if (!AtEndOfBuffer())
                {
                    // Now that the material has finished loading check what the next data type is.
                    ReadFromBuffer(&m_currentDataID);
                }
                else
                {
                    break;
                }
            }
        }

        void MikeLoader::LoadTexturesFromBuffer()
        {
            while (true)
            {
                if (m_currentDataID != TextureDataID)
                {
                    break;
                }

                TextureData texture;

                // Texture ID that will be used to match the textures with materials
                ReadFromBuffer(&texture.ID);

                // The size of the texture file in bytes
                ReadFromBuffer(&texture.dataSize);

                // The .dds texture data
                texture.data.resize(texture.dataSize);
                ReadFromBuffer(texture.data.data(), texture.dataSize);

                m_sceneData.Textures.push_back(texture);

                // If we are yet to reach the end of the buffer
                // then read the next data type ID
                if (!AtEndOfBuffer())
                {
                    // Now that the material has finished loading check what the next data type is.
                    ReadFromBuffer(&m_currentDataID);
                }
                else
                {
                    break;
                }
            }
        }

        bool MikeLoader::AtEndOfBuffer()
        {
            return m_readBytes >= m_buffer.size();
        }

    } // End namespace Loader
} // End namespace Utils