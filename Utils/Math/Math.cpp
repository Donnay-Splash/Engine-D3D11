#include "stdafx.h"
#include "Math.h"

namespace Utils
{
    namespace Maths
    {
        /****************************************************************************
        *
        * Vector4 constants
        *
        ****************************************************************************/
        const Matrix Matrix::Identity = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        /****************************************************************************
        *
        * Vector4 constants
        *
        ****************************************************************************/
        const Vector4 Vector4::Zero = { 0.0f, 0.0f, 0.0f, 0.0f };
        const Vector4 Vector4::One = { 1.0f, 1.0f, 1.0f, 1.0f };

        /****************************************************************************
        *
        * Vector3 constants
        *
        ****************************************************************************/
        const Vector3 Vector3::Zero = { 0.0f, 0.0f, 0.0f };
        const Vector3 Vector3::One = { 1.0f, 1.0f, 1.0f };

        /****************************************************************************
        *
        * Vector2 constants
        *
        ****************************************************************************/
        const Vector2 Vector2::Zero = { 0.0f, 0.0f };
        const Vector2 Vector2::One = { 1.0f, 1.0f };

        /****************************************************************************
        *
        * Quaternion constants
        *
        ****************************************************************************/
        const Quaternion Quaternion::Identity = { 0.0f, 0.0f, 0.0f, 1.0f };


    } // End namespace Maths
} // End namespace Utils
