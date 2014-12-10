#pragma once

#include "gep/math3d/constants.h"
#include "gep/math3d/vec3.h"

namespace gep
{
    /// \brief 3x3 matrix
    ///
    /// column-major data layout:
    /// [0] [3] [6]
    /// [1] [4] [7]
    /// [2] [5] [8]
    ///
    // default definition
    ///     |m00 m01 m02|
    /// M = |m10 m11 m12|
    ///     |m20 m21 m22|
    template <typename T>
    struct mat3_t {
        union {
            struct
            {
                //the members are transposed because of column major data layout
                T   m00, m10, m20,
                    m01, m11, m21,
                    m02, m12, m22;
            };
            T data[9]; /// data
        };
        typedef T component_t;

        /// \brief default constructor
        mat3_t()
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(data); i++)
                data[i] = 0;
        }

        /// \brief constructor for a unitialized instance
        inline mat3_t(DoNotInitialize) {}

        /// \brief constructor
        mat3_t(float data[9])
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(this->data); i++)
                this->data[i] = data[i];
        }

        /// \brief returns the determinant of this matrix
        inline T det() const
        {
            T det = data[0] * ( data[4]*data[8] - data[5]*data[7] )
                - data[3] * ( data[1]*data[8] - data[2]*data[7] )
                + data[6] * ( data[1]*data[5] - data[2]*data[4] );
            return det;
        }

        /// \brief returns the transposed version of this matrix
        inline const mat3_t<T> transposed() const
        {
            mat3_t<T> res = *this;
            res.data[1] = data[3];
            res.data[3] = data[1];
            res.data[6] = data[2];
            res.data[2] = data[6];
            res.data[5] = data[7];
            res.data[7] = data[5];
            return res;
        }

        /// \brief returns the inverse of this matrix
        inline const mat3_t<T> inverse() const {
            T det = this->det();
            if(det > -GetEpsilon<T>::value() && det < GetEpsilon<T>::value())
                return mat3_t<T>::identity();

            mat3 res;
            res.m00 =  (m11 * m22 - m21 * m12) / det;
            res.m10 = -(m01 * m22 - m02 * m21) / det;
            res.m20 =  (m01 * m12 - m02 * m11) / det;
            res.m01 = -(m10 * m22 - m12 * m20) / det;
            res.m11 =  (m00 * m22 - m02 * m20) / det;
            res.m21 = -(m00 * m12 - m10 * m02) / det;
            res.m02 =  (m10 * m21 - m20 * m11) / det;
            res.m12 = -(m00 * m21 - m20 * m01) / det;
            res.m22 =  (m00 * m11 - m10 * m01) / det;
            return res;
        }

        /// \brief returns a identity 3x3 matrix
        static const mat3_t<T> identity() {
            mat3_t<T> res(DO_NOT_INITIALIZE);
            res.data[0] = 1; res.data[1] = 0; res.data[2] = 0;
            res.data[3] = 0; res.data[4] = 1; res.data[5] = 0;
            res.data[6] = 0; res.data[7] = 0; res.data[8] = 1;
            return res;
        }

        /// \brief * operator for multiplying this matrix with a 3 component vector
        inline const vec3_t<T> operator * (const vec3_t<T>& v) const {
            vec3_t<T> temp(DO_NOT_INITIALIZE);
            temp.x = v.x * this->data[0] + v.y * this->data[3] + v.z * this->data[6];
            temp.y = v.x * this->data[1] + v.y * this->data[4] + v.z * this->data[7];
            temp.z = v.x * this->data[2] + v.y * this->data[5] + v.z * this->data[8];
            return temp;
        }

        /// \brief * operator for multiplying this matrix with another 3x3 matrix
        inline const mat3_t<T> operator * (const mat3_t<T>& m) const
        {
            mat3_t<T> result(DO_NOT_INITIALIZE);
            for(int i=0;i<3;i++){
                result.data[i*3]   = this->data[0] * m.data[i*3] + this->data[3] * m.data[i*3+1] + this->data[6] * m.data[i*3+2];
                result.data[i*3+1] = this->data[1] * m.data[i*3] + this->data[4] * m.data[i*3+1] + this->data[7] * m.data[i*3+2];
                result.data[i*3+2] = this->data[2] * m.data[i*3] + this->data[5] * m.data[i*3+1] + this->data[8] * m.data[i*3+2];
            }
            return result;
        }

        /// \brief creates a rotation matrix
        ///
        /// \param rotation
        ///   rotation in degrees
        static const mat3_t<T> rotationMatrixXYZ(const vec3_t<T>& rotation)
        {
            mat3_t<T> result(DO_NOT_INITIALIZE);
            float A,B,C,D,E,F,AD,BD;

            A       = cos(-toRadians(rotation.x));
            B       = sin(-toRadians(rotation.x));
            C       = cos(-toRadians(rotation.y));
            D       = sin(-toRadians(rotation.y));
            E       = cos(-toRadians(rotation.z));
            F       = sin(-toRadians(rotation.z));
            AD      =   A * D;
            BD      =   B * D;
            result.data[0]  =   C * E;
            result.data[3]  =  -C * F;
            result.data[6]  =   D;
            result.data[1]  =  BD * E + A * F;
            result.data[4]  = -BD * F + A * E;
            result.data[7]  =  -B * C;
            result.data[2]  = -AD * E + B * F;
            result.data[5]  =  AD * F + B * E;
            result.data[8] =   A * C;
            return result;
        }

        /// \brief creates a scale matrix
        ///
        /// \param scale
        ///   scale to use
        static const mat3_t<T> scaleMatrix(const vec3_t<T>& scale)
        {
            mat3_t<T> res(DO_NOT_INITIALIZE);
            res.data[0] = scale.x; res.data[1] = 0;       res.data[2] = 0;
            res.data[3] = 0;       res.data[4] = scale.y; res.data[5] = 0;
            res.data[6] = 0;       res.data[7] = 0;       res.data[8] = scale.z;
            return res;
        }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(inverse)
            // mat3 * mat3 operator
            LUA_BIND_FUNCTION_PTR(static_cast<const mat3_t<T>(mat3_t<T>::*)(const mat3_t<T>&) const>(&operator *), "__mul")
            // mat3 * vec3 operator
            LUA_BIND_FUNCTION_PTR(static_cast<const vec3_t<T>(mat3_t<T>::*)(const vec3_t<T>&) const>(&operator *), "mulVec3")
            LUA_BIND_FUNCTION(transposed)
            LUA_BIND_FUNCTION(det)
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(m00)
            LUA_BIND_MEMBER(m01)
            LUA_BIND_MEMBER(m02)
            LUA_BIND_MEMBER(m10)
            LUA_BIND_MEMBER(m11)
            LUA_BIND_MEMBER(m12)
            LUA_BIND_MEMBER(m20)
            LUA_BIND_MEMBER(m21)
            LUA_BIND_MEMBER(m22)
        LUA_BIND_VALUE_TYPE_END

        private:
            const vec3_t<T> mulVec3FromScript (const vec3_t<T>& v) const { return *this * v; }
    };

    typedef mat3_t<float> mat3;
};
