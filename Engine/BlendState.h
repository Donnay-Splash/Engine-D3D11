#pragma once
#include "pch.h"

enum class BlendMode
{
    // Alpha blending is disabled
    Opaque = 0,
    // Alpha blending is enabled, straight post multiplied alpha blending.
    Transparent = 1
    // Need to add pre multiplied alpha blending.
};

class BlendState
{
public:
    using Ptr = std::shared_ptr<BlendState>;
    BlendState(BlendMode initMode = BlendMode::Opaque);
    ~BlendState() {};

    void SetBlendMode(BlendMode mode);

private:
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;
};