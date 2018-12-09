#pragma once

namespace Engine
{
    class DepthState
    {
    public:
        using Ptr = std::shared_ptr<DepthState>;
        DepthState(bool depthEnabled);

        void UploadData(ID3D11DeviceContext* deviceContext);

    private:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;
    };
}