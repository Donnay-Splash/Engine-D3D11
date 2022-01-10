#include "pch.h"
#include "ImGuiRenderer.h"
#include "imgui.h"

namespace Engine
{
    ImGuiDrawData::ImGuiDrawData(int vertexCount, int indexCount)
    {
        m_PositionBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount, sizeof(ImVec2));
        m_UVBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount, sizeof(ImVec2));
        m_ColorBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount, sizeof(Utils::Maths::Color));

        m_IndexBuffer = std::make_shared<IndexBuffer>(nullptr, indexCount, false);

        m_VertexCount = vertexCount;
        m_IndexCount = indexCount;
    }

    void ImGuiDrawData::ResizeBuffers(uint64_t vertexCount, uint32_t indexCount)
    {
        if (vertexCount > m_VertexCount)
        {
            m_PositionBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount + 5000, sizeof(ImVec2));
            m_UVBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount + 5000, sizeof(ImVec2));
            m_ColorBuffer = std::make_shared<VertexBuffer>(nullptr, vertexCount + 5000, sizeof(Utils::Maths::Color));

            m_VertexCount = vertexCount + 5000;
        }

        if (indexCount > m_IndexCount)
        {
            m_IndexBuffer = std::make_shared<IndexBuffer>(nullptr, indexCount + 5000, false);

            m_IndexCount = indexCount + 5000;
        }
    }

    void ImGuiDrawData::SetVertexData(std::vector<ImDrawVert> verts)
    {
        std::vector<ImVec2> positions(verts.size());
        std::vector<ImVec2> uvs(verts.size());
        std::vector<Utils::Maths::Color> colors(verts.size());
        for (size_t idx = 0; idx < verts.size(); idx++)
        {
            positions[idx] = verts[idx].pos;
            uvs[idx] = verts[idx].uv;
            colors[idx] = Utils::Maths::Color::FromARGB(verts[idx].col);
        }

        m_PositionBuffer->SetNewData(positions.data(), positions.size() * sizeof(ImVec2));
        m_UVBuffer->SetNewData(uvs.data(), uvs.size() * sizeof(ImVec2));
        m_ColorBuffer->SetNewData(colors.data(), colors.size() * sizeof(Utils::Maths::Color));
    }

    void ImGuiDrawData::SetIndexData(std::vector<unsigned short> indices)
    {
        m_IndexBuffer->SetNewData(indices.data(), indices.size());
    }

    void ImGuiDrawData::UploadData(ID3D12GraphicsCommandList* cmdlist) const
    {
        m_PositionBuffer->UploadData(cmdlist, 0);
        m_UVBuffer->UploadData(cmdlist, 1);
        m_ColorBuffer->UploadData(cmdlist, 2);

        m_IndexBuffer->UploadData(cmdlist);
    }

    ImGuiRenderer::ImGuiRenderer()
    {

    }

    ImGuiRenderer::~ImGuiRenderer()
    {

    }

    void ImGuiRenderer::Init(int width, int height, ShaderPipeline::Ptr shaderPipeline, int frameCount)
    {
        m_ShaderPipeline = shaderPipeline;

        m_DrawBuffers.clear();
        m_CurrentFrameIndex = 0;
        m_FrameCount = frameCount;

        // Init buffers
        for (int i = 0; i < m_FrameCount; ++i)
        {
            m_DrawBuffers.emplace_back(c_DefaultBufferSize, c_DefaultBufferSize);
        }

        ImGuiIO& io = ImGui::GetIO();
        // Setup ImGui render data
        io.BackendRendererName = "imgui_dx12";
        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;  // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
        io.DisplaySize = ImVec2((float)width, (float)height);

        // Load Font
        unsigned char* pixels;
        int fondWidth, fontHeight;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &fondWidth, &fontHeight);
        m_FontTexture = Texture::CreateTexture(pixels, fondWidth, fontHeight, TextureCreationFlags::BindShaderResource, DXGI_FORMAT_R8G8B8A8_UNORM);
        io.Fonts->TexID = (ImTextureID)m_FontTexture->GetGPUHandle().ptr;
    }

    void ImGuiRenderer::BeginFrame()
    {
        m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % m_FrameCount;

        // Kick off new ImGui frame just before updating scene
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        // As a test show the demo window
        ImGui::ShowDemoWindow();
    }

    void ImGuiRenderer::EndFrame()
    {

    }

    void ImGuiRenderer::Render(ID3D12GraphicsCommandList* commandList)
    {
        ImGui::Render();
        ImDrawData* drawData = ImGui::GetDrawData();
        // Only frame for valid display sizes
        if (drawData->DisplaySize.x > 0.0f && drawData->DisplaySize.y > 0.0f)
        {
            Utils::Maths::Matrix projectionMatrix = Utils::Maths::Matrix::CreateOrthographicProjectionMatrix(0, drawData->DisplaySize.x, drawData->DisplaySize.y, 0, 0, 1.0f);

            uint64_t vertexCount = drawData->TotalVtxCount;
            uint32_t indexCount = drawData->TotalIdxCount;

            ImGuiDrawData currentFrameData = m_DrawBuffers[m_CurrentFrameIndex];
            currentFrameData.ResizeBuffers(vertexCount, indexCount);

            std::vector<ImDrawVert> imVerts(drawData->TotalVtxCount);
            std::vector<ImDrawIdx> imIndices(drawData->TotalIdxCount);

            ImDrawVert* vertDst = imVerts.data();
            ImDrawIdx* indxDst = imIndices.data();

            for (int n = 0; n < drawData->CmdListsCount; n++)
            {
                const ImDrawList* cmdList = drawData->CmdLists[n];
                memcpy(vertDst, cmdList->VtxBuffer.Data, cmdList->VtxBuffer.size() * sizeof(ImDrawVert));
                memcpy(indxDst, cmdList->IdxBuffer.Data, cmdList->IdxBuffer.size() * sizeof(ImDrawIdx));
                vertDst += cmdList->VtxBuffer.size();
                indxDst += cmdList->IdxBuffer.size();
            }

            currentFrameData.SetVertexData(imVerts);
            currentFrameData.SetIndexData(imIndices);

            currentFrameData.UploadData(commandList);

            // Setup render state. We might as well just create a material for UI rendering
            m_ShaderPipeline->UploadData(commandList);

            // Run through cmdlist and draw our UI
            int vertexOffset = 0;
            int indexOffset = 0;
            ImVec2 displayOffset = drawData->DisplayPos;
            for (int n = 0; n < drawData->CmdListsCount; n++)
            {
                const ImDrawList* cmdList = drawData->CmdLists[n];
                for (int commandIndex = 0; commandIndex < cmdList->CmdBuffer.size(); commandIndex++)
                {
                    const ImDrawCmd& cmd = cmdList->CmdBuffer[commandIndex];
                    if (cmd.UserCallback != nullptr)
                    {
                        // User callback, registered via ImDrawList::AddCallback()
                        // (ImDrawCallback_ResetRenderState is a special callback value used by the user to request the renderer to reset render state.)
                        if (cmd.UserCallback == ImDrawCallback_ResetRenderState)
                        {
                            //ImGui_ImplDX12_SetupRenderState(draw_data, ctx, fr);
                            // Reset render state
                        }
                        else
                        {
                            cmd.UserCallback(cmdList, &cmd);
                        }
                    }
                    else
                    {
                        const D3D12_RECT r = { static_cast<LONG>(cmd.ClipRect.x - displayOffset.x),
                                               static_cast<LONG>(cmd.ClipRect.y - displayOffset.y),
                                               static_cast<LONG>(cmd.ClipRect.z - displayOffset.x),
                                               static_cast<LONG>(cmd.ClipRect.w - displayOffset.y) };
                        // TODO: If we want to use a different texture ID we could override it here
                        commandList->SetGraphicsRoot32BitConstants(0, 16, &projectionMatrix, 0);
                        commandList->SetGraphicsRootDescriptorTable(1, *(D3D12_GPU_DESCRIPTOR_HANDLE*)&cmd.TextureId);
                        commandList->RSSetScissorRects(1, &r);
                        commandList->DrawIndexedInstanced(cmd.ElemCount, 1, cmd.IdxOffset + indexOffset, cmd.VtxOffset + vertexOffset, 0);
                    }
                }
                vertexOffset += cmdList->VtxBuffer.size();
                indexOffset += cmdList->IdxBuffer.size();
            }
        }
    }
}