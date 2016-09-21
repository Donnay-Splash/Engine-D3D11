#pragma once
#include "Math.h"
#include "Light.h"
#define ALIGN_16 _declspec(align(16))

static const uint32_t kMaxLightCount = 4;

ALIGN_16
struct ViewConstants
{
    static const UINT kRegister = 0;
    Utils::Maths::Matrix view;
    Utils::Maths::Matrix projection;

    bool operator!=(const ViewConstants& rhs)
    {
        return  (this->view != rhs.view) ||
                (this->projection != rhs.projection);
    }

    bool operator==(const ViewConstants& rhs)
    {
        return  (this->view == rhs.view) &&
                (this->projection == rhs.projection);
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
            if(this->lights[i] != rhs.lights[i])
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