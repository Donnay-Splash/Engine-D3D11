#define NOMINMAX

#include "Math.h"
#include <Utils\DirectxHelpers\EngineHelpers.h>
using namespace DirectX;


namespace Utils
{
    namespace Maths
    {
        // Helpers
        namespace
        {
            Matrix ConvertFromXMMATRIX(const XMMATRIX& matrix)
            {
                Matrix convertedMatrix;
                XMStoreFloat4x4A(&convertedMatrix, matrix);
                return convertedMatrix;
            }

            Quaternion XMVECTORtoQuaternion(const XMVECTOR& vector)
            {
                Quaternion convertedQuaternion;
                XMStoreFloat4A(&convertedQuaternion, vector);
                return convertedQuaternion;
            }
        }

        /****************************************************************************
        *
        * Matrix operations
        *
        ****************************************************************************/
        inline Matrix::Matrix(Vector4 r1, Vector4 r2, Vector4 r3, Vector4 r4)
            : XMFLOAT4X4A(r1.x, r1.y, r1.z, r1.w, r2.x, r2.y, r2.z, r2.w, r3.x, r3.y, r3.z, r3.w, r4.x, r4.y, r4.z, r4.w) {}

        inline Vector3 Matrix::Forward()
        {
            return { _31, _32, _33 };
        }

        inline Vector3 Matrix::Up()
        {
            return { _21, _22, _23 };
        }

        inline Vector3 Matrix::Right()
        {
            return { _11, _12, _13 };
        }

        inline Vector3 Matrix::Translation()
        {
            return { _41, _42, _43 };
        }

        inline Matrix Matrix::CreateFromTranslation(Vector3 translation)
        {
            FXMVECTOR offset = XMLoadFloat3A(&translation);
            XMMATRIX translationMatrix = XMMatrixTranslationFromVector(offset);

            return ConvertFromXMMATRIX(translationMatrix);
        }

        inline Matrix Matrix::CreateFromRotation(Quaternion rotation)
        {
            FXMVECTOR vectorQuaternion = XMLoadFloat4A(&rotation);
            XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(vectorQuaternion);

            return ConvertFromXMMATRIX(rotationMatrix);
        }

        inline Matrix Matrix::CreateFromScale(float uniformScaleFactor)
        {
            XMMATRIX scaleMatrix = XMMatrixScaling(uniformScaleFactor, uniformScaleFactor, uniformScaleFactor);

            return ConvertFromXMMATRIX(scaleMatrix);
        }

        inline Matrix Matrix::CreateFromTranslationRotationScale(Vector3 translation, Quaternion rotation, float uniformScaleFactor)
        {
            Vector3 scaleVector = { uniformScaleFactor, uniformScaleFactor, uniformScaleFactor };
            XMVECTOR xmTranslation = XMLoadFloat3A(&translation);
            XMVECTOR xmRotationOrigin = XMVectorZero();
            XMVECTOR xmRotation = XMLoadFloat4A(&rotation);
            XMVECTOR xmScale = XMLoadFloat3A(&scaleVector);

            XMMATRIX transformMatrix = XMMatrixAffineTransformation(xmScale, xmRotationOrigin, xmRotation, xmTranslation);

            return ConvertFromXMMATRIX(transformMatrix);
        }

        inline Matrix Matrix::CreatePerspectiveProjectionMatrix(float fovY, float aspectHByW, float nearClip, float farClip)
        {
            XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(fovY, aspectHByW, nearClip, farClip);

            return ConvertFromXMMATRIX(projectionMatrix);
        }

        inline Matrix Matrix::CreateOrthographicProjectionMatrix(float viewWidth, float viewHeight, float nearClip, float farClip)
        {
            XMMATRIX projectionMatrix = XMMatrixOrthographicLH(viewWidth, viewHeight, nearClip, farClip);

            return ConvertFromXMMATRIX(projectionMatrix);
        }

        inline Vector3 Matrix::TransformNormal(const Vector3& normal) const
        {
            FXMMATRIX matrix = XMLoadFloat4x4A(this);
            XMVECTOR normalVec = XMLoadFloat3A(&normal);

            XMVECTOR transformedNormal = XMVector3TransformNormal(normalVec, matrix);

            Vector3 result;
            XMStoreFloat3A(&result, transformedNormal);

            return result;
        }

        inline Matrix Matrix::GetTranspose()
        {
            FXMMATRIX matrix = XMLoadFloat4x4A(this);
            XMMATRIX transposeMatrix = XMMatrixTranspose(matrix);

            return ConvertFromXMMATRIX(transposeMatrix);
        }

        inline Matrix Matrix::GetInverse() const
        {
            FXMMATRIX matrix = XMLoadFloat4x4A(this);
            XMVECTOR determinant = XMMatrixDeterminant(matrix);
            XMMATRIX inverseMatrix = XMMatrixInverse(&determinant, matrix);

            return ConvertFromXMMATRIX(inverseMatrix);
        }

        inline std::tuple<Vector3, Quaternion, Vector3> Matrix::Decompose() const
        {
            XMMATRIX matrix = XMLoadFloat4x4A(this);
            XMVECTOR outScale;
            XMVECTOR outRotation;
            XMVECTOR outTranslation;
            XMMatrixDecompose(&outScale, &outRotation, &outTranslation, matrix);

            Vector3 scale;
            Quaternion rotation;
            Vector3 translation;
            XMStoreFloat3A(&scale, outScale);
            XMStoreFloat4A(&rotation, outRotation);
            XMStoreFloat3A(&translation, outTranslation);

            return std::make_tuple(translation, rotation, scale);
        }

        inline bool Matrix::operator == (const Matrix& rhs) const
        {
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._41));

            return (XMVector4Equal(x1, y1)
                && XMVector4Equal(x2, y2)
                && XMVector4Equal(x3, y3)
                && XMVector4Equal(x4, y4)) != 0;
        }

        inline bool Matrix::operator != (const Matrix& rhs) const
        {
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&_41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&rhs._41));

            return (XMVector4NotEqual(x1, y1)
                || XMVector4NotEqual(x2, y2)
                || XMVector4NotEqual(x3, y3)
                || XMVector4NotEqual(x4, y4)) != 0;
        }

        inline Matrix operator+ (const Matrix& M1, const Matrix& M2)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._41));

            x1 = XMVectorAdd(x1, y1);
            x2 = XMVectorAdd(x2, y2);
            x3 = XMVectorAdd(x3, y3);
            x4 = XMVectorAdd(x4, y4);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        inline Matrix& Matrix::operator+= (const Matrix& M)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            x1 = XMVectorAdd(x1, y1);
            x2 = XMVectorAdd(x2, y2);
            x3 = XMVectorAdd(x3, y3);
            x4 = XMVectorAdd(x4, y4);

            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41), x4);
            return *this;
        }

        inline Matrix& Matrix::operator-= (const Matrix& M)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            x1 = XMVectorSubtract(x1, y1);
            x2 = XMVectorSubtract(x2, y2);
            x3 = XMVectorSubtract(x3, y3);
            x4 = XMVectorSubtract(x4, y4);

            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41), x4);
            return *this;
        }

        inline Matrix& Matrix::operator*= (const Matrix& M)
        {
            using namespace DirectX;
            XMMATRIX M1 = XMLoadFloat4x4A(this);
            XMMATRIX M2 = XMLoadFloat4x4A(&M);
            XMMATRIX X = XMMatrixMultiply(M1, M2);
            XMStoreFloat4x4A(this, X);
            return *this;
        }

        inline Matrix& Matrix::operator*= (float S)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41));

            x1 = XMVectorScale(x1, S);
            x2 = XMVectorScale(x2, S);
            x3 = XMVectorScale(x3, S);
            x4 = XMVectorScale(x4, S);

            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41), x4);
            return *this;
        }

        inline Matrix& Matrix::operator/= (float S)
        {
            using namespace DirectX;
            assert(S != 0.f);
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41));

            float rs = 1.f / S;

            x1 = XMVectorScale(x1, rs);
            x2 = XMVectorScale(x2, rs);
            x3 = XMVectorScale(x3, rs);
            x4 = XMVectorScale(x4, rs);

            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41), x4);
            return *this;
        }

        inline Matrix& Matrix::operator/= (const Matrix& M)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            x1 = XMVectorDivide(x1, y1);
            x2 = XMVectorDivide(x2, y2);
            x3 = XMVectorDivide(x3, y3);
            x4 = XMVectorDivide(x4, y4);

            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&_41), x4);
            return *this;
        }

        inline Matrix operator- (const Matrix& M1, const Matrix& M2)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._41));

            x1 = XMVectorSubtract(x1, y1);
            x2 = XMVectorSubtract(x2, y2);
            x3 = XMVectorSubtract(x3, y3);
            x4 = XMVectorSubtract(x4, y4);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        inline Matrix operator* (const Matrix& M1, const Matrix& M2)
        {
            using namespace DirectX;
            XMMATRIX m1 = XMLoadFloat4x4A(&M1);
            XMMATRIX m2 = XMLoadFloat4x4A(&M2);
            XMMATRIX X = XMMatrixMultiply(m1, m2);

            Matrix R;
            XMStoreFloat4x4A(&R, X);
            return R;
        }

        inline Matrix operator* (const Matrix& M, float S)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            x1 = XMVectorScale(x1, S);
            x2 = XMVectorScale(x2, S);
            x3 = XMVectorScale(x3, S);
            x4 = XMVectorScale(x4, S);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        inline Matrix operator/ (const Matrix& M, float S)
        {
            using namespace DirectX;
            assert(S != 0.f);

            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            float rs = 1.f / S;

            x1 = XMVectorScale(x1, rs);
            x2 = XMVectorScale(x2, rs);
            x3 = XMVectorScale(x3, rs);
            x4 = XMVectorScale(x4, rs);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        inline Matrix operator/ (const Matrix& M1, const Matrix& M2)
        {
            using namespace DirectX;
            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M1._41));

            XMVECTOR y1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._11));
            XMVECTOR y2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._21));
            XMVECTOR y3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._31));
            XMVECTOR y4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M2._41));

            x1 = XMVectorDivide(x1, y1);
            x2 = XMVectorDivide(x2, y2);
            x3 = XMVectorDivide(x3, y3);
            x4 = XMVectorDivide(x4, y4);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        inline Matrix operator* (float S, const Matrix& M)
        {
            using namespace DirectX;

            XMVECTOR x1 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._11));
            XMVECTOR x2 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._21));
            XMVECTOR x3 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._31));
            XMVECTOR x4 = XMLoadFloat4A(reinterpret_cast<const XMFLOAT4A*>(&M._41));

            x1 = XMVectorScale(x1, S);
            x2 = XMVectorScale(x2, S);
            x3 = XMVectorScale(x3, S);
            x4 = XMVectorScale(x4, S);

            Matrix R;
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._11), x1);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._21), x2);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._31), x3);
            XMStoreFloat4A(reinterpret_cast<XMFLOAT4A*>(&R._41), x4);
            return R;
        }

        /****************************************************************************
        *
        * Vector4 operations
        *
        ****************************************************************************/

        inline Vector4 Vector4::Min(const Vector4& lhs, const Vector4& rhs)
        {
            auto minX = std::min(rhs.x, lhs.x);
            auto minY = std::min(rhs.y, lhs.y);
            auto minZ = std::min(rhs.z, lhs.z);
            auto minW = std::min(rhs.w, lhs.w);

            return { minX, minY, minZ, minW };
        }

        inline Vector4 Vector4::Max(const Vector4& lhs, const Vector4& rhs)
        {
            auto maxX = std::max(rhs.x, lhs.x);
            auto maxY = std::max(rhs.y, lhs.y);
            auto maxZ = std::max(rhs.z, lhs.z);
            auto maxW = std::max(rhs.w, lhs.w);

            return{ maxX, maxY, maxZ, maxW };
        }

        inline Vector4 Vector4::Normalize(const Vector4& vec)
        {
            XMVECTOR xmVec = XMLoadFloat4A(&vec);
            auto normalized = XMVector4Normalize(xmVec);

            Vector4 result;
            XMStoreFloat4A(&result, normalized);
            return result;
        }

        inline float Vector4::Dot(const Vector4& lhs, const Vector4& rhs)
        {
            XMVECTOR vecL = XMLoadFloat4A(&lhs);
            XMVECTOR vecR = XMLoadFloat4A(&rhs);

            auto result = XMVector4Dot(vecL, vecR);

            float out;
            XMStoreFloat(&out, result);

            return out;
        }

        inline Vector4 Vector4::Abs(const Vector4& vec)
        {
            return{ abs(vec.x), abs(vec.y), abs(vec.z), abs(vec.w) };
        }

        inline void Vector4::Normalize()
        {
            XMVECTOR xmVec = XMLoadFloat4A(this);
            auto normalized = XMVector4Normalize(xmVec);

            XMStoreFloat4A(this, normalized);
        }

        inline float Vector4::Length() const
        {
            XMVECTOR thisVec = XMLoadFloat4A(this);

            auto length = XMVector4Length(thisVec);

            float result;
            XMStoreFloat(&result, length);

            return result;
        }

        inline bool Vector4::operator == (const Vector4& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4Equal(thisVector, rhsVector);
        }

        inline bool Vector4::operator != (const Vector4& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4NotEqual(thisVector, rhsVector);
        }

        inline Vector4& Vector4::operator+= (const Vector4& M)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR otherVector = XMLoadFloat4A(&M);

            XMVECTOR result = XMVectorAdd(thisVector, otherVector);

            XMStoreFloat4A(this, result);
            return *this;
        }

        inline Vector4& Vector4::operator-= (const Vector4& M)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR otherVector = XMLoadFloat4A(&M);

            XMVECTOR result = XMVectorSubtract(thisVector, otherVector);

            XMStoreFloat4A(this, result);
            return *this;
        }

        inline Vector4& Vector4::operator*= (const Vector4& M)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR otherVector = XMLoadFloat4A(&M);

            XMVECTOR result = XMVectorMultiply(thisVector, otherVector);

            XMStoreFloat4A(this, result);
            return *this;
        }

        inline Vector4& Vector4::operator*= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);

            XMVECTOR result = XMVectorScale(thisVector, S);

            XMStoreFloat4A(this, result);
            return *this;
        }

        inline Vector4& Vector4::operator/= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(thisVector, scale);

            XMStoreFloat4A(this, result);
            return *this;
        }

        inline Vector4& Vector4::operator/= (const Vector4& M)
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR otherVector = XMLoadFloat4A(&M);

            XMVECTOR result = XMVectorDivide(thisVector, otherVector);

            XMStoreFloat4A(this, result);
            return *this;
        }

        // Binary operators
        inline Vector4 operator* (const Vector4& vector, const Matrix& matrix)
        {
            XMVECTOR xmVector = XMLoadFloat4A(&vector);
            XMMATRIX xmMatrix = XMLoadFloat4x4A(&matrix);
            XMVECTOR result = XMVector4Transform(xmVector, xmMatrix);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator+ (const Vector4& V1, const Vector4& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat4A(&V1);
            XMVECTOR Vector2 = XMLoadFloat4A(&V2);

            XMVECTOR result = XMVectorAdd(Vector1, Vector2);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator- (const Vector4& V1, const Vector4& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat4A(&V1);
            XMVECTOR Vector2 = XMLoadFloat4A(&V2);

            XMVECTOR result = XMVectorSubtract(Vector1, Vector2);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator- (const Vector4& vec)
        {
            return{ -vec.x, -vec.y, -vec.z, -vec.w };
        }

        inline Vector4 operator* (const Vector4& V1, const Vector4& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat4A(&V1);
            XMVECTOR Vector2 = XMLoadFloat4A(&V2);

            XMVECTOR result = XMVectorMultiply(Vector1, Vector2);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator* (const Vector4& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat4A(&V);

            XMVECTOR result = XMVectorScale(Vector, S);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator/ (const Vector4& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat4A(&V);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(Vector, scale);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        inline Vector4 operator/ (const Vector4& V1, const Vector4& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat4A(&V1);
            XMVECTOR Vector2 = XMLoadFloat4A(&V2);

            XMVECTOR result = XMVectorDivide(Vector1, Vector2);

            Vector4 returnVector;
            XMStoreFloat4A(&returnVector, result);
            return returnVector;
        }

        /****************************************************************************
        *
        * Vector3 operations
        *
        ****************************************************************************/

        inline Vector3 Vector3::Min(const Vector3& lhs, const Vector3& rhs)
        {
            auto minX = std::min(rhs.x, lhs.x);
            auto minY = std::min(rhs.y, lhs.y);
            auto minZ = std::min(rhs.z, lhs.z);

            return{ minX, minY, minZ };
        }

        inline Vector3 Vector3::Max(const Vector3& lhs, const Vector3& rhs)
        {
            auto maxX = std::max(rhs.x, lhs.x);
            auto maxY = std::max(rhs.y, lhs.y);
            auto maxZ = std::max(rhs.z, lhs.z);

            return{ maxX, maxY, maxZ };
        }

        inline Vector3 Vector3::Normalize(const Vector3& vec)
        {
            XMVECTOR xmVec = XMLoadFloat3A(&vec);
            auto normalized = XMVector3Normalize(xmVec);

            Vector3 result;
            XMStoreFloat3A(&result, normalized);
            return result;
        }

        inline Vector3 Vector3::Cross(const Vector3& lhs, const Vector3& rhs)
        {
            XMVECTOR left = XMLoadFloat3A(&lhs);
            XMVECTOR right = XMLoadFloat3A(&rhs);

            auto cross = XMVector3Cross(left, right);

            Vector3 result;
            XMStoreFloat3A(&result, cross);

            return result;
        }

        inline float Vector3::Dot(const Vector3& lhs, const Vector3& rhs)
        {
            XMVECTOR vecL = XMLoadFloat3A(&lhs);
            XMVECTOR vecR = XMLoadFloat3A(&rhs);

            auto result = XMVector3Dot(vecL, vecR);

            float out;
            XMStoreFloat(&out, result);

            return out;
        }

        inline Vector3 Vector3::Abs(const Vector3& vec)
        {
            return{ abs(vec.x), abs(vec.y), abs(vec.z) };
        }

        inline void Vector3::Normalize()
        {
            XMVECTOR xmVec = XMLoadFloat3A(this);
            auto normalized = XMVector3Normalize(xmVec);

            XMStoreFloat3A(this, normalized);
        }

        inline float Vector3::Length() const
        {
            XMVECTOR thisVec = XMLoadFloat3A(this);

            auto length = XMVector3Length(thisVec);

            float result;
            XMStoreFloat(&result, length);

            return result;
        }

        inline bool Vector3::operator == (const Vector3& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat3(this);
            XMVECTOR rhsVector = XMLoadFloat3(&rhs);

            return XMVector3Equal(thisVector, rhsVector);
        }

        inline bool Vector3::operator != (const Vector3& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat3(this);
            XMVECTOR rhsVector = XMLoadFloat3(&rhs);

            return XMVector3NotEqual(thisVector, rhsVector);
        }

        inline Vector3& Vector3::operator+= (const Vector3& M)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);
            XMVECTOR otherVector = XMLoadFloat3A(&M);

            XMVECTOR result = XMVectorAdd(thisVector, otherVector);

            XMStoreFloat3A(this, result);
            return *this;
        }

        inline Vector3& Vector3::operator-= (const Vector3& M)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);
            XMVECTOR otherVector = XMLoadFloat3A(&M);

            XMVECTOR result = XMVectorSubtract(thisVector, otherVector);

            XMStoreFloat3A(this, result);
            return *this;
        }

        inline Vector3& Vector3::operator*= (const Vector3& M)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);
            XMVECTOR otherVector = XMLoadFloat3A(&M);

            XMVECTOR result = XMVectorMultiply(thisVector, otherVector);

            XMStoreFloat3A(this, result);
            return *this;
        }

        inline Vector3& Vector3::operator*= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);

            XMVECTOR result = XMVectorScale(thisVector, S);

            XMStoreFloat3A(this, result);
            return *this;
        }

        inline Vector3& Vector3::operator/= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(thisVector, scale);

            XMStoreFloat3A(this, result);
            return *this;
        }

        inline Vector3& Vector3::operator/= (const Vector3& M)
        {
            XMVECTOR thisVector = XMLoadFloat3A(this);
            XMVECTOR otherVector = XMLoadFloat3A(&M);

            XMVECTOR result = XMVectorDivide(thisVector, otherVector);

            XMStoreFloat3A(this, result);
            return *this;
        }

        // Binary operators
        inline Vector3 operator* (const Vector3& vector, const Matrix& matrix)
        {
            XMVECTOR xmVector = XMLoadFloat3A(&vector);
            XMMATRIX xmMatrix = XMLoadFloat4x4A(&matrix);
            XMVECTOR result = XMVector3TransformCoord(xmVector, xmMatrix);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator+ (const Vector3& V1, const Vector3& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat3A(&V1);
            XMVECTOR Vector2 = XMLoadFloat3A(&V2);

            XMVECTOR result = XMVectorAdd(Vector1, Vector2);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator- (const Vector3& V1, const Vector3& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat3A(&V1);
            XMVECTOR Vector2 = XMLoadFloat3A(&V2);

            XMVECTOR result = XMVectorSubtract(Vector1, Vector2);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator- (const Vector3& vec)
        {
            return { -vec.x, -vec.y, -vec.z };
        }

        inline Vector3 operator* (const Vector3& V1, const Vector3& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat3A(&V1);
            XMVECTOR Vector2 = XMLoadFloat3A(&V2);

            XMVECTOR result = XMVectorMultiply(Vector1, Vector2);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator* (const Vector3& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat3A(&V);

            XMVECTOR result = XMVectorScale(Vector, S);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator/ (const Vector3& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat3A(&V);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(Vector, scale);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        inline Vector3 operator/ (const Vector3& V1, const Vector3& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat3A(&V1);
            XMVECTOR Vector2 = XMLoadFloat3A(&V2);

            XMVECTOR result = XMVectorDivide(Vector1, Vector2);

            Vector3 returnVector;
            XMStoreFloat3A(&returnVector, result);
            return returnVector;
        }

        /****************************************************************************
        *
        * Vector2 operations
        *
        ****************************************************************************/

        inline Vector2 Vector2::Min(const Vector2& lhs, const Vector2& rhs)
        {
            auto minX = std::min(rhs.x, lhs.x);
            auto minY = std::min(rhs.y, lhs.y);

            return{ minX, minY };
        }

        inline Vector2 Vector2::Max(const Vector2& lhs, const Vector2& rhs)
        {
            auto maxX = std::max(rhs.x, lhs.x);
            auto maxY = std::max(rhs.y, lhs.y);

            return{ maxX, maxY };
        }

        inline Vector2 Vector2::Normalize(const Vector2& vec)
        {
            XMVECTOR xmVec = XMLoadFloat2A(&vec);
            auto normalized = XMVector2Normalize(xmVec);

            Vector2 result;
            XMStoreFloat2A(&result, normalized);
            return result;
        }

        inline float Vector2::Dot(const Vector2& lhs, const Vector2& rhs)
        {
            XMVECTOR vecL = XMLoadFloat2A(&lhs);
            XMVECTOR vecR = XMLoadFloat2A(&rhs);

            auto result = XMVector2Dot(vecL, vecR);

            float out;
            XMStoreFloat(&out, result);

            return out;
        }

        inline Vector2 Vector2::Abs(const Vector2& vec)
        {
            return{ abs(vec.x), abs(vec.y) };
        }

        inline void Vector2::Normalize()
        {
            XMVECTOR xmVec = XMLoadFloat2A(this);
            auto normalized = XMVector2Normalize(xmVec);

            XMStoreFloat2A(this, normalized);
        }

        inline float Vector2::Length() const
        {
            XMVECTOR thisVec = XMLoadFloat2A(this);

            auto length = XMVector2Length(thisVec);

            float result;
            XMStoreFloat(&result, length);

            return result;
        }

        inline bool Vector2::operator == (const Vector2& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat2(this);
            XMVECTOR rhsVector = XMLoadFloat2(&rhs);

            return XMVector3Equal(thisVector, rhsVector);
        }

        inline bool Vector2::operator != (const Vector2& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat2(this);
            XMVECTOR rhsVector = XMLoadFloat2(&rhs);

            return XMVector2NotEqual(thisVector, rhsVector);
        }

        inline Vector2& Vector2::operator+= (const Vector2& M)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);
            XMVECTOR otherVector = XMLoadFloat2A(&M);

            XMVECTOR result = XMVectorAdd(thisVector, otherVector);

            XMStoreFloat2A(this, result);
            return *this;
        }

        inline Vector2& Vector2::operator-= (const Vector2& M)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);
            XMVECTOR otherVector = XMLoadFloat2A(&M);

            XMVECTOR result = XMVectorSubtract(thisVector, otherVector);

            XMStoreFloat2A(this, result);
            return *this;
        }

        inline Vector2& Vector2::operator*= (const Vector2& M)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);
            XMVECTOR otherVector = XMLoadFloat2A(&M);

            XMVECTOR result = XMVectorMultiply(thisVector, otherVector);

            XMStoreFloat2A(this, result);
            return *this;
        }

        inline Vector2& Vector2::operator*= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);

            XMVECTOR result = XMVectorScale(thisVector, S);

            XMStoreFloat2A(this, result);
            return *this;
        }

        inline Vector2& Vector2::operator/= (float S)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(thisVector, scale);

            XMStoreFloat2A(this, result);
            return *this;
        }

        inline Vector2& Vector2::operator/= (const Vector2& M)
        {
            XMVECTOR thisVector = XMLoadFloat2A(this);
            XMVECTOR otherVector = XMLoadFloat2A(&M);

            XMVECTOR result = XMVectorDivide(thisVector, otherVector);

            XMStoreFloat2A(this, result);
            return *this;
        }

        // Binary operators
        inline Vector2 operator* (const Vector2& vector, const Matrix& matrix)
        {
            XMVECTOR xmVector = XMLoadFloat2A(&vector);
            XMMATRIX xmMatrix = XMLoadFloat4x4A(&matrix);
            XMVECTOR result = XMVector2TransformCoord(xmVector, xmMatrix);

            Vector2 returnVector;
            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator+ (const Vector2& V1, const Vector2& V2)
        {
            XMVECTOR Vector1 = XMLoadFloat2A(&V1);
            Vector2 returnVector;
            XMVECTOR Vector2 = XMLoadFloat2A(&V2);
            XMVECTOR result = XMVectorAdd(Vector1, Vector2);
            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator- (const Vector2& V1, const Vector2& V2)
        {
            Vector2 returnVector;
            XMVECTOR Vector1 = XMLoadFloat2A(&V1);
            XMVECTOR Vector2 = XMLoadFloat2A(&V2);

            XMVECTOR result = XMVectorSubtract(Vector1, Vector2);

            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator- (const Vector2& vec)
        {
            return{ -vec.x, -vec.y };
        }

        inline Vector2 operator* (const Vector2& V1, const Vector2& V2)
        {
            Vector2 returnVector;
            XMVECTOR Vector1 = XMLoadFloat2A(&V1);
            XMVECTOR Vector2 = XMLoadFloat2A(&V2);

            XMVECTOR result = XMVectorMultiply(Vector1, Vector2);

            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator* (const Vector2& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat2A(&V);

            XMVECTOR result = XMVectorScale(Vector, S);

            Vector2 returnVector;
            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator/ (const Vector2& V, float S)
        {
            XMVECTOR Vector = XMLoadFloat2A(&V);

            float scale = 1.0f / S;
            XMVECTOR result = XMVectorScale(Vector, scale);

            Vector2 returnVector;
            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        inline Vector2 operator/ (const Vector2& V1, const Vector2& V2)
        {
            Vector2 returnVector;
            XMVECTOR Vector1 = XMLoadFloat2A(&V1);
            XMVECTOR Vector2 = XMLoadFloat2A(&V2);

            XMVECTOR result = XMVectorDivide(Vector1, Vector2);

            XMStoreFloat2A(&returnVector, result);
            return returnVector;
        }

        /****************************************************************************
        *
        * Quaternion operations
        *
        ****************************************************************************/

        inline Quaternion Quaternion::CreateFromAxisAngle(Vector3 axis, float angle)
        {
            FXMVECTOR xmAxis = XMLoadFloat3A(&axis);
            XMVECTOR quaternionVector = XMQuaternionRotationAxis(xmAxis, angle);

            return XMVECTORtoQuaternion(quaternionVector);
        }

        inline Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
        {
            XMVECTOR quaternionVector = XMQuaternionRotationRollPitchYaw(pitch, yaw, roll);
            XMVECTOR normalizedQuaternion = XMVector4Normalize(quaternionVector);

            return XMVECTORtoQuaternion(normalizedQuaternion);
        }

        inline bool Quaternion::operator == (const Quaternion& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4Equal(thisVector, rhsVector);
        }

        inline bool Quaternion::operator != (const Quaternion& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4NotEqual(thisVector, rhsVector);
        }

        /****************************************************************************
        *
        * Rotation Angles
        *
        ****************************************************************************/

        // Taken from 3D math primer for graphics and game development
        // Authors: Fletcher Dunn, Ian Parberry
        inline RotationAngles::RotationAngles(const Quaternion& quat)
        {
            // extract sin(pitch)
            float sp = -2.0f * (quat.y*quat.z - quat.w*quat.x);

            // Check for gimbal lock, giving slight tolerance for numerical imprecision
            if (abs(sp) > 0.9999f)
            {
                // looking straight up or down
                m_pitch = RadiansToDegrees(kPI * 0.5f * sp);

                // Compute Yaw and set roll to 0
                m_yaw = RadiansToDegrees(atan2(-quat.x*quat.z + quat.w*quat.y, 0.5f - quat.y*quat.y - quat.z*quat.z));
                m_roll = 0.0f;
            }
            else
            {
                // compute angles
                m_pitch = RadiansToDegrees(asin(sp));
                m_yaw = RadiansToDegrees(atan2(quat.x*quat.z + quat.w*quat.y, 0.5f - quat.x*quat.x - quat.y*quat.y));
                m_roll = RadiansToDegrees(atan2(quat.x*quat.y + quat.w*quat.z, 0.5f - quat.x*quat.x - quat.z*quat.z));
            }
        }

        // Converts the angles to a quaternion
        inline Quaternion RotationAngles::AsQuaternion() const 
        {
            float pitchRads = DegreesToRadians(m_pitch);
            float yawRads = DegreesToRadians(m_yaw);
            float rollRads = DegreesToRadians(m_roll);

            return Quaternion::CreateFromYawPitchRoll(yawRads, pitchRads, rollRads);
        }

        inline float RotationAngles::GetPitch() const { return m_pitch; }
        inline float RotationAngles::GetYaw() const { return m_yaw; }
        inline float RotationAngles::GetRoll() const { return m_roll; }

        inline void RotationAngles::SetPitch(float pitch) { m_pitch = pitch; }
        inline void RotationAngles::SetYaw(float yaw) { m_yaw = yaw; }
        inline void RotationAngles::SetRoll(float roll) { m_roll = roll; }

        /****************************************************************************
        *
        * Color operations
        *
        ****************************************************************************/
        inline Color::Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255U)
        {
            float red = static_cast<float>(r) / 255.0f;
            float green = static_cast<float>(g) / 255.0f;
            float blue = static_cast<float>(b) / 255.0f;
            float alpha = static_cast<float>(a) / 255.0f;

            ::XMFLOAT4A(red, green, blue, alpha);
        }

        inline Color Color::FromRGBA(uint32_t rgba)
        {
            uint8_t r = static_cast<uint8_t>(rgba >> 24);
            uint8_t g = static_cast<uint8_t>(rgba >> 16);
            uint8_t b = static_cast<uint8_t>(rgba >> 8);
            uint8_t a = static_cast<uint8_t>(rgba >> 0);

            return Color(r, g, b, a);
        }

        inline Color Color::FromBGRA(uint32_t bgra)
        {
            uint8_t b = static_cast<uint8_t>(bgra >> 24);
            uint8_t g = static_cast<uint8_t>(bgra >> 16);
            uint8_t r = static_cast<uint8_t>(bgra >> 8);
            uint8_t a = static_cast<uint8_t>(bgra >> 0);

            return Color(r, g, b, a);
        }

        inline Color Color::FromARGB(uint32_t argb)
        {
            uint8_t a = static_cast<uint8_t>(argb >> 24);
            uint8_t r = static_cast<uint8_t>(argb >> 16);
            uint8_t g = static_cast<uint8_t>(argb >> 8);
            uint8_t b = static_cast<uint8_t>(argb >> 0);

            return Color(r, g, b, a);
        }

        inline Color Color::FromABGR(uint32_t abgr)
        {
            uint8_t a = static_cast<uint8_t>(abgr >> 24);
            uint8_t b = static_cast<uint8_t>(abgr >> 16);
            uint8_t g = static_cast<uint8_t>(abgr >> 8);
            uint8_t r = static_cast<uint8_t>(abgr >> 0);

            return Color(r, g, b, a);
        }

        inline Color& Color::operator*=(float S)
        {
            // Don't want to modify alpha as it has special purpose.
            // Avoids intrinsics root but is a basic operation so shouldn't
            // be too costly
            this->x *= S;
            this->y *= S;
            this->z *= S;

            return *this;
        }

        inline Color& Color::operator/=(float S)
        {
            // Don't want to modify alpha as it has special purpose.
            // Avoids intrinsics root but is a basic operation so shouldn't
            // be too costly
            this->x /= S;
            this->y /= S;
            this->z /= S;

            return *this;
        }

        inline bool Color::operator == (const Color& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4Equal(thisVector, rhsVector);
        }

        inline bool Color::operator != (const Color& rhs) const
        {
            XMVECTOR thisVector = XMLoadFloat4A(this);
            XMVECTOR rhsVector = XMLoadFloat4A(&rhs);

            return XMVector4NotEqual(thisVector, rhsVector);
        }

        /****************************************************************************
        *
        * BoundingBox operations
        *
        ****************************************************************************/

        inline BoundingBox::BoundingBox(Vector3 centre, Vector3 size) :
            m_centre(centre), m_size(size)
        {
            // Size cannot be negative
            size.x = abs(size.x);
            size.y = abs(size.y);
            size.z = abs(size.z);
            m_maxPos = centre + size;
            m_minPos = centre - size;
        }

        // Adds a new position to be encapsulated by the bounding box.
        inline void BoundingBox::AddPosition(Vector3 position)
        {
            // First position so just set both values to this
            if (m_minPos.x > m_maxPos.x && m_minPos.y > m_maxPos.y && m_minPos.z > m_maxPos.z)
            {
                m_minPos = m_maxPos = position;
            }
            else
            {
                // Max pos
                if (position.x > m_maxPos.x) m_maxPos.x = position.x;
                if (position.y > m_maxPos.y) m_maxPos.y = position.y;
                if (position.z > m_maxPos.z) m_maxPos.z = position.z;

                // Min pos
                if (position.x < m_minPos.x) m_minPos.x = position.x;
                if (position.y < m_minPos.y) m_minPos.y = position.y;
                if (position.z < m_minPos.z) m_minPos.z = position.z;

            }
            // Now update centre and size
            m_size = (m_maxPos - m_minPos) / 2.0f;
            m_centre = m_maxPos - m_size;
        }

        inline BoundingBox BoundingBox::Transform(Matrix transform)
        {
            auto xa = transform.Right() * m_minPos.x;
            auto xb = transform.Right() * m_maxPos.x;

            auto ya = transform.Up() * m_minPos.y;
            auto yb = transform.Up() * m_maxPos.y;

            auto za = transform.Forward() * m_minPos.z;
            auto zb = transform.Forward() * m_maxPos.z;

            auto minPos = Vector3::Min(xa, xb) + Vector3::Min(ya, yb) + Vector3::Min(za, zb) + transform.Translation();
            auto maxPos = Vector3::Max(xa, xb) + Vector3::Max(ya, yb) + Vector3::Max(za, zb) + transform.Translation();
            BoundingBox transformedBounds;
            transformedBounds.AddPosition(minPos);
            transformedBounds.AddPosition(maxPos);
            return transformedBounds;
        }

        inline BoundingBox BoundingBox::CreateFromVertices(std::vector<Vector3> positions)
        {
            BoundingBox bounds;
            for (auto position : positions)
            {
                bounds.AddPosition(position);
            }

            return bounds;
        }

        inline BoundingBox BoundingBox::Combine(const BoundingBox& lhs, const BoundingBox& rhs)
        {
            BoundingBox combinedBoundingBox;
            auto minPos = Vector3::Min(lhs.m_minPos, rhs.m_minPos);
            auto maxPos = Vector3::Max(lhs.m_maxPos, rhs.m_maxPos);
            combinedBoundingBox.AddPosition(minPos);
            combinedBoundingBox.AddPosition(maxPos);
            return combinedBoundingBox;
        }

    } // end namespace Maths
} // end namespace Utils