#pragma once
#include "Math.h"
#include <Scene\Components\Light.h>
#define ALIGN_16 _declspec(align(16))

namespace Engine
{
    static const uint32_t kMaxLightCount = 4;

    ALIGN_16
        struct ViewConstants
    {
        static const UINT kRegister = 0;
        Utils::Maths::Matrix view;
        Utils::Maths::Matrix projection;
        Utils::Maths::Vector3 cameraPosition;

        bool operator!=(const ViewConstants& rhs)
        {
            return  (view != rhs.view) ||
                (projection != rhs.projection) ||
                (cameraPosition != rhs.cameraPosition);
        }

        bool operator==(const ViewConstants& rhs)
        {
            return  (view == rhs.view) &&
                (projection == rhs.projection) &&
                (cameraPosition == rhs.cameraPosition);
        }
    };

    ALIGN_16
        struct ObjectConstants
    {
        static const UINT kRegister = 1;
        Utils::Maths::Matrix objectTransform;

        bool operator!=(const ObjectConstants& rhs)
        {
            return  (this->objectTransform != rhs.objectTransform);
        }

        bool operator==(const ObjectConstants& rhs)
        {
            return  (this->objectTransform == rhs.objectTransform);
        }
    };


    ALIGN_16
        struct LightConstants
    {
        static const UINT kRegister = 0;
        LightData lights[kMaxLightCount];
        float activeLights;

        bool operator!=(const LightConstants& rhs)
        {
            for (int i = 0; i < kMaxLightCount; i++)
            {
                if (this->lights[i] != rhs.lights[i])
                {
                    return true;
                }
            }

            return activeLights != rhs.activeLights;
        }

        bool operator==(const LightConstants& rhs)
        {
            bool result = true;
            for (int i = 0; i < kMaxLightCount; i++)
            {
                result &= (lights[i] == rhs.lights[i]);
                if (!result)
                {
                    break;
                }
            }

            return result && (activeLights == rhs.activeLights);
        }
    };

    ALIGN_16
        struct MaterialConstants
    {
        static const UINT kRegister = 1;

        Utils::Maths::Color diffuseColorAndOpacity;
        Utils::Maths::Color specularColorAndSmoothness;
        Utils::Maths::Color emissiveColor;
        float hasDiffuseTexture = 0.0f;
        float hasSpecularTexture = 0.0f;
        float hasEmissiveTexture = 0.0f;
        float hasNormalTexture = 0.0f;
        float hasSmoothnessTexture = 0.0f;
        float hasOpacityTexture = 0.0f;
        float hasAOTexture = 0.0f;

        bool operator!=(const MaterialConstants& rhs)
        {
            return (diffuseColorAndOpacity != rhs.diffuseColorAndOpacity) ||
                (specularColorAndSmoothness != rhs.specularColorAndSmoothness) ||
                (emissiveColor != rhs.emissiveColor) ||
                (hasDiffuseTexture != rhs.hasDiffuseTexture) ||
                (hasSpecularTexture != rhs.hasSpecularTexture) ||
                (hasEmissiveTexture != rhs.hasEmissiveTexture) ||
                (hasNormalTexture != rhs.hasNormalTexture) ||
                (hasSmoothnessTexture != rhs.hasSmoothnessTexture) ||
                (hasOpacityTexture != rhs.hasOpacityTexture) ||
                (hasAOTexture != rhs.hasAOTexture);
        }

        bool operator==(const MaterialConstants& rhs)
        {
            return (diffuseColorAndOpacity == rhs.diffuseColorAndOpacity) &&
                (specularColorAndSmoothness == rhs.specularColorAndSmoothness) &&
                (emissiveColor == rhs.emissiveColor) &&
                (hasDiffuseTexture == rhs.hasDiffuseTexture) &&
                (hasSpecularTexture == rhs.hasSpecularTexture) &&
                (hasEmissiveTexture == rhs.hasEmissiveTexture) &&
                (hasNormalTexture == rhs.hasNormalTexture) &&
                (hasSmoothnessTexture == rhs.hasSmoothnessTexture) &&
                (hasOpacityTexture == rhs.hasOpacityTexture) &&
                (hasAOTexture == rhs.hasAOTexture);
        }
    };
}