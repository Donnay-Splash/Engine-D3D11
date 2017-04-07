#include "stdafx.h"
#include "MathHelpers.h"

namespace Utils
{
    namespace MathHelpers
    {
        std::vector<float> GenerateHaltonSequence(uint32_t count, uint32_t base)
        {
            float basef = float(base);
            std::vector<float> sequence;
            for (uint32_t index = 1; index <= count; index++)
            {
                float f = 1.0f;
                double result = 0.0f;
                auto i = index;
                while (i > 0)
                {
                    f /= basef;
                    result += f * (fmod(i, basef));
                    i /= base;
                }
                sequence.push_back(result);
            }
            return sequence;
        }

        bool IsPowerOfTwo(int val)
        {
            return (val != 0) && ((val & (val - 1)) == 0);
        }
    }
}