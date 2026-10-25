#include "Math.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace Engine {
namespace Maths {
namespace {

float length(Vector3 value)
{
    return std::sqrt(value.x * value.x + value.y * value.y + value.z * value.z);
}

} // namespace

Vector2::Vector2() : x(0.0f), y(0.0f)
{
}

Vector2::Vector2(float xValue, float yValue) : x(xValue), y(yValue)
{
}

Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f)
{
}

Vector3::Vector3(float xValue, float yValue, float zValue)
    : x(xValue), y(yValue), z(zValue)
{
}

Vector4::Vector4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f)
{
}

Vector4::Vector4(float xValue, float yValue, float zValue, float wValue)
    : x(xValue), y(yValue), z(zValue), w(wValue)
{
}

Quaternion::Quaternion() : x(0.0f), y(0.0f), z(0.0f), w(1.0f)
{
}

Quaternion::Quaternion(float xValue, float yValue, float zValue, float wValue)
    : x(xValue), y(yValue), z(zValue), w(wValue)
{
}

Quaternion Quaternion::axisAngle(Vector3 axis, float radians)
{
    const float axisLength = length(axis);
    if (axisLength == 0.0f) {
        throw std::invalid_argument("A quaternion axis must be non-zero.");
    }
    const float halfAngle = radians * 0.5f;
    const float scale = std::sin(halfAngle) / axisLength;
    return Quaternion(
        axis.x * scale,
        axis.y * scale,
        axis.z * scale,
        std::cos(halfAngle));
}

Matrix4::Matrix4()
{
    for (unsigned int row = 0; row < 4; ++row) {
        for (unsigned int column = 0; column < 4; ++column) {
            elements[row * 4 + column] = row == column ? 1.0f : 0.0f;
        }
    }
}

Matrix4 Matrix4::identity()
{
    Matrix4 matrix;
    for (unsigned int row = 0; row < 4; ++row) {
        for (unsigned int column = 0; column < 4; ++column) {
            matrix.elements[row * 4 + column] = row == column ? 1.0f : 0.0f;
        }
    }
    return matrix;
}

Matrix4 Matrix4::translation(Vector3 offset)
{
    Matrix4 matrix = identity();
    matrix.at(3, 0) = offset.x;
    matrix.at(3, 1) = offset.y;
    matrix.at(3, 2) = offset.z;
    return matrix;
}

Matrix4 Matrix4::scaling(Vector3 scale)
{
    Matrix4 matrix = identity();
    matrix.at(0, 0) = scale.x;
    matrix.at(1, 1) = scale.y;
    matrix.at(2, 2) = scale.z;
    return matrix;
}

Matrix4 Matrix4::rotation(Quaternion value)
{
    const float quaternionLength = std::sqrt(
        value.x * value.x + value.y * value.y + value.z * value.z + value.w * value.w);
    if (quaternionLength == 0.0f) {
        throw std::invalid_argument("A rotation quaternion must be non-zero.");
    }

    const float x = value.x / quaternionLength;
    const float y = value.y / quaternionLength;
    const float z = value.z / quaternionLength;
    const float w = value.w / quaternionLength;
    Matrix4 matrix = identity();
    matrix.at(0, 0) = 1.0f - 2.0f * (y * y + z * z);
    matrix.at(0, 1) = 2.0f * (x * y + z * w);
    matrix.at(0, 2) = 2.0f * (x * z - y * w);
    matrix.at(1, 0) = 2.0f * (x * y - z * w);
    matrix.at(1, 1) = 1.0f - 2.0f * (x * x + z * z);
    matrix.at(1, 2) = 2.0f * (y * z + x * w);
    matrix.at(2, 0) = 2.0f * (x * z + y * w);
    matrix.at(2, 1) = 2.0f * (y * z - x * w);
    matrix.at(2, 2) = 1.0f - 2.0f * (x * x + y * y);
    return matrix;
}

Matrix4 Matrix4::transform(Vector3 offset, Quaternion rotationValue, float uniformScale)
{
    return scaling(Vector3(uniformScale, uniformScale, uniformScale))
        * rotation(rotationValue)
        * translation(offset);
}

Matrix4 Matrix4::perspectiveLeftHanded(
    float verticalFieldOfViewRadians,
    float widthOverHeight,
    float nearClip,
    float farClip)
{
    if (verticalFieldOfViewRadians <= 0.0f
        || verticalFieldOfViewRadians >= 3.14159265358979323846f
        || widthOverHeight <= 0.0f
        || nearClip <= 0.0f
        || farClip <= nearClip) {
        throw std::invalid_argument("Perspective projection parameters are invalid.");
    }

    const float yScale = 1.0f / std::tan(verticalFieldOfViewRadians * 0.5f);
    const float xScale = yScale / widthOverHeight;
    Matrix4 matrix;
    for (unsigned int index = 0; index < 16; ++index) {
        matrix.elements[index] = 0.0f;
    }
    matrix.at(0, 0) = xScale;
    matrix.at(1, 1) = yScale;
    matrix.at(2, 2) = farClip / (farClip - nearClip);
    matrix.at(2, 3) = 1.0f;
    matrix.at(3, 2) = -nearClip * farClip / (farClip - nearClip);
    return matrix;
}

float &Matrix4::at(unsigned int row, unsigned int column)
{
    if (row >= 4 || column >= 4) {
        throw std::out_of_range("Matrix indices must be in the range [0, 4).");
    }
    return elements[row * 4 + column];
}

float Matrix4::at(unsigned int row, unsigned int column) const
{
    if (row >= 4 || column >= 4) {
        throw std::out_of_range("Matrix indices must be in the range [0, 4).");
    }
    return elements[row * 4 + column];
}

Vector4 Matrix4::transform(Vector4 value) const
{
    return Vector4(
        value.x * at(0, 0) + value.y * at(1, 0) + value.z * at(2, 0) + value.w * at(3, 0),
        value.x * at(0, 1) + value.y * at(1, 1) + value.z * at(2, 1) + value.w * at(3, 1),
        value.x * at(0, 2) + value.y * at(1, 2) + value.z * at(2, 2) + value.w * at(3, 2),
        value.x * at(0, 3) + value.y * at(1, 3) + value.z * at(2, 3) + value.w * at(3, 3));
}

Matrix4 operator*(const Matrix4 &lhs, const Matrix4 &rhs)
{
    Matrix4 result;
    for (unsigned int row = 0; row < 4; ++row) {
        for (unsigned int column = 0; column < 4; ++column) {
            result.at(row, column) = 0.0f;
            for (unsigned int index = 0; index < 4; ++index) {
                result.at(row, column) += lhs.at(row, index) * rhs.at(index, column);
            }
        }
    }
    return result;
}

Bounds::Bounds()
    : minimum_(
          std::numeric_limits<float>::max(),
          std::numeric_limits<float>::max(),
          std::numeric_limits<float>::max()),
      maximum_(
          -std::numeric_limits<float>::max(),
          -std::numeric_limits<float>::max(),
          -std::numeric_limits<float>::max()),
      empty_(true)
{
}

bool Bounds::empty() const
{
    return empty_;
}

Vector3 Bounds::minimum() const
{
    return minimum_;
}

Vector3 Bounds::maximum() const
{
    return maximum_;
}

Vector3 Bounds::center() const
{
    if (empty_) {
        return Vector3();
    }
    return Vector3(
        (minimum_.x + maximum_.x) * 0.5f,
        (minimum_.y + maximum_.y) * 0.5f,
        (minimum_.z + maximum_.z) * 0.5f);
}

Vector3 Bounds::extents() const
{
    if (empty_) {
        return Vector3();
    }
    return Vector3(
        (maximum_.x - minimum_.x) * 0.5f,
        (maximum_.y - minimum_.y) * 0.5f,
        (maximum_.z - minimum_.z) * 0.5f);
}

void Bounds::include(Vector3 point)
{
    if (empty_) {
        minimum_ = maximum_ = point;
        empty_ = false;
        return;
    }

    if (point.x < minimum_.x) minimum_.x = point.x;
    if (point.y < minimum_.y) minimum_.y = point.y;
    if (point.z < minimum_.z) minimum_.z = point.z;
    if (point.x > maximum_.x) maximum_.x = point.x;
    if (point.y > maximum_.y) maximum_.y = point.y;
    if (point.z > maximum_.z) maximum_.z = point.z;
}

Bounds Bounds::transformed(const Matrix4 &matrix) const
{
    Bounds result;
    if (empty_) {
        return result;
    }

    for (unsigned int corner = 0; corner < 8; ++corner) {
        const Vector4 point(
            (corner & 1) ? maximum_.x : minimum_.x,
            (corner & 2) ? maximum_.y : minimum_.y,
            (corner & 4) ? maximum_.z : minimum_.z,
            1.0f);
        const Vector4 transformedPoint = matrix.transform(point);
        if (transformedPoint.w == 0.0f) {
            continue;
        }
        result.include(Vector3(
            transformedPoint.x / transformedPoint.w,
            transformedPoint.y / transformedPoint.w,
            transformedPoint.z / transformedPoint.w));
    }
    return result;
}

} // namespace Maths
} // namespace engine
