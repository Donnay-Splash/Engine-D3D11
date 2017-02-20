#include "pch.h"
#include "MeshMaker.h"

using namespace Engine;
namespace Utils
{
        Mesh::Ptr MeshMaker::CreateTriangle(ID3D11Device* device)
        {
            Mesh::PositionContainer positions(3);
            Mesh::NormalContainer normals(3);
            Mesh::UVContainer uvs(3);
            Mesh::IndexContainer indices(3);
            
            positions[0] = Utils::Maths::Vector3(-0.5f, -0.5f, 0.0f);  // Bottom left.
            normals[0] = Utils::Maths::Vector3(0.0f, 0.0f, -1.0f);
            uvs[0] = Utils::Maths::Vector2(0.0f, 1.0f);

            positions[1] = Utils::Maths::Vector3(0.0f, 0.5f, 0.0f);  // Top middle.
            normals[1] = Utils::Maths::Vector3(0.0f, 0.0f, -1.0f);
            uvs[1] = Utils::Maths::Vector2(0.0f, 0.0f);

            positions[2] = Utils::Maths::Vector3(0.5f, -0.5f, 0.0f);  // bottom right.
            normals[2] = Utils::Maths::Vector3(0.0f, 0.0f, -1.0f);
            uvs[2] = Utils::Maths::Vector2(1.0f, 0.0f);

            // Load the index array with data.
            indices[0] = 0;  // Bottom left.
            indices[1] = 1;  // Top left.
            indices[2] = 2;  // Top right.

            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->SetPositions(positions);
            mesh->SetNormals(normals);
            mesh->SetUVs(uvs);
            mesh->SetIndices(indices);
            mesh->FinaliseMesh(device);
            return mesh;
        }

        Mesh::Ptr MeshMaker::CreateSphere(ID3D11Device* device)
        {
            // Not yet implemented.
            EngineAssert(false);
            return nullptr;
        }

        Mesh::Ptr MeshMaker::CreateCube(ID3D11Device* device)
        {
            Mesh::PositionContainer positions(24);
            Mesh::NormalContainer normals(24);
            Mesh::UVContainer uvs(24);
            Mesh::IndexContainer indices(36);

            // Front face
            positions[0] = { -0.5f, -0.5f, -0.5f };  // Bottom left.
            normals[0] = { 0.0f, 0.0f, -1.0f };
            uvs[0] = { 0.0f, 1.0f };

            positions[1] = { -0.5f, 0.5f, -0.5f };  // Top left.
            normals[1] = { 0.0f, 0.0f, -1.0f };
            uvs[1] = { 0.0f, 0.0f };

            positions[2] = { 0.5f, -0.5f, -0.5f };  // Bottom right.
            normals[2] = { 0.0f, 0.0f, -1.0f };
            uvs[2] = { 1.0f, 1.0f };

            positions[3] = { 0.5f, 0.5f, -0.5f };  // Top right.
            normals[3] = { 0.0f, 0.0f, -1.0f };
            uvs[3] = { 1.0f, 0.0f };

            // Left face
            positions[4] = { -0.5f, -0.5f, 0.5f };  // Bottom left.
            normals[4] = { -1.0f, 0.0f, 0.0f };
            uvs[4] = { 0.0f, 1.0f };

            positions[5] = { -0.5f, 0.5f, 0.5f };  // Top left.
            normals[5] = { -1.0f, 0.0f, 0.0f };
            uvs[5] = { 0.0f, 0.0f };

            positions[6] = { -0.5f, -0.5f, -0.5f };  // Bottom right.
            normals[6] = { -1.0f, 0.0f, 0.0f };
            uvs[6] = { 1.0f, 1.0f };

            positions[7] = { -0.5f, 0.5f, -0.5f };  // Top right.
            normals[7] = { -1.0f, 0.0f, 0.0f };
            uvs[7] = { 1.0f, 0.0f };

            // Back face
            positions[8] = { 0.5f, -0.5f, 0.5f };  // Bottom left.
            normals[8] = { 0.0f, 0.0f, 1.0f };
            uvs[8] = { 0.0f, 1.0f };

            positions[9] = { 0.5f, 0.5f, 0.5f };  // Top left.
            normals[9] = { 0.0f, 0.0f, 1.0f };
            uvs[9] = { 0.0f, 0.0f };

            positions[10] = { -0.5f, -0.5f, 0.5f };  // Bottom right.
            normals[10] = { 0.0f, 0.0f, 1.0f };
            uvs[10] = { 1.0f, 1.0f };

            positions[11] = { -0.5f, 0.5f, 0.5f };  // Top right.
            normals[11] = { 0.0f, 0.0f, 1.0f };
            uvs[11] = { 1.0f, 0.0f };

            // Right face
            positions[12] = { 0.5f, -0.5f, -0.5f };  // Bottom left.
            normals[12] = { 1.0f, 0.0f, 0.0f };
            uvs[12] = { 0.0f, 1.0f };

            positions[13] = { 0.5f, 0.5f, -0.5f };  // Top left.
            normals[13] = { 1.0f, 0.0f, 0.0f };
            uvs[13] = { 0.0f, 0.0f };

            positions[14] = { 0.5f, -0.5f, 0.5f };  // Bottom right.
            normals[14] = { 1.0f, 0.0f, 0.0f };
            uvs[14] = { 1.0f, 1.0f };

            positions[15] = { 0.5f, 0.5f, 0.5f };  // Top right.
            normals[15] = { 1.0f, 0.0f, 0.0f };
            uvs[15] = { 1.0f, 0.0f };

            // Top face
            positions[16] = { -0.5f, 0.5f, -0.5f };  // Bottom left.
            normals[16] = { 0.0f, 1.0f, 0.0f };
            uvs[16] = { 0.0f, 1.0f };

            positions[17] = { -0.5f, 0.5f, 0.5f };  // Top left.
            normals[17] = { 0.0f, 1.0f, 0.0f };
            uvs[17] = { 0.0f, 0.0f };

            positions[18] = { 0.5f, 0.5f, -0.5f };  // Bottom right.
            normals[18] = { 0.0f, 1.0f, 0.0f };
            uvs[18] = { 1.0f, 1.0f };

            positions[19] = { 0.5f, 0.5f, 0.5f };  // Top right.
            normals[19] = { 0.0f, 1.0f, 0.0f };
            uvs[19] = { 1.0f, 0.0f };

            // Bottom face
            positions[20] = { -0.5f, -0.5f, 0.5f };  // Bottom left.
            normals[20] = { 0.0f, -1.0f, 0.0f };
            uvs[20] = { 0.0f, 1.0f };

            positions[21] = { -0.5f, -0.5f, -0.5f };  // Top left.
            normals[21] = { 0.0f, -1.0f, 0.0f };
            uvs[21] = { 0.0f, 0.0f };

            positions[22] = { 0.5f, -0.5f, 0.5f };  // Bottom right.
            normals[22] = { 0.0f, -1.0f, 0.0f };
            uvs[22] = { 1.0f, 1.0f };

            positions[23] = { 0.5f, -0.5f, -0.5f };  // Top right.
            normals[23] = { 0.0f, -1.0f, -0.0f };
            uvs[23] = { 1.0f, 0.0f };

            // Load the index array with data.
            // FRONT FACE
            indices[0] = 0;  // Bottom left.
            indices[1] = 1;  // Top left.
            indices[2] = 3;  // Top right.

            indices[3] = 3;  // Top Right.
            indices[4] = 2;  // Bottom Right.
            indices[5] = 0;  // Bottom Left.

            // LEFT FACE
            indices[6] = 4;  // Bottom left.
            indices[7] = 5;  // Top left.
            indices[8] = 7;  // Top right.

            indices[9]  = 7;  // Top Right.
            indices[10] = 6;  // Bottom Right.
            indices[11] = 4;  // Bottom Left.

            // BACK FACE
            indices[12] = 8;  // Bottom left.
            indices[13] = 9;  // Top left.
            indices[14] = 11;  // Top right.

            indices[15] = 11;  // Bottom left.
            indices[16] = 10;  // Top left.
            indices[17] = 8;  // Top right.

            // RIGHT FACE
            indices[18] = 12;  // Bottom left.
            indices[19] = 13;  // Top left.
            indices[20] = 15;  // Top right.

            indices[21] = 15;  // Bottom left.
            indices[22] = 14;  // Top left.
            indices[23] = 12;  // Top right.

             // TOP FACE
            indices[24] = 16;  // Bottom left.
            indices[25] = 17;  // Top left.
            indices[26] = 19;  // Top right.

            indices[27] = 19;  // Bottom left.
            indices[28] = 18;  // Top left.
            indices[29] = 16;  // Top right.

            // BOTTOM FACE
            indices[30] = 20;  // Bottom left.
            indices[31] = 21;  // Top left.
            indices[32] = 23;  // Top right.

            indices[33] = 23;  // Bottom left.
            indices[34] = 22;  // Top left.
            indices[35] = 20;  // Top right.

            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->SetPositions(positions);
            mesh->SetNormals(normals);
            mesh->SetUVs(uvs);
            mesh->SetIndices(indices);
            mesh->FinaliseMesh(device);
            return mesh;
        }

        Engine::Mesh::Ptr MeshMaker::CreateTorus(ID3D11Device * device, float internalRadius, float externalRadius, uint32_t triDensity)
        {
            const uint32_t minAngleIterations = 8;
            uint32_t angleIterations = std::max(minAngleIterations, triDensity);
            // Change in angle for both internal and external angles. In radians
            float angleDelta = 2 * Utils::Maths::kPI / angleIterations;

            const uint32_t vertCount = angleIterations * angleIterations * 4u;
            const uint32_t indexCount = angleIterations * angleIterations * 6u;

            // Create containers for vertex data. UV mapping is a little tricky for
            // a Torus so for now I'm leaving it.
            Mesh::PositionContainer positions(vertCount);
            Mesh::NormalContainer normals(vertCount);
            Mesh::IndexContainer indices(indexCount);

            auto CalcVertex = [=](float internalAngle, float externalAngle)->Utils::Maths::Vector3
            {
                Utils::Maths::Vector3 vert;
                const float internalPos = (internalRadius + externalRadius*cos(externalAngle));
                vert.x = internalPos * cos(internalAngle);
                vert.y = internalPos * sin(internalAngle);
                vert.z = externalRadius * sin(externalAngle);

                return vert;
            };


            // Function for a Torus
            // x = (c + a*cos(v))*cos(u)
            // y = (c + a*cos(v))*sin(u)
            // z = a * sin(v)
            // Where c is the major radius in metres (internalRadius)
            // and a is the minor radius in metres (externalRadius)
            // u is the internal angle in radians
            // and v is the external angle in radians
            // This gives us a Torus sitting on the x-y plane.

            // To generate out Torus we run a loop for the major internal radius
            // and inside of that create rings of quads by incrementing our minor external radius.
            // Since we want smooth normals we can share vertices between quads. This will take some
            // fiddeling but will result in the most optimal mesh.

            float internalAngle = 0.0f;
            uint32_t currentIndexCount = 0u;
            for (uint32_t i = 0; i < angleIterations; i++)
            {
                uint32_t baseIndex = i * angleIterations * 4u;
                
                // Minor circle centre pos.
                Utils::Maths::Vector3 minorCircleCentreLeft = { internalRadius * cos(internalAngle), internalRadius * sin(internalAngle), 0.0f };
                Utils::Maths::Vector3 minorCircleCentreRight = { internalRadius * cos(internalAngle + angleDelta), internalRadius * sin(internalAngle + angleDelta), 0.0f };

                float externalAngle = 0.0f;
                for (uint32_t j = 0; j < angleIterations; j++)
                {
                    uint32_t currentIndex = baseIndex + j * 4u;

                    // Top Left = internalAngle, externalAngle
                    // Top Right = internalAngle + delta, externalAngle
                    // Bottom Right = internalAngle + delta, externalAngle + delta
                    // Bottom Left = internalAngle, externalAngle + delta

                    // Top Left
                    Utils::Maths::Vector3 topLeftP = CalcVertex(internalAngle, externalAngle); // position
                    Utils::Maths::Vector3 topLeftN = Utils::Maths::Vector3::Normalize(topLeftP - minorCircleCentreLeft); // normal

                    // Top Right
                    Utils::Maths::Vector3 topRightP = CalcVertex(internalAngle + angleDelta, externalAngle); // position
                    Utils::Maths::Vector3 topRightN = Utils::Maths::Vector3::Normalize(topRightP - minorCircleCentreRight); // normal

                    // Bottom Right
                    Utils::Maths::Vector3 bottomRightP = CalcVertex(internalAngle + angleDelta, externalAngle + angleDelta); // position
                    Utils::Maths::Vector3 bottomRightN = Utils::Maths::Vector3::Normalize(bottomRightP - minorCircleCentreRight); // normal

                    // BottomLeft
                    Utils::Maths::Vector3 bottomLeftP = CalcVertex(internalAngle, externalAngle + angleDelta); // position
                    Utils::Maths::Vector3 bottomLeftN = Utils::Maths::Vector3::Normalize(bottomLeftP - minorCircleCentreLeft); // normal

                    // Now add the verts to the verts list
                    positions[currentIndex] = topLeftP;
                    positions[currentIndex + 1] = topRightP;
                    positions[currentIndex + 2] = bottomRightP;
                    positions[currentIndex + 3] = bottomLeftP;

                    normals[currentIndex] = topLeftN;
                    normals[currentIndex + 1] = topRightN;
                    normals[currentIndex + 2] = bottomRightN;
                    normals[currentIndex + 3] = bottomLeftN;

                    // Write indices
                    // Tri 1
                    indices[currentIndexCount++] = currentIndex;
                    indices[currentIndexCount++] = currentIndex + 1;
                    indices[currentIndexCount++] = currentIndex + 2;

                    // Tri 2
                    indices[currentIndexCount++] = currentIndex + 2;
                    indices[currentIndexCount++] = currentIndex + 3;
                    indices[currentIndexCount++] = currentIndex;

                    // increment angle
                    externalAngle += angleDelta;
                }
                internalAngle += angleDelta;
            }

            // Now generate mesh from vertex data
            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->SetPositions(positions);
            mesh->SetNormals(normals);
            mesh->SetIndices(indices);
            mesh->FinaliseMesh(device);
            return mesh;
        }

        Mesh::Ptr MeshMaker::CreateGroundPlane(ID3D11Device* device)
        {
            // Not yet implemented.
            EngineAssert(false);
            return nullptr;
        }

        Mesh::Ptr MeshMaker::CreateTesselatedGroundPlane(ID3D11Device* device, uint32_t width, uint32_t height)
        {
            // Calculate the number of vertices in the terrain mesh.
            uint32_t vertexCount = (width - 1) * (height - 1) * 8;
            
            // Set the index count to the same as the vertex count.
            uint32_t indexCount = vertexCount;

            Mesh::PositionContainer positions(vertexCount);
            Mesh::NormalContainer normals(vertexCount);
            Mesh::UVContainer uvs(vertexCount);
            Mesh::IndexContainer indices(indexCount);
            
            // Load the vertex and index arrays with the terrain data.
            float positionX, positionZ;
            uint32_t index = 0;
            for(uint32_t j=0; j<(height-1); j++)
            {
                for(uint32_t i=0; i<(width-1); i++)
                {
                    // Upper left.
                    positionX = (float)i;
                    positionZ = (float)(j);
            
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    // Upper right.
                    positionX = (float)(i);
                    positionZ = (float)(j+1);
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    // lower left
                    positionX = (float)(i+1);
                    positionZ = (float)(j);
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    // Bottom right.  lower left
                    positionX = (float)(i);
                    positionZ = (float)(j+1);
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    // Bottom right.  upper right
                    positionX = (float)(i+1);
                    positionZ = (float)(j+1);
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    // Bottom rigjht.
                    positionX = (float)(i+1);
                    positionZ = (float)(j);
            
                    positions[index] = Utils::Maths::Vector3(positionX, 0.0f, positionZ);
                    normals[index] = Utils::Maths::Vector3(0.0, 1.0, 0.0);
                    uvs[index] = Utils::Maths::Vector2(0.0, 0.0);
                    indices[index] = index;
                    index++;
            
                    
                }
            }
            
            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->SetPositions(positions);
            mesh->SetNormals(normals);
            mesh->SetUVs(uvs);
            mesh->SetIndices(indices);
            mesh->FinaliseMesh(device);
            return mesh;
        }

        Mesh::Ptr MeshMaker::CreateFullScreenQuad(ID3D11Device* device)
        {
            const uint32_t vertexCount = 4;
            const uint32_t indexCount = 6;

            Mesh::PositionContainer positions(vertexCount);
            Mesh::UVContainer uvs(vertexCount);
            Mesh::IndexContainer indices(indexCount);

            // Positions for out unit quad centred at the origin
            // aligned along the z axis
            positions[0] = { -1.0f, 1.0f, 0.0f }; // Top left
            positions[1] = { 1.0f, 1.0f, 0.0f }; // Top right
            positions[2] = { 1.0f, -1.0f, 0.0f }; // Bottom right
            positions[3] = { -1.0f, -1.0f, 0.0f }; // Bottom left

            // Uv coordinates for the quad
            uvs[0] = { 0.0f, 0.0f }; // Top left
            uvs[1] = { 1.0f, 0.0f }; // Top right
            uvs[2] = { 1.0f, 1.0f }; // Bottom right
            uvs[3] = { 0.0f, 1.0f }; // Bottom left

            // Indices drawn clockwise
            indices[0] = 0;
            indices[1] = 1;
            indices[2] = 2;

            indices[3] = 2;
            indices[4] = 3;
            indices[5] = 0;

            Mesh::Ptr mesh = std::make_shared<Mesh>();
            mesh->SetPositions(positions);
            mesh->SetUVs(uvs);
            mesh->SetIndices(indices);
            mesh->FinaliseMesh(device);

            return mesh;
        }
}