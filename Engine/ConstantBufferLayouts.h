#pragma once
#include "Math.h"

_declspec(align(16))
struct ViewConstants
{
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

_declspec(align(16))
struct ObjectConstants
{
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

_declspec(align(16))
struct TimeBuffer
{
    float time;
    float height;
    float length;
    float padding;

    bool operator!=(const TimeBuffer& rhs)
    {
        return  (this->time != rhs.time) ||
            (this->height != rhs.height) ||
            (this->length != rhs.length);
    }

    bool operator==(const TimeBuffer& rhs)
    {
        return  (this->time == rhs.time) &&
            (this->height == rhs.height) &&
            (this->length == rhs.length);
    }
};

_declspec(align(16))
struct LightBuffer
{
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