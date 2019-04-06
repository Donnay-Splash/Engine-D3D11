//#pragma once
//
//
//// DEPRECATED FOR D3D12 // 
//namespace Engine
//{
//    // forward declaration
//    class Shader;
//    class ShaderPipeline;
//
//    class InputLayout
//    {
//    public:
//        using Ptr = std::shared_ptr<InputLayout>;
//        InputLayout(const uint32_t& inputFlags);
//
//    private:
//        void UploadData(ID3D11DeviceContext* deviceContext);
//
//    private:
//        Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
//        std::vector<D3D12_INPUT_ELEMENT_DESC> m_inputElements;
//
//        friend class Shader;
//        friend class ShaderPipeline;
//    };
//}