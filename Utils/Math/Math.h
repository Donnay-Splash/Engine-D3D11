#pragma once
/*
Helper classes to wrap DirectXMath constructs
*/
#define NOMINMAX
#include <Math.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <tuple>
#include <algorithm>
#include <vector>

namespace Utils
{
    namespace Maths
    {
        // Constants
        static const float kPI = DirectX::XM_PI;

        // Forward declared classes
        class Matrix;
        class Vector4;
        class Vector3;
        class Vector2;
        class Quaternion;
        class Color;


        /****************************************************************************
        *
        * Matrix
        *
        ****************************************************************************/
        class Matrix : public DirectX::XMFLOAT4X4A
        {
        public:
            static const Matrix Identity;

            inline Matrix() : XMFLOAT4X4A(Matrix::Identity) {}
            Matrix(Vector4 r1, Vector4 r2, Vector4 r3, Vector4 r4);

            inline Matrix(float m00, float m01, float m02, float m03,
                float m10, float m11, float m12, float m13,
                float m20, float m21, float m22, float m23,
                float m30, float m31, float m32, float m33)
                :XMFLOAT4X4A(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33) {}

            // Create a diagonal matrix
            inline Matrix(float m00, float m11, float m22, float m33 = 1.0f) :
                XMFLOAT4X4A(m00, 0.0f, 0.0f, 0.0f, 0.0f, m11, 0.0f, 0.0f, 0.0f, 0.0f, m22, 0.0f, 0.0f, 0.0f, 0.0f, m33) {}

            ~Matrix() {}

            static Matrix CreateFromTranslation(Vector3 translation);
            static Matrix CreateFromRotation(Quaternion rotation);
            // only support uniform scale at the moment
            static Matrix CreateFromScale(float uniformScaleFactor);
            static Matrix CreateFromTranslationRotationScale(Vector3 translation, Quaternion rotation, float uniformScaleFactor);
            // Aspect ratio of viewspace x:y how many units in x per every y unit (x/y)
            static Matrix CreatePerspectiveProjectionMatrix(float fovY, float aspectHByW, float nearClip, float farClip);
            static Matrix CreateOrthographicProjectionMatrix(float viewWidth, float viewHeight, float nearClip, float farClip);

            Vector3 TransformNormal(const Vector3& normal) const;

            Matrix GetTranspose();
            Matrix GetInverse() const;

            // Extract orientation vectors from matrix.
            Vector3 Forward();
            Vector3 Up();
            Vector3 Right();
            Vector3 Translation();

            // Returns a tuple containing Translation, Rotation and Scale in that order.
            std::tuple<Vector3, Quaternion, Vector3> Decompose() const;

            bool operator==(const Matrix& rhs) const;
            bool operator!=(const Matrix& rhs) const;

            Matrix& operator+= (const Matrix& M);
            Matrix& operator-= (const Matrix& M);
            Matrix& operator*= (const Matrix& M);
            Matrix& operator*= (float S);
            Matrix& operator/= (float S);
            Matrix& operator/= (const Matrix& M);

        private:

        };

        // Binary operators
        Matrix operator+ (const Matrix& M1, const Matrix& M2);
        Matrix operator- (const Matrix& M1, const Matrix& M2);
        Matrix operator* (const Matrix& M1, const Matrix& M2);
        Matrix operator* (const Matrix& M, float S);
        Matrix operator/ (const Matrix& M, float S);
        Matrix operator/ (const Matrix& M1, const Matrix& M2);
        // Element-wise divide
        Matrix operator* (float S, const Matrix& M);

        /****************************************************************************
        *
        * Vector4
        *
        ****************************************************************************/
        class Vector4 : public DirectX::XMFLOAT4A
        {
        public:
            static const Vector4 Zero;
            static const Vector4 One;

            Vector4() : XMFLOAT4A(Zero) {}
            Vector4(float x, float y, float z, float w)
                :XMFLOAT4A(x, y, z, w) {}

            static Vector4 Min(const Vector4& lhs, const Vector4& rhs);
            static Vector4 Max(const Vector4& lhs, const Vector4& rhs);
            static Vector4 Normalize(const Vector4& vec);

            void Normalize();
            float Length() const;

            bool operator==(const Vector4& rhs) const;
            bool operator!=(const Vector4& rhs) const;

            Vector4& operator+= (const Vector4& V);
            Vector4& operator-= (const Vector4& V);
            Vector4& operator*= (const Vector4& V);
            Vector4& operator*= (float S);
            Vector4& operator/= (float S);
            Vector4& operator/= (const Vector4& V);
        };

        // Binary operators
        Vector4 operator* (const Vector4& vector, const Matrix& matrix);
        Vector4 operator+ (const Vector4& V1, const Vector4& V2);
        Vector4 operator- (const Vector4& V1, const Vector4& V2);
        Vector4 operator* (const Vector4& V1, const Vector4& V2);
        Vector4 operator* (const Vector4& M, float S);
        Vector4 operator/ (const Vector4& M, float S);
        Vector4 operator/ (const Vector4& V1, const Vector4& V2);


        /****************************************************************************
        *
        * Vector3
        *
        ****************************************************************************/
        class Vector3 : public DirectX::XMFLOAT3A
        {
        public:
            static const Vector3 Zero;
            static const Vector3 One;

            Vector3() : XMFLOAT3A(Zero) {}
            Vector3(float x, float y, float z)
                : XMFLOAT3A(x, y, z) {}

            static Vector3 Min(const Vector3& lhs, const Vector3& rhs);
            static Vector3 Max(const Vector3& lhs, const Vector3& rhs);
            static Vector3 Normalize(const Vector3& vec);
            static Vector3 Cross(const Vector3& lhs, const Vector3& rhs);

            void Normalize();
            float Length() const;

            bool operator==(const Vector3& rhs) const;
            bool operator!=(const Vector3& rhs) const;

            Vector3& operator+= (const Vector3& V);
            Vector3& operator-= (const Vector3& V);
            Vector3& operator*= (const Vector3& V);
            Vector3& operator*= (float S);
            Vector3& operator/= (float S);
            Vector3& operator/= (const Vector3& V);

        };

        // Binary operators
        Vector3 operator* (const Vector3& vector, const Matrix& matrix);
        Vector3 operator+ (const Vector3& V1, const Vector3& V2);
        Vector3 operator- (const Vector3& V1, const Vector3& V2);
        Vector3 operator* (const Vector3& V1, const Vector3& V2);
        Vector3 operator* (const Vector3& M, float S);
        Vector3 operator/ (const Vector3& M, float S);
        Vector3 operator/ (const Vector3& V1, const Vector3& V2);

        /****************************************************************************
        *
        * Vector2
        *
        ****************************************************************************/
        class Vector2 : public DirectX::XMFLOAT2A
        {
        public:
            static const Vector2 Zero;
            static const Vector2 One;

            Vector2() : XMFLOAT2A(Zero) {}
            Vector2(float x, float y)
                : XMFLOAT2A(x, y) {}

            static Vector2 Min(const Vector2& lhs, const Vector2& rhs);
            static Vector2 Max(const Vector2& lhs, const Vector2& rhs);
            static Vector2 Normalize(const Vector2& vec);

            void Normalize();
            float Length() const;

            bool operator==(const Vector2& rhs) const;
            bool operator!=(const Vector2& rhs) const;

            Vector2& operator+= (const Vector2& V);
            Vector2& operator-= (const Vector2& V);
            Vector2& operator*= (const Vector2& V);
            Vector2& operator*= (float S);
            Vector2& operator/= (float S);
            Vector2& operator/= (const Vector2& V);
        };

        // Binary operators
        Vector2 operator* (const Vector2& vector, const Matrix& matrix);
        Vector2 operator+ (const Vector2& V1, const Vector2& V2);
        Vector2 operator- (const Vector2& V1, const Vector2& V2);
        Vector2 operator* (const Vector2& V1, const Vector2& V2);
        Vector2 operator* (const Vector2& M, float S);
        Vector2 operator/ (const Vector2& M, float S);
        Vector2 operator/ (const Vector2& V1, const Vector2& V2);

        /****************************************************************************
        *
        * Quaternion
        *
        ****************************************************************************/
        class Quaternion : public DirectX::XMFLOAT4A
        {
        public:
            static const Quaternion Identity;

            Quaternion() : XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f) {}
            Quaternion(float x, float y, float z, float w)
                : XMFLOAT4A(x, y, z, w) {}

            static Quaternion CreateFromAxisAngle(Vector3 axis, float angle);
            static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll);

            bool operator==(const Quaternion& rhs) const;
            bool operator!=(const Quaternion& rhs) const;
        };

        /****************************************************************************
        *
        * Rotation Angles
        *
        ****************************************************************************/

        // Simple class. Doesn't require DirectX::Maths simd.
        // Not indended to be used as shader constants. Ideally
        // only used for more intuitive representation of quaternion.
        // Even more so, It is designed as a human readable version
        // of a rotation hence why it contains functions to convert
        // to and from quaternions.
        // Roll pitch and yaw are all stored in degress rather than radians.
        class RotationAngles
        {
        public:
            RotationAngles(float pitch, float yaw, float roll) :
                m_pitch(pitch), m_yaw(yaw), m_roll(roll) {}

            // Converts the given quaternion to euler/rotation angles
            RotationAngles(const Quaternion& quat);
            RotationAngles() {}

            // Converts the angles to a quaternion
            Quaternion AsQuaternion() const;

            float GetPitch() const;
            float GetYaw() const;
            float GetRoll() const;

            void SetPitch(float pitch);
            void SetYaw(float yaw);
            void SetRoll(float roll);

        private:
            float m_pitch = 0.0f;
            float m_yaw = 0.0f;
            float m_roll = 0.0f;
        };

        /****************************************************************************
        *
        * Color
        *
        ****************************************************************************/
        class Color : public DirectX::XMFLOAT4A
        {
        public:
            Color() : XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f) {}
            Color(float r, float g, float b, float a = 1.0f)
                : XMFLOAT4A(r, g, b, a) {}
            Color(DirectX::XMVECTORF32 color) : XMFLOAT4A(color.f) {}
            inline Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

            static Color FromRGBA(uint32_t rgba);
            static Color FromBGRA(uint32_t bgra);
            static Color FromARGB(uint32_t argb);
            static Color FromABGR(uint32_t abgr);

            Color& operator*= (float S);
            Color& operator/= (float S);

            bool operator==(const Color& rhs) const;
            bool operator!=(const Color& rhs) const;
        };

        /****************************************************************************
        *
        * Bounding box
        *
        ****************************************************************************/
        class BoundingBox
        {
        public:
            BoundingBox() {}
            BoundingBox(Vector3 centre, Vector3 size);

            // Adds a new position to be encapsulated by the bounding box.
            void AddPosition(Vector3 Position);

            inline Vector3 GetSize() const { return m_size; }
            inline Vector3 GetCentre() const { return m_centre; }

            // Returns the result of this bounding box transformed by the matrix
            BoundingBox Transform(Matrix transform);

            static BoundingBox CreateFromVertices(std::vector<Vector3> positions);
            static BoundingBox Combine(const BoundingBox& lhs, const BoundingBox& rhs);

        private:
            Vector3 m_maxPos = { -1.0f, -1.0f, -1.0f };
            Vector3 m_minPos = { 1.0f, 1.0f, 1.0f };
            Vector3 m_centre;
            Vector3 m_size;
        };

        /****************************************************************************
        *
        * Helper Functions
        *
        ****************************************************************************/

        inline float DegreesToRadians(float degrees) { return degrees * (kPI / 180.0f); }
        inline float RadiansToDegrees(float radians) { return radians * (180.0f / kPI); }

        template <typename T>
        inline T Clamp(const T& n, const T& low, const T& high)
        {
            return std::max(low, std::min(n, high));
        }

    }// End namespace Maths
} // End namespace Utils

#include "Math.inl"