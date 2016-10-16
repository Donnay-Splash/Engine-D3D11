#pragma once
#include <sstream>
#include <iomanip>

namespace Utils
{
    namespace StringHelpers
    {
        // float specializations
        template<>
        inline std::string ToStringWithPrecision(float value, uint32_t precision, bool removeTrailingZeroes)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(precision) << value;
            auto result = out.str();
            if (removeTrailingZeroes)
            {
                result.erase(result.find_last_not_of('0') + 1);
                if (result.back() == '.')
                {
                    result.erase(result.size() - 1);
                }
            }
            return result;
        }

        template<>
        inline std::wstring ToWideStringWithPrecision(float value, uint32_t precision, bool removeTrailingZeroes)
        {
            std::wostringstream out;
            out << std::fixed << std::setprecision(precision) << value;
            auto result = out.str();
            if (removeTrailingZeroes)
            {
                result.erase(result.find_last_not_of('0') + 1);
                if (result.back() == '.')
                {
                    result.erase(result.size() - 1);
                }
            }
            return result;
        }

        // double specializations
        template<>
        inline std::string ToStringWithPrecision(double value, uint32_t precision, bool removeTrailingZeroes)
        {
            std::ostringstream out;
            out << std::fixed << std::setprecision(precision) << value;
            auto result = out.str();
            if (removeTrailingZeroes)
            {
                result.erase(result.find_last_not_of('0') + 1);
                if (result.back() == '.')
                {
                    result.erase(result.size() - 1);
                }
            }
            return result;
        }

        template<>
        inline std::wstring ToWideStringWithPrecision(double value, uint32_t precision, bool removeTrailingZeroes)
        {
            std::wostringstream out;
            out << std::fixed << std::setprecision(precision) << value;
            auto result = out.str();
            if (removeTrailingZeroes)
            {
                result.erase(result.find_last_not_of('0') + 1);
                if (result.back() == '.')
                {
                    result.erase(result.size() - 1);
                }
            }
            return result;
        }
    }
}