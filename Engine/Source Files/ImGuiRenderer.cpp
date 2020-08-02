#include "pch.h"
#include "ImGuiRenderer.h"
#include "imgui.h"

namespace Engine
{
    ImGuiRenderer::ImGuiRenderer()
    {

    }

    ImGuiRenderer::~ImGuiRenderer()
    {

    }

    void ImGuiRenderer::Init(int width, int height, ShaderPipeline::Ptr shaderPipeline, int frameCount)
    {
        m_ShaderPipeline = shaderPipeline;

        m_VertexBuffers.resize(frameCount, nullptr);
        m_IndexBuffers.resize(frameCount, nullptr);
        m_CurrentFrameIndex = 0;
        m_FrameCount = frameCount;

        // Init buffers
        for (int i = 0; i < m_FrameCount; ++i)
        {
            m_VertexBuffers[i] = std::make_shared<VertexBuffer>(nullptr, c_DefaultBufferSize, sizeof(ImDrawVert));
            m_IndexBuffers[i] = std::make_shared<IndexBuffer>(nullptr, c_DefaultBufferSize, false);
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
            Utils::Maths::Matrix projectionMatrix = Utils::Maths::Matrix::CreateOrthographicProjectionMatrix(drawData->DisplaySize.x, drawData->DisplaySize.y, -1.0f, 1.0f);

            uint64_t vertexBufferSize = drawData->TotalVtxCount * sizeof(ImDrawVert);
            uint32_t indexCount = drawData->TotalIdxCount;

            VertexBuffer::Ptr currentFrameVerts = m_VertexBuffers[m_CurrentFrameIndex];
            IndexBuffer::Ptr currentFrameIndices = m_IndexBuffers[m_CurrentFrameIndex];

            if (vertexBufferSize > currentFrameVerts->GetBufferSize())
            {
                m_VertexBuffers[m_CurrentFrameIndex] = std::make_shared<VertexBuffer>(nullptr, drawData->TotalVtxCount + 5000, sizeof(ImDrawVert));
                currentFrameVerts = m_VertexBuffers[m_CurrentFrameIndex];
            }
            if (indexCount > currentFrameIndices->GetIndexCount())
            {
                m_IndexBuffers[m_CurrentFrameIndex] = std::make_shared<IndexBuffer>(nullptr, indexCount + 5000, false);
                currentFrameIndices = m_IndexBuffers[m_CurrentFrameIndex];
            }

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

            currentFrameVerts->SetNewData(imVerts.data(), imVerts.size() * sizeof(ImDrawVert));
            currentFrameIndices->SetNewData(imIndices.data(), (uint32_t)imIndices.size());

            currentFrameVerts->UploadData(commandList, 0);
            currentFrameIndices->UploadData(commandList);

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