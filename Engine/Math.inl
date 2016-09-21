#include "Math.h"
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

        inline Matrix Matrix::GetTranspose()
        {
            FXMMATRIX matrix = XMLoadFloat4x4A(this);
            XMMATRIX transposeMatrix = XMMatrixTranspose(matrix);

            return ConvertFromXMMATRIX(transposeMatrix);
        }

        inline Matrix Matrix::GetInverse()
        {
            FXMMATRIX matrix = XMLoadFloat4x4A(this);
            XMVECTOR determinant = XMMatrixDeterminant(matrix);
            XMMATRIX inverseMatrix = XMMatrixInverse(&determinant, matrix);

            return ConvertFromXMMATRIX(inverseMatrix);
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

        /****************************************************************************
        *
        * Vector3 operations
        *
        ****************************************************************************/

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

        /****************************************************************************
        *
        * Vector2 operations
        *
        ****************************************************************************/

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

    } // end namespace Maths
} // end namespace Utils