#pragma once
#include "Math.h"
#include <Scene\Components\Light.h>
#define ALIGN_16 _declspec(align(16))

// TODO: Formalise buffer registers
// View constants are global so must not be overwritten
// effectively making register 0 system reserved
namespace Engine
{
    static const uint32_t kMaxLightCount = 4;
    static const uint32_t kTSAASampleCount = 9;

    ALIGN_16
        struct ViewConstants
    {
        static const UINT kRegister = 0;
        Utils::Maths::Matrix view;
        Utils::Maths::Matrix invView;
        Utils::Maths::Matrix prevInvView;
        Utils::Maths::Matrix projection;
        Utils::Maths::Matrix jitteredProjection;
        Utils::Maths::Matrix cameraToScreenMatrix;
        Utils::Maths::Vector4 projectionInfo;
        Utils::Maths::Vector3 cameraPosition;
        Utils::Maths::Vector3 clipInfo;
        Utils::Maths::Vector2 invViewSize;
        float projectionScale;

        bool operator!=(const ViewConstants& rhs)
        {
            return  (view != rhs.view) ||
                (invView != rhs.invView) ||
                (prevInvView != rhs.prevInvView) ||
                (projection != rhs.projection) ||
                (jitteredProjection != rhs.jitteredProjection) ||
                (cameraToScreenMatrix != rhs.cameraToScreenMatrix) ||
                (projectionInfo != rhs.projectionInfo) ||
                (cameraPosition != rhs.cameraPosition) ||
                (projectionScale != rhs.projectionScale) ||
                (clipInfo != rhs.clipInfo) ||
                (invViewSize != rhs.invViewSize);
        }

        bool operator==(const ViewConstants& rhs)
        {
            return  (view == rhs.view) &&
                (invView == rhs.invView) &&
                (prevInvView == rhs.prevInvView) &&
                (projection == rhs.projection) &&
                (jitteredProjection == rhs.jitteredProjection) &&
                (cameraToScreenMatrix == rhs.cameraToScreenMatrix) &&
                (projectionInfo == rhs.projectionInfo) &&
                (cameraPosition == rhs.cameraPosition) &&
                (projectionScale == rhs.projectionScale) &&
                (clipInfo == rhs.clipInfo) &&
                (invViewSize == rhs.invViewSize);
        }
    };

    ALIGN_16
        struct ObjectConstants
    {
        static const UINT kRegister = 1;
        Utils::Maths::Matrix objectToCameraTransform;
        Utils::Maths::Matrix prevObjectToCameraTransform;

        bool operator!=(const ObjectConstants& rhs)
        {
            return  (this->objectToCameraTransform != rhs.objectToCameraTransform) ||
                    (this->prevObjectToCameraTransform != rhs.prevObjectToCameraTransform);
        }

        bool operator==(const ObjectConstants& rhs)
        {
            return  (this->objectToCameraTransform == rhs.objectToCameraTransform) &&
                    (this->prevObjectToCameraTransform == rhs.prevObjectToCameraTransform);
        }
    };


    ALIGN_16
        struct LightConstants
    {
        static const UINT kRegister = 2;
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
        static const UINT kRegister = 3;

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

    ALIGN_16
        struct PostEffectConstants
    {
        static const UINT kRegister = 1;

        float displaySecondLayer = 0.0f;
        float gBufferTargetIndex = 0.0f;
        float basicTemporalFilterBlend = 0.0f;
        float blurEdgeSharpness = 1.0f; // Controls how sharp the edges are for the blur. A higher value increases sharpness but creates flickering at edges.
        
        float temporalBlendWeight = 1.0f; // Controls the contribution of the current frame to the final color. By default current frame has full contribution
        float sceneTime = 0.0f; // elapsed scene time in seconds
        float aoEnabled = 1.0f; // contols whether ao is used in final shading
        float radiosityEnabled = 1.0f; // Controls whether radiosity is used in final shading

        // Axis that we are blurring along. either [0, 1] for vertical or [1, 0] for horizontal
        Utils::Maths::Vector2 blurAxis = { 0.0f, 0.0f };

        bool operator!=(const PostEffectConstants& rhs)
        {
            return (displaySecondLayer != rhs.displaySecondLayer) ||
            (gBufferTargetIndex != rhs.gBufferTargetIndex) ||
            (basicTemporalFilterBlend != rhs.basicTemporalFilterBlend) ||
            (blurEdgeSharpness != rhs.blurEdgeSharpness) ||
            (temporalBlendWeight != rhs.temporalBlendWeight) ||
            (sceneTime != rhs.sceneTime) ||
            (aoEnabled != rhs.aoEnabled) ||
            (radiosityEnabled != rhs.radiosityEnabled) ||
            (blurAxis != rhs.blurAxis);
        }
        bool operator==(const PostEffectConstants& rhs)
        {
            return (displaySecondLayer == rhs.displaySecondLayer) &&
            (gBufferTargetIndex == rhs.gBufferTargetIndex) &&
            (basicTemporalFilterBlend == rhs.basicTemporalFilterBlend) &&
            (blurEdgeSharpness == rhs.blurEdgeSharpness) &&
            (temporalBlendWeight == rhs.temporalBlendWeight) &&
            (sceneTime == rhs.sceneTime) &&
            (aoEnabled == rhs.aoEnabled) &&
            (radiosityEnabled == rhs.radiosityEnabled) &&
            (blurAxis == rhs.blurAxis);
        }

    };

    ALIGN_16
        struct TemporalAAConstants
    {
        static const UINT kRegister = 4;

        float sampleWeights[kTSAASampleCount];
        float pad1;
        float pad2;
        float pad3;
        float currentFrameWeight = 0.04f;

        // We know that these constants will change each frame so 
        // the comparison operators pass through
        bool operator != (const TemporalAAConstants& rhs)
        {
            return true;
        }
        bool operator == (const TemporalAAConstants& rhs)
        {
            return false;
        }
    };

    ALIGN_16
        struct GIConstants
    {
        static const UINT kRegister = 5;

        float numSamples = 0.0f; // Total number of samples to take for AO
        float numSpiralTurns = 0.0f; // Number of spirals to take when sampling. Best value of turns calculated in engine based on selected number of samples
        float Radius = 0.0f; // Ambient occlusion sphere radius in world-space
        float aoBias = 0.0f; // AO Bias to avoid darkening in smooth corners e.g. 0.01m

        float aoIntensity = 0.0f; // Controls the sharpness of the calculated AO
        float aoUseSecondLayer = 1.0f;
        float radiosityPropogationDamping = 0.1f; // Controls how much affect the previous frames radiosity has on the current frame

        // We know that these constants will change each frame so 
        // the comparison operators pass through
        bool operator != (const GIConstants& rhs)
        {
            return  (numSamples != rhs.numSamples) ||
            (numSpiralTurns != rhs.numSpiralTurns) ||
            (Radius != rhs.Radius) ||
            (aoBias != rhs.aoBias) ||
            (aoIntensity != rhs.aoIntensity) ||
            (aoUseSecondLayer != rhs.aoUseSecondLayer);
        }
        bool operator == (const GIConstants& rhs)
        {
            return (numSamples == rhs.numSamples) &&
            (numSpiralTurns == rhs.numSpiralTurns) &&
            (Radius == rhs.Radius) &&
            (aoBias == rhs.aoBias) &&
            (aoIntensity == rhs.aoIntensity) &&
            (aoUseSecondLayer == rhs.aoUseSecondLayer);
        }
    };

    ALIGN_16
        struct DeepGBufferConstants
    {
        static const UINT kRegister = 2;

        // TODO: Decide on correct default value
        float minimumSeparation = 0.0f;

        // Currently empty but can add debug stuff or something later
        bool operator!=(const DeepGBufferConstants& rhs)
        {
            return minimumSeparation != rhs.minimumSeparation;
        }
        bool operator==(const DeepGBufferConstants& rhs)
        {
            return minimumSeparation == rhs.minimumSeparation;
        }

    };
}