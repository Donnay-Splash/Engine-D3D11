#ifndef ENGINE_CORE_MATH_H
#define ENGINE_CORE_MATH_H

namespace engine {
namespace core {

struct Vector2
{
    float x;
    float y;

    Vector2();
    Vector2(float xValue, float yValue);
};

struct Vector3
{
    float x;
    float y;
    float z;

    Vector3();
    Vector3(float xValue, float yValue, float zValue);
};

struct Vector4
{
    float x;
    float y;
    float z;
    float w;

    Vector4();
    Vector4(float xValue, float yValue, float zValue, float wValue);
};

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;

    Quaternion();
    Quaternion(float xValue, float yValue, float zValue, float wValue);

    static Quaternion axisAngle(Vector3 axis, float radians);
};

struct Matrix4
{
    // Row-major storage and row-vector multiplication match the migrated legacy math.
    // Perspective matrices are left-handed and map depth to [0, 1].
    float elements[16];

    Matrix4();

    static Matrix4 identity();
    static Matrix4 translation(Vector3 offset);
    static Matrix4 scaling(Vector3 scale);
    static Matrix4 rotation(Quaternion rotation);
    static Matrix4 transform(Vector3 translation, Quaternion rotation, float uniformScale);
    static Matrix4 perspectiveLeftHanded(
        float verticalFieldOfViewRadians,
        float widthOverHeight,
        float nearClip,
        float farClip);

    float &at(unsigned int row, unsigned int column);
    float at(unsigned int row, unsigned int column) const;
    Vector4 transform(Vector4 value) const;
};

Matrix4 operator*(const Matrix4 &lhs, const Matrix4 &rhs);

class Bounds
{
public:
    Bounds();

    bool empty() const;
    Vector3 minimum() const;
    Vector3 maximum() const;
    Vector3 center() const;
    Vector3 extents() const;
    void include(Vector3 point);
    Bounds transformed(const Matrix4 &matrix) const;

private:
    Vector3 minimum_;
    Vector3 maximum_;
    bool empty_;
};

} // namespace core
} // namespace engine

#endif
