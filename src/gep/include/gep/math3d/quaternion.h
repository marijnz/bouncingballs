#pragma once

#include "gep/math3d/mat3.h"
#include "gep/math3d/mat4.h"
#include "gep/math3d/algorithm.h"

#include "gep/math3d/vec4.h"
#include "gep/interfaces/scripting.h"

namespace gep
{
    template <typename T>
    struct Quaternion_t
    {
        union
        {
            struct
            {
                T x,y,z,angle;
            };
            T data[4];
        };
        typedef T component_t;

        /// \brief default constructor
        Quaternion_t()
        {
            this->x = 0;
            this->y = 0;
            this->z = 0;
            this->angle = 1;
        }

        /// \brief constructor without initialization
        Quaternion_t(DoNotInitialize arg) {}

        /// \brief constructor
        /// \param axis
        ///   the rotation axis
        /// \param angle
        ///   the amount of rotation around axis (in degrees)
        Quaternion_t(vec3_t<T> axis, T angle)
        {
            angle = toRadians(angle) / 2;
            float temp = sin(angle);

            this->x = axis.x * temp;
            this->y = axis.y * temp;
            this->z = axis.z * temp;
            this->angle = cos(angle);
        }

        Quaternion_t(vec3_t<T> axis1, vec3_t<T> axis2)
        {
           if (axis1.epsilonCompare(-axis2))
           {
               // TODO Make this quaternion a 180 degree rotation!!!
               // The axes are the opposite of one another
               this->x = 0;
               this->y = 0;
               this->z = 1;
               this->angle = 0;
               return;
           }

           T cosTheta = axis1.dot(axis2);
           T theta = acos(cosTheta);

           vec3_t<T> nAxis = axis1.cross(axis2);
           if(gep::epsilonCompare<T>(nAxis.squaredLength(), 0)) // Axis 1 and Axis2 are parallel!
           {
               this->x = 0;
               this->y = 0;
               this->z = 0;
               this->angle = 1;
               return;
           }

           angle = theta /2;
           float temp = sin(angle);

           this->x = nAxis.x * temp;
           this->y = nAxis.y * temp;
           this->z = nAxis.z * temp;
           this->angle = cos(angle);

        }

        /**
        * constructs a quaternion from a rotation matrix
        */
        Quaternion_t(mat3_t<T> rot){
            T trace = 1.0f + rot.data[0] + rot.data[4] + rot.data[8];
            if(trace > GetEpsilon<T>::value())
            {
                T S = gep::sqrt(trace) * 2.0f;
                this->x = ( rot.data[5] - rot.data[7] ) / S;
                this->y = ( rot.data[6] - rot.data[2] ) / S;
                this->z = ( rot.data[1] - rot.data[3] ) / S;
                this->angle = T(0.25) * S;
            }
            else
            {
                if( rot.data[0] > rot.data[4] && rot.data[0] > rot.data[8] ) //Column 0:
                {
                    T S = sqrt( 1.0f + rot.data[0] - rot.data[4] - rot.data[8] ) * 2;
                    this->x = 0.25f * S;
                    this->y = ( rot.data[3] + rot.data[1] ) / S;
                    this->z = ( rot.data[2] + rot.data[6] ) / S;
                    this->angle = ( rot.data[5] - rot.data[7] ) / S;
                }
                else if( rot.data[4] > rot.data[8] ) // Column 1:
                {
                    float S = sqrt( 1.0f + rot.data[4] - rot.data[0] - rot.data[8] ) * 2.0f;
                    this->x = ( rot.data[3] + rot.data[1] ) / S;
                    this->y = 0.25f * S;
                    this->z = ( rot.data[7] + rot.data[5] ) / S;
                    this->angle = ( rot.data[6] - rot.data[2] ) / S;
                }
                else
                {
                    float S = sqrt( 1.0f + rot.data[8] - rot.data[0] - rot.data[4] ) * 2.0f;
                    this->x = ( rot.data[2] + rot.data[6] ) / S;
                    this->y = ( rot.data[7] + rot.data[5] ) / S;
                    this->z = 0.25f * S;
                    this->angle = ( rot.data[1] - rot.data[3] ) / S;
                }
            }
        }

        /// \brief returns the normalized quaternion
        const Quaternion_t<T> normalized() const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            auto length = gep::sqrt(x * x + y * y + z * z + angle * angle);
            if(gep::epsilonCompare<decltype(length)>(length, 0.0f))
            {
                GEP_ASSERT(false, "Cannot normalize an invalid quaternion!");
            }

            res.x = x / length;
            res.y = y / length;
            res.z = z / length;
            res.angle = angle / length;
            return res;
        }

        /// \brief returns the inverse of this quaternion
        const Quaternion_t<T> inverse() const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            res.x = x * -1;
            res.y = y * -1;
            res.z = z * -1;
            res.angle = angle;
            return res;
        }

        /// \brief converts this quaternion into a 4x4 rotation matrix
        const mat4_t<T> toMat4() const
        {
            GEP_ASSERT(isValid(), "quaternion is not valid");
            mat4_t<T> mat(DO_NOT_INITIALIZE);
            Quaternion norm = normalized();
            auto xx  = norm.x * norm.x;
            auto xy  = norm.x * norm.y;
            auto xz  = norm.x * norm.z;
            auto xw  = norm.x * norm.angle;
            auto yy  = norm.y * norm.y;
            auto yz  = norm.y * norm.z;
            auto yw  = norm.y * norm.angle;
            auto zz  = norm.z * norm.z;
            auto zw  = norm.z * norm.angle;

            mat.m00 = T(1) - T(2) * ( yy + zz );
            mat.m01  =        T(2) * ( xy - zw );
            mat.m02  =        T(2) * ( xz + yw );

            mat.m10  =        T(2) * ( xy + zw );
            mat.m11  = T(1) - T(2) * ( xx + zz );
            mat.m12  =        T(2) * ( yz - xw );

            mat.m20  =        T(2) * ( xz - yw );
            mat.m21  =        T(2) * ( yz + xw );
            mat.m22  = T(1) - T(2) * ( xx + yy );

            mat.data[3]  = mat.data[7] = mat.data[11] = mat.data[12] = mat.data[13] = mat.data[14] = T(0);
            mat.data[15] = T(1);
            return mat;
        }

        /// \brief converts this quaternion into a 3x3 rotation matrix
        const mat3_t<T> toMat3() const
        {
            mat3_t<T> mat(DO_NOT_INITIALIZE);
            auto norm = this->normalized();
            auto xx  = norm.x * norm.x;
            auto xy  = norm.x * norm.y;
            auto xz  = norm.x * norm.z;
            auto xw  = norm.x * norm.angle;
            auto yy  = norm.y * norm.y;
            auto yz  = norm.y * norm.z;
            auto yw  = norm.y * norm.angle;
            auto zz  = norm.z * norm.z;
            auto zw  = norm.z * norm.angle;
            mat.m00 = T(1) - T(2) * ( yy + zz );
            mat.m01 =        T(2) * ( xy - zw );
            mat.m02 =        T(2) * ( xz + yw );
            mat.m10 =        T(2) * ( xy + zw );
            mat.m11 = T(1) - T(2) * ( xx + zz );
            mat.m12 =        T(2) * ( yz - xw );
            mat.m20 =        T(2) * ( xz - yw );
            mat.m21 =        T(2) * ( yz + xw );
            mat.m22 = T(1) - T(2) * ( xx + yy );
            return mat;
        }

        /// \brief * operator for muliplying two quaternions
        const Quaternion_t<T> operator * (const Quaternion_t<T>& rh) const
        {
            Quaternion res(DO_NOT_INITIALIZE);
            res.x     = this->angle * rh.x     + this->x * rh.angle + this->y * rh.z - this->z * rh.y;
            res.y     = this->angle * rh.y     + this->y * rh.angle + this->z * rh.x - this->x * rh.z;
            res.z     = this->angle * rh.z     + this->z * rh.angle + this->x * rh.y - this->y * rh.x;
            res.angle = this->angle * rh.angle - this->x * rh.x     - this->y * rh.y - this->z * rh.z;
            return res;
        }

        /// \brief * operator for multiplying with a scalar
        const Quaternion_t<T> operator * (const float rh) const
        {
            Quaternion_t<T> res(DO_NOT_INITIALIZE);
            res.x = this->x * rh;
            res.y = this->y * rh;
            res.z = this->z * rh;
            res.angle = this->angle * rh;
            return res;
        }

        bool epsilonCompare(const Quaternion_t& rhs)
        {
            return gep::epsilonCompare(this->x, rhs.x) &&  gep::epsilonCompare(this->y, rhs.y) &&  gep::epsilonCompare(this->z, rhs.z) &&  gep::epsilonCompare(this->angle, rhs.angle);
        }

        /// \brief checks if this quaternion is valid or not
        bool isValid() const
        {
            //nan checks
            return (!(x != x) && !(y != y) && !(z != z) && !(angle != angle)
                /*&& x != float.infinity && y != float.infinity && z != float.infinity && angle != float.infinity*/);
        }

        bool isIdentity() const
        {
            using gep::epsilonCompare;
            return epsilonCompare(x, 0.0f)
                && epsilonCompare(y, 0.0f)
                && epsilonCompare(z, 0.0f)
                && epsilonCompare(angle, 1.0f);
        }

        const vec4_t<T> toVec4()
        {
            return vec4_t<T>(this->x,this->y,this->z,this->angle);
        }

        const Quaternion_t<T> Integrate(const vec3_t<T>& angularVelocity, T deltaTime) const
        {
            Quaternion_t<T> deltaQ(DO_NOT_INITIALIZE);
            auto scaledAngularVelocity = angularVelocity * (deltaTime * T(0.5));
            auto squaredVelocityLength = scaledAngularVelocity.squaredLength();

            T s = 1;

            if(squaredVelocityLength * squaredVelocityLength / T(24) < GetEpsilon<T>::value())
            {
                deltaQ.angle = T(1) - squaredVelocityLength / T(2);
                s = T(1) - squaredVelocityLength / T(6);
            }
            else
            {
                auto velocityLength = gep::sqrt(squaredVelocityLength);

                deltaQ.angle = gep::cos(velocityLength);
                s = gep::sin(velocityLength) / velocityLength;
            }

            deltaQ.x = scaledAngularVelocity.x * s;
            deltaQ.y = scaledAngularVelocity.y * s;
            deltaQ.z = scaledAngularVelocity.z * s;

            return deltaQ * (*this);
        }

        static const Quaternion_t<T> fromMat4(mat4 mat)
        {
            // http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/
            // TODO: This calculation has to be checked! Strange behaviour with some (small?) rotation angles
            Quaternion_t<T> q = Quaternion_t<T>();
            auto a = mat.data;

            float trace = a[0] + a[5] + a[10]; // I removed + 1.0f; see discussion with Ethan
            if( trace > 0 ) {// I changed M_EPSILON to 0
                float s = 0.5f / sqrtf(trace+ 1.0f);
                q.angle = 0.25f / s;
                q.x = ( a[6] - a[9] ) * s;
                q.y = ( a[8] - a[2] ) * s;
                q.z = ( a[1] - a[4] ) * s;
            } else {
                if ( a[0] > a[5] && a[0] > a[10] ) {
                    float s = 2.0f * sqrtf( 1.0f + a[0] - a[5] - a[10]);
                    q.angle = (a[6] - a[9] ) / s;
                    q.x = 0.25f * s;
                    q.y = (a[4] + a[2] ) / s;
                    q.z = (a[8] + a[2] ) / s;
                } else if (a[5] > a[10]) {
                    float s = 2.0f * sqrtf( 1.0f + a[5] - a[0] - a[10]);
                    q.angle = (a[8] - a[2] ) / s;
                    q.x = (a[4] + a[1] ) / s;
                    q.y = 0.25f * s;
                    q.z = (a[9] + a[6] ) / s;
                } else {
                    float s = 2.0f * sqrtf( 1.0f + a[10] - a[0] - a[5] );
                    q.angle = (a[1] - a[4] ) / s;
                    q.x = (a[8] + a[2] ) / s;
                    q.y = (a[9] + a[6] ) / s;
                    q.z = 0.25f * s;
                }
            }
            return q;
        }

        LUA_BIND_VALUE_TYPE_BEGIN
            LUA_BIND_FUNCTION(normalized)
            LUA_BIND_FUNCTION(inverse)
            LUA_BIND_FUNCTION_NAMED(inverse, "negate")
            LUA_BIND_FUNCTION_NAMED(inverse, "__unm")
            LUA_BIND_FUNCTION_NAMED(mulFromScript, "mul")
            LUA_BIND_FUNCTION_NAMED(mulFromScript, "__mul")
            LUA_BIND_FUNCTION_NAMED(mulScalarFromScript, "mulScalar")
            LUA_BIND_FUNCTION(toMat3)
            LUA_BIND_FUNCTION(isValid)
            //LUA_BIND_FUNCTION(Integrate) // Not exactly sure what this does...
        LUA_BIND_VALUE_TYPE_MEMBERS
            LUA_BIND_MEMBER(x)
            LUA_BIND_MEMBER(y)
            LUA_BIND_MEMBER(z)
            LUA_BIND_MEMBER(angle)
        LUA_BIND_VALUE_TYPE_END;

    private:
        const Quaternion_t<T> mulFromScript(const Quaternion_t<T>& rh) const { return *this * rh; }
        const Quaternion_t<T> mulScalarFromScript(T rh) const { return *this * rh; }
    };

    typedef Quaternion_t<float> Quaternion;
};
