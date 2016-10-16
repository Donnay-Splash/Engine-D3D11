#pragma once
#include <string>

namespace Utils
{
    namespace StringHelpers
    {
        // Primary templates
        template<class T>
        std::string ToStringWithPrecision(T value, uint32_t precision = 5, bool removeTrailingZeroes = false);

        template<class T>
        std::wstring ToWideStringWithPrecision(T value, uint32_t precision = 5, bool removeTrailingZeroes = false);

        // float specializations
        template<>
        static std::string ToStringWithPrecision(float value, uint32_t precision, bool removeTrailingZeroes);

        template<>
        static std::wstring ToWideStringWithPrecision(float value, uint32_t precision, bool removeTrailingZeroes);

        // double specializations
        template<>
        static std::string ToStringWithPrecision(double value, uint32_t precision, bool removeTrailingZeroes);

        template<>
        static std::wstring ToWideStringWithPrecision(double value, uint32_t precision, bool removeTrailingZeroes);
    }
}

#include "StringHelpers.inl"