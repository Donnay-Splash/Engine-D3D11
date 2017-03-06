#pragma once
#include <vector>

namespace Utils
{
    namespace MathHelpers
    {
        std::vector<float> GenerateHaltonSequence(uint32_t count, uint32_t base);
    }
}