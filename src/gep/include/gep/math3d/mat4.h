#pragma once

#include "gep/math3d/constants.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/vec4.h"
#include "gep/math3d/mat3.h"

namespace gep
{
    /// \brief a 4x4 matrix
    ///
    /// uses a column-major data layout
    /// [ 0] [ 4] [ 8] [12]
    /// [ 1] [ 5] [ 9] [13]
    /// [ 2] [ 6] [10] [14]
    /// [ 3] [ 7] [11] [15]
    ///
    /// definition is:
    ///     |m00 m01 m02 m03|
    ///     |m10 m11 m12 m13|
    /// M = |m20 m21 m22 m23|
    ///     |m30 m31 m32 m33|
    template <typename T>
    struct mat4_t
    {
        union {
            struct {
                //the members are transposed because of column major data layout
                T   m00, m10, m20, m30,
                    m01, m11, m21, m31,
                    m02, m12, m22, m32,
                    m03, m13, m23, m33;
            };
            T data[16];
        };
        typedef T component_t;

        /// \brief default constructor
        mat4_t()
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(data); i++)
                data[i] = 0;
        }

        /// \brief constructor without initialization
        inline mat4_t(DoNotInitialize) {}

        /// \brief constructor
        inline mat4_t(const T (&data)[16])
        {
            for(size_t i=0; i < GEP_ARRAY_SIZE(data); i++)
                this->data[i] = data[i];
        }

        /// \brief * operator for multiplying with another 4x4 matrix
        inline const mat4_t<T> operator * (const mat4_t<T>& m) const
        {
            mat4_t<T> result(DO_NOT_INITIALIZE);
            for(int i=0;i<4;i++){
                result.data[i*4]   = this->data[0] * m.data[i*4] + this->data[4] * m.data[i*4+1] + this->data[ 8] * m.data[i*4+2] + this->data[12] * m.data[i*4+3];
                result.data[i*4+1] = this->data[1] * m.data[i*4] + this->data[5] * m.data[i*4+1] + this->data[ 9] * m.data[i*4+2] + this->data[13] * m.data[i*4+3];
                result.data[i*4+2] = this->data[2] * m.data[i*4] + this->data[6] * m.data[i*4+1] + this->data[10] * m.data[i*4+2] + this->data[14] * m.data[i*4+3];
                result.data[i*4+3] = this->data[3] * m.data[i*4] + this->data[7] * m.data[i*4+1] + this->data[11] * m.data[i*4+2] + this->data[15] * m.data[i*4+3];
            }
            return result;
        }

        /// \brief * operator for multiplying this matrix with a 4 component vector
        inline const vec4_t<T> operator * (const vec4_t<T>& v) const {
            vec4_t<T> temp(DO_NOT_INITIALIZE);
            temp.x = v.x * this->data[0] + v.y * this->data[4] + v.z * this->data[8] + v.w * this->data[12];
            temp.y = v.x * this->data[1] + v.y * this->data[5] + v.z * this->data[9] + v.w * this->data[13];
            temp.z = v.x * this->data[2] + v.y * this->data[6] + v.z * this->data[10] + v.w * this->data[14];
            temp.w = v.x * this->data[3] + v.y * this->data[7] + v.z * this->data[11] + v.w * this->data[15];
            return temp;
        }

        /// \brief transforms a direction vector
        inline const vec3_t<T> transformDirection(const vec3_t<T>& v) const
        {
            vec3_t<T> temp(DO_NOT_INITIALIZE);
            temp.x = v.x * data[0] + v.y * data[4] + v.z * data[8];
            temp.y = v.x * data[1] + v.y * data[5] + v.z * data[9];
            temp.z = v.x * data[2] + v.y * data[6] + v.z * data[10];
            return temp;
        }

        /// \brief transform a position vector
        inline const vec3_t<T> transformPosition(const vec3_t<T>& v) const
        {
            vec3_t<T> temp(DO_NOT_INITIALIZE);
            temp.x = v.x * data[0] + v.y * data[4] + v.z * data[8]  + data[12];
            temp.y = v.x * data[1] + v.y * data[5] + v.z * data[9]  + data[13];
            temp.z = v.x * data[2] + v.y * data[6] + v.z * data[10] + data[14];
            return temp;
        }

        /// \brief Returns: a 3x3 submatrix of this one
        /// \param i
        ///   x shift
        ///
        /// \param j
        ///   y shift
        inline const mat3_t<T> submat(const int i, const int j) const
        {
            mat3_t<T> mb(DO_NOT_INITIALIZE);
            int di, dj, si, sj;
            // loop through 3x3 submatrix
            for( di = 0; di < 3; di ++ ) {
                for( dj = 0; dj < 3; dj ++ ) {
                    // map 3x3 element (destination) to 4x4 element (source)
                    si = di + ( ( di >= i ) ? 1 : 0 );
                    sj = dj + ( ( dj >= j ) ? 1 : 0 );
                    // copy element
                    mb.data[di * 3 + dj] = data[si * 4 + sj];
                }
            }
            return mb;
        }

        /// \brief returns the determinant of this matrix
        inline T det() const
        {
            T det = 0, result = 0, i = 1;
            mat3_t<T> msub3(DO_NOT_INITIALIZE);
            for (int n = 0; n < 4; n++, i *= -1 )
            {
                msub3   = this->submat(0,n);
                det     = msub3.det();
                result += data[n] * det * i;
            }
            return result;
        }

        /// \brief returns the inverse of this matrix
        inline const mat4_t<T> inverse() const
        {
            mat4_t<T> mr(DO_NOT_INITIALIZE);
            T mdet = this->det();
            mat3_t<T> mtemp(DO_NOT_INITIALIZE);
            int sign=0;
            if ( mdet > -GetEpsilon<T>::value() && mdet < GetEpsilon<T>::value() )
                return mat4::identity();
            for (int i = 0; i < 4; i++ )
                for (int j = 0; j < 4; j++ ){
                    sign = 1 - ( (i +j) % 2 ) * 2;
                    mtemp = this->submat(i, j);
                    mr.data[i+j*4] = ( mtemp.det() * sign ) / mdet;
                }
                return mr;
        }

        /// \brief returns the transposed version of this matrix
        inline const mat4_t<T> transposed() const
        {
            mat4_t<T> mr(DO_NOT_INITIALIZE);
            mr.data[0] = data[0];
            mr.data[1] = data[4];
            mr.data[2] = data[8];
            mr.data[3] = data[12];
            mr.data[4] = data[1];
            mr.data[5] = data[5];
            mr.data[6] = data[9];
            mr.data[7] = data[13];
            mr.data[8] = data[2];
            mr.data[9] = data[6];
            mr.data[10] = data[10];
            mr.data[11] = data[14];
            mr.data[12] = data[3];
            mr.data[13] = data[7];
            mr.data[14] = data[11];
            mr.data[15] = data[15];
            return mr;
        }

        /// \brief returns the normal matrix of this matrix (inverse transposed)
        inline const mat3_t<T> normalMatrix() const
        {
            mat4_t<T> mr(*this);
            mr.data[3] = 0;
            mr.data[7] = 0;
            mr.data[11] = 0;
            mr.data[12] = 0;
            mr.data[13] = 0;
            mr.data[14] = 0;
            mr.data[15] = 1;
            return mr.inverse().transposed().rotationPart();
        }

        /// \brief returns the rotation part of this matrix
        inline const mat3_t<T> rotationPart() const
        {
            mat3_t<T> m3(DO_NOT_INITIALIZE);
            for(int y=0;y<3;y++){
                for(int x=0;x<3;x++){
                    m3.data[y*3+x] = data[y*4+x];
                }
            }
            return m3;
        }

        /// \brief sets the rotation part of this matrix
        inline void setRotationPart(const mat3& m3)
        {
            for(int y=0;y<3;y++){
                for(int x=0;x<3;x++){
                    data[y*4+x] = m3.data[y*3+x];
                }
            }
        }


        /// \brief returns the translation part of this matrix
        inline const vec3_t<T> translationPart() const
        {
            return vec3_t<T>(data[12], data[13], data[14]);
        }

        /// \brief switches the matrix from left to righthanded
        inline const mat4_t<T> right2Left() const
        {
            mat4_t<T> mr(*this);
            for(int i=0;i<4;i++)
            {
                mr.data[4+i] = this->data[8+i];
                mr.data[8+i] = this->data[4+i] * -1;
            }
            return mr;
        }

        /// \brief a 4x4 identity matrix
        static const mat4_t<T> identity()
        {
            mat4_t<T> mat(DO_NOT_INITIALIZE);
            mat.data[ 0]=1; mat.data[ 1]=0; mat.data[ 2]=0; mat.data[ 3]=0;
            mat.data[ 4]=0; mat.data[ 5]=1; mat.data[ 6]=0; mat.data[ 7]=0;
            mat.data[ 8]=0; mat.data[ 9]=0; mat.data[10]=1; mat.data[11]=0;
            mat.data[12]=0; mat.data[13]=0; mat.data[14]=0; mat.data[15]=1;
            return mat;
        }


        /// \brief Creates a perspective projection matrix
        ///
        ///  \param pViewAngle
        ///    the view angle in degrees
        ///  \param pAspectRatio
        ///    the screen aspect ratio
        ///  \param pNear
        ///    near clipping plane distance
        ///  \param pFar
        ///    far clipping plane distance
        ///
        static const mat4_t<T> projectionMatrix(T pViewAngle, T pAspectRatio, T pNear, T pFar)
        {
            mat4_t<T> res;
            pViewAngle = (pViewAngle / static_cast<T>(180)) * GetPi<T>::value();
            //T yScale = cos(pViewAngle / 2) / sin(pViewAngle / 2);
            //T xScale = yScale / pAspectRatio;
            // X-Achse
            //res.data[0] = xScale;
            res.data[0] = 1 / (tan( pViewAngle/2) * pAspectRatio);
            res.data[1] = 0; res.data[2] = 0; res.data[3] = 0;

            // Y-Achse
            //res.data[5] = yScale;
            res.data[5] = 1 / tan( pViewAngle/2);
            res.data[4] = 0; res.data[6] = 0; res.data[7] = 0;

            // Z-Achse
            //res.data[10] = pFar / (pFar - pNear);
            res.data[10] = pFar / (pNear - pFar);
            res.data[11] = -1;
            res.data[8] = 0; res.data[9] = 0;

            // W-Achse -zn*zf/(zf-zn)
            //res.data[14] = -pNear * pFar / (pFar - pNear);
            res.data[14] = (pFar * pNear) / (pNear - pFar);
            res.data[12] = 0; res.data[13] = 0; res.data[15] = 0;
            return res;
        }

        /// \brief Creates a perspective projection matrix
        ///
        ///  \param pLeft
        ///    left bound
        ///  \param pRight
        ///    right bound
        ///  \param pBottom
        ///    bottom bound
        ///  \param pTop
        ///    top bound
        ///  \param pNear
        ///    near clipping plane distance
        ///  \param pFar
        ///    far clipping plane distance
        ///
        static const mat4_t<T> frustrum(T pLeft, T pRight, T pBottom, T pTop, T pNear, T pFar)
        {
            mat4_t<T> res(DO_NOT_INITIALIZE);
            res.data[0] = (2*pNear) / (pRight - pLeft);
            res.data[1] = 0;
            res.data[2] = 0;
            res.data[3] = 0;

            res.data[4] = 0;
            res.data[5] = (2*pNear) / (pTop - pBottom);
            res.data[6] = 0;
            res.data[7] = 0;

            res.data[8] = (pRight + pLeft) / (pRight - pLeft);
            res.data[9] = (pTop + pBottom) / (pTop - pBottom);
            res.data[10] = -(pFar + pNear) / (pFar - pNear);
            res.data[11] = -1;

            res.data[12] = 0;
            res.data[13] = 0;
            res.data[14] = (-2 * pFar * pNear) / (pFar - pNear);
            res.data[15] = 0;
            return res;
        }

        /// \brief Creates a paralell projection matrix
        ///
        /// \param pLeft
        ///   left bound
        /// \param pRight
        ///   right bound
        /// \param pBottom
        ///   bottom bound
        /// \param pTop
        ///   top bound
        /// \param pNear
        ///   near clipping plane distance
        /// \param pFar
        ///   far clipping plane distance
        ///
        static const mat4_t<T> ortho(T pLeft, T pRight, T pBottom, T pTop, T pNear, T pFar)
        {
            mat4_t<T> res(DO_NOT_INITIALIZE);
            res.data[0] = 2 / (pRight - pLeft);
            res.data[1] = 0;
            res.data[2] = 0;
            res.data[3] = 0;

            res.data[4] = 0;
            res.data[5] = 2 / (pTop - pBottom);
            res.data[6] = 0;
            res.data[7] = 0;

            res.data[8] = 0;
            res.data[9] = 0;
            res.data[10] = -1 / (pFar - pNear);
            res.data[11] = 0;

            res.data[12] = (pLeft + pRight) / (pLeft - pRight);
            res.data[13] = (pTop + pBottom) / (pBottom - pTop);
            res.data[14] = pNear / (pNear - pFar);
            res.data[15] = 1;
            return res;
        }

        /// \brief Creates a look at matrix for camera usage
        ///
        /// \param pFrom
        ///   position of the viewer
        /// \param pTo
        ///   position to look at
        /// \param pUp
        ///   up vector
        ///
        static const mat4_t<T> lookAtMatrix(const vec3_t<T>& pFrom, const vec3_t<T>& pTo, const vec3_t<T>& pUp)
        {
            vec3_t<T> x,y,z;
            mat4_t<T> res(DO_NOT_INITIALIZE);
            z = (pTo - pFrom).normalized();
            x = z.cross(pUp.normalized()).normalized();
            y = x.cross(z);
            z = z * -1;

            //X
            res.data[0] = x.x;
            res.data[4] = x.y;
            res.data[8] = x.z;
            res.data[12] = -x.dot(pFrom);

            //Y
            res.data[1] = y.x;
            res.data[5] = y.y;
            res.data[9] = y.z;
            res.data[13] = -y.dot(pFrom);

            //Z
            res.data[2] = z.x;
            res.data[6] = z.y;
            res.data[10] = z.z;
            res.data[14] = -z.dot(pFrom);

            //W
            res.data[3] = res.data[7] = res.data[11] = 0;
            res.data[15] = 1;

            return res;
        }


        /// \brief Creats a look at matrix for camera usage
        ///
        /// \param pFrom
        ///   position of the viewer
        /// \param pTo
        ///   position to look at
        /// \param pUp
        ///   up vector
        ///
        static const mat4_t<T> lookDirMatrix(const vec3_t<T>& dir, const vec3_t<T>& pUp)
        {
            vec3_t<T> x,y,z;
            mat4_t<T> res(DO_NOT_INITIALIZE);
            z = dir.normalized();
            x = z.cross(pUp.normalized()).normalized();
            y = x.cross(z);
            z = z * -1;

            //X
            res.data[0] = x.x;
            res.data[4] = x.y;
            res.data[8] = x.z;
            res.data[12] = 0;

            //Y
            res.data[1] = y.x;
            res.data[5] = y.y;
            res.data[9] = y.z;
            res.data[13] = 0;

            //Z
            res.data[2] = z.x;
            res.data[6] = z.y;
            res.data[10] = z.z;
            res.data[14] = 0;

            //W
            res.data[3] = res.data[7] = res.data[11] = 0;
            res.data[15] = 1;

            return res;
        }

        /// \brief creates a rotation matrix
        ///
        /// \param rotation
        ///   rotation in degrees
        static const mat4_t<T> rotationMatrixXYZ(const vec3_t<T> rotation)
        {
            mat4_t<T> result(DO_NOT_INITIALIZE);
            float A,B,C,D,E,F,AD,BD;

            A       = cos(toRadians(rotation.x));
            B       = sin(toRadians(rotation.x));
            C       = cos(toRadians(rotation.y));
            D       = sin(toRadians(rotation.y));
            E       = cos(toRadians(rotation.z));
            F       = sin(toRadians(rotation.z));
            AD      =   A * D;
            BD      =   B * D;
            result.data[0]  =   C * E;
            result.data[4]  =  -C * F;
            result.data[8]  =   D;
            result.data[1]  =  BD * E + A * F;
            result.data[5]  = -BD * F + A * E;
            result.data[9]  =  -B * C;
            result.data[2]  = -AD * E + B * F;
            result.data[6]  =  AD * F + B * E;
            result.data[10] =   A * C;
            result.data[3]  =  result.data[7] = result.data[11] = result.data[12] = result.data[13] = result.data[14] = 0;
            result.data[15] =  1;
            return result;
        }

        /// \brief creates a translation matrix
        ///
        /// \param translation
        ///   translation to apply
        static const mat4_t<T> translationMatrix(const vec3& translation){
            mat4_t<T> result(DO_NOT_INITIALIZE);
            result.data[12] = translation.x;
            result.data[13] = translation.y;
            result.data[14] = translation.z;
            result.data[15] = 1.0f;
            result.data[0] = result.data[5] = result.data[10] = 1.0f;
            result.data[4] = result.data[8] = result.data[1] = result.data[9] = result.data[2] =
                             result.data[6] = result.data[3] = result.data[7] = result.data[11] = 0.0f;
            return result;
        }

        /// \brief creates a scale matrix
        ///
        /// \param scale
        ///   scale to use
        static const mat4_t<T> scaleMatrix(const vec3& scale){
            mat4_t<T> result(DO_NOT_INITIALIZE);
            result.data[0] = scale.x;
            result.data[5] = scale.y;
            result.data[10] = scale.z;
            result.data[15] = 1.0f;
            result.data[1] = result.data[2] = result.data[3] = result.data[4] = result.data[6] = result.data[7] = result.data[8]
                           = result.data[9] = result.data[11] = result.data[12] = result.data[13] = result.data[14] = 0.0f;
            return result;
        }

		
		LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(inverse)
            LUA_BIND_FUNCTION(right2Left)
            LUA_BIND_FUNCTION(transformDirection)
            LUA_BIND_FUNCTION(transformPosition)
            LUA_BIND_FUNCTION(submat)
            LUA_BIND_FUNCTION(det)
            LUA_BIND_FUNCTION(transposed)
            LUA_BIND_FUNCTION(translationPart)
            LUA_BIND_FUNCTION(rotationPart)
            LUA_BIND_FUNCTION(setRotationPart)
            LUA_BIND_FUNCTION(normalMatrix)
            LUA_BIND_FUNCTION_NAMED(mulFromLua, "__mul")
		LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(m00)
            LUA_BIND_MEMBER(m01) 
            LUA_BIND_MEMBER(m02)
            LUA_BIND_MEMBER(m03)
            LUA_BIND_MEMBER(m10)
            LUA_BIND_MEMBER(m11)
            LUA_BIND_MEMBER(m12)
            LUA_BIND_MEMBER(m13)
            LUA_BIND_MEMBER(m20)
            LUA_BIND_MEMBER(m21)
            LUA_BIND_MEMBER(m22)
            LUA_BIND_MEMBER(m23)
            LUA_BIND_MEMBER(m30)
            LUA_BIND_MEMBER(m31)
            LUA_BIND_MEMBER(m32)
            LUA_BIND_MEMBER(m33)
		LUA_BIND_VALUE_TYPE_END

        private:
            const mat4_t<T> mulFromLua (const mat4_t<T>& m) const {return *this * m;}
    };

    typedef mat4_t<float> mat4;
};
