#pragma once
#include <Resources\Texture.h>
#include <Resources\VertexBuffer.h>
#include <Resources\IndexBuffer.h>
#include <Resources\ShaderPipeline.h>
#include <Resources\ConstantBuffer.h>


namespace Engine
{
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
        std::vector<VertexBuffer::Ptr> m_VertexBuffers;
        std::vector<IndexBuffer::Ptr> m_IndexBuffers;
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