#pragma once


namespace Engine
{
    class RasterizerState
    {
    public:
        using Ptr = std::shared_ptr<RasterizerState>;
        RasterizerState(ID3D11Device* device, D3D11_CULL_MODE cullMode = D3D11_CULL_BACK);

        void UploadData(ID3D11DeviceContext* deviceContext);

    private:
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;
    };
}