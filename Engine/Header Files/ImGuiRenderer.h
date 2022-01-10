#pragma once
#include <Resources\Texture.h>
#include <Resources\VertexBuffer.h>
#include <Resources\IndexBuffer.h>
#include <Resources\ShaderPipeline.h>
#include <Resources\ConstantBuffer.h>

struct ImDrawVert;

namespace Engine
{
    class ImGuiDrawData
    {
    public:
        ImGuiDrawData(int vertexCount, int indexCount);
        void ResizeBuffers(uint64_t vertexCount, uint32_t indexCount);
        void SetVertexData(std::vector<ImDrawVert> verts);
        void SetIndexData(std::vector<unsigned short> indices);
        void UploadData(ID3D12GraphicsCommandList* cmdlist) const;

    private:
        VertexBuffer::Ptr m_PositionBuffer;
        VertexBuffer::Ptr m_UVBuffer;
        VertexBuffer::Ptr m_ColorBuffer;

        IndexBuffer::Ptr m_IndexBuffer;

        uint64_t m_VertexCount;
        uint32_t m_IndexCount;
    };

    class ImGuiRenderer
    {
    public:
        ImGuiRenderer();
        ~ImGuiRenderer();

        void Init(int width, int height, ShaderPipeline::Ptr shaderPipeline, int frameCount);

        void BeginFrame();
        void EndFrame();

        void Render(ID3D12GraphicsCommandList* commandList);


        using Ptr = std::shared_ptr<ImGuiRenderer>;
    private:
        // Here we can store the vertex buffers 
        std::vector<ImGuiDrawData> m_DrawBuffers;
        Texture::Ptr m_FontTexture;
        ShaderPipeline::Ptr m_ShaderPipeline;

        int m_FrameCount;
        int m_CurrentFrameIndex;

        struct ImGuiConstants
        {
            Utils::Maths::Matrix m_ViewProjMatrix;
        };
        ConstantBuffer<ImGuiConstants>::Ptr m_Constants;

        const int c_DefaultBufferSize = 5000;
    };
}