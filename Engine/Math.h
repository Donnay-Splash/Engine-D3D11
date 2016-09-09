#pragma once
/*
Helper classes to wrap DirectXMath constructs
*/

#include <DirectXMath.h>
#include <DirectXColors.h>

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
        class Quaternion;
        class Color;

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

            ~Matrix() {}

            static Matrix CreateFromTranslation(Vector3 translation);
            static Matrix CreateFromRotation(Quaternion rotation);
            // only support uniform scale at the moment
            static Matrix CreateFromScale(float uniformScaleFactor);
            static Matrix CreateFromTranslationRotationScale(Vector3 translation, Quaternion rotation, float uniformScaleFactor);
            static Matrix CreatePerspectiveProjectionMatrix(float fovY, float aspectHByW, float nearClip, float farClip);
            static Matrix CreateOrthographicProjectionMatrix(float viewWidth, float viewHeight, float nearClip, float farClip);
            
            Matrix GetTranspose();
			Matrix GetInverse();

        private:

        };

        class Vector4 : public DirectX::XMFLOAT4A
        {
        public:
            static const Vector4 Zero;
            static const Vector4 One;

            Vector4() : XMFLOAT4A(Zero) {}
            Vector4(float x, float y, float z, float w)
                :XMFLOAT4A(x, y, z, w) {}

        };

        class Vector3 : public DirectX::XMFLOAT3A
        {
        public :
            static const Vector3 Zero;
            static const Vector3 One;

            Vector3() : XMFLOAT3A(Zero) {}
            Vector3(float x, float y, float z)
                : XMFLOAT3A(x, y, z) {}

        };

        class Vector2 : public DirectX::XMFLOAT2A
        {
        public:
            static const Vector2 Zero;
            static const Vector2 One;

            Vector2() : XMFLOAT2A(Zero) {}
            Vector2(float x, float y)
                : XMFLOAT2A(x, y) {}
        };

        class Quaternion : public DirectX::XMFLOAT4A
        {
        public:
            static const Quaternion Identity;

            Quaternion() : XMFLOAT4A(0.0f, 0.0f, 0.0f, 1.0f) {}
            Quaternion(float x, float y, float z, float w)
                : XMFLOAT4A(x, y, z, w) {}

            static Quaternion CreateFromAxisAngle(Vector3 axis, float angle);
            static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll);
        };

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
        };
        
    }// End namespace Maths
} // End namespace Utils

#include "Math.inl"