#pragma once
#include "Math.h"
#define ALIGN_16 _declspec(align(16))

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
struct LightBuffer
{
    static const UINT kRegister = 0;
    Utils::Maths::Color ambientColor;
    Utils::Maths::Color diffuseColor;
    Utils::Maths::Vector3 lightDirection;
    float padding;

    bool operator!=(const LightBuffer& rhs)
    {
        return  (this->ambientColor != rhs.ambientColor) ||
            (this->diffuseColor != rhs.diffuseColor) ||
            (this->lightDirection != rhs.lightDirection);
    }

    bool operator==(const LightBuffer& rhs)
    {
        return  (this->ambientColor == rhs.ambientColor) &&
            (this->diffuseColor == rhs.diffuseColor) &&
            (this->lightDirection == rhs.lightDirection);
    }
};