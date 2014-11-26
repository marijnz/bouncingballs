#pragma once
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"

namespace gep
{
    class GEP_API ITransform
    {
    public:
        virtual ~ITransform() {}

        virtual void setPosition(const vec3& pos) = 0;
        virtual void setRotation(const Quaternion& rot) = 0;
        virtual void setBaseOrientation(const Quaternion& orientation) = 0;
        virtual void setBaseViewDirection(const vec3& direction) = 0;

        virtual mat4 getWorldTransformationMatrix() const = 0;
        virtual vec3 getWorldPosition() const = 0;
        virtual Quaternion getWorldRotation() const = 0;

        virtual mat4 getTransformationMatrix() const = 0;
        virtual vec3 getPosition() const = 0;
        virtual Quaternion getRotation() const = 0;

        virtual vec3 getViewDirection() const = 0;
        virtual vec3 getUpDirection() const = 0;
        virtual vec3 getRightDirection() const = 0;

        virtual void setParent(ITransform* parent) = 0;
        virtual ITransform* getParent() = 0;
        virtual const ITransform* getParent() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(setBaseOrientation)
            LUA_BIND_FUNCTION(setBaseViewDirection)
            LUA_BIND_FUNCTION(getWorldTransformationMatrix)
            LUA_BIND_FUNCTION(getWorldPosition)
            LUA_BIND_FUNCTION(getWorldRotation)
            LUA_BIND_FUNCTION(getTransformationMatrix)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(getViewDirection)
            LUA_BIND_FUNCTION(getUpDirection)
            LUA_BIND_FUNCTION(getRightDirection)
            LUA_BIND_FUNCTION(setParent)
            LUA_BIND_FUNCTION_PTR(static_cast<ITransform*(ITransform::*)()>(&getParent), "getParent")
        LUA_BIND_REFERENCE_TYPE_END;
    };

    GEP_API ITransform* getIdentityTransform();

    class GEP_API Transform : public ITransform
    {
    public:
        Transform() :
            m_position(),
            m_rotation(),
            m_baseOrientation(),
            m_pParent(getIdentityTransform())
        {
        }
        virtual ~Transform() { m_pParent = nullptr; }

        virtual void setPosition(const vec3& pos) override { m_position = pos; }
        virtual void setRotation(const Quaternion& rot) override { m_rotation = rot; }
        virtual void setBaseOrientation(const Quaternion& orientation) override { m_baseOrientation = orientation; }
        virtual void setBaseViewDirection(const vec3& direction) override { m_baseOrientation = Quaternion(direction, vec3(0, 1, 0)); }

        virtual vec3 getWorldPosition() const override
        {
            return getWorldTransformationMatrix().translationPart();
        }
        virtual Quaternion getWorldRotation() const override
        {
            return m_pParent->getWorldRotation() * m_rotation;
        }

        virtual mat4 getWorldTransformationMatrix() const override
        {
            return m_pParent->getWorldTransformationMatrix() * getTransformationMatrix();
        }
        virtual vec3 getViewDirection() const override { return (getWorldRotation() * m_baseOrientation).toMat3() * vec3(0, 1, 0); }
        virtual vec3 getUpDirection() const override { return  (getWorldRotation()  * m_baseOrientation).toMat3() * vec3(0, 0, 1); }
        virtual vec3 getRightDirection() const override { return (getWorldRotation() * m_baseOrientation).toMat3() * vec3(1, 0, 0); }

        virtual void setParent(ITransform* parent) override
        {
            GEP_ASSERT(parent, "The parent of a transform needs to be valid! "
                       "If you want to detach this instance, "
                       "set the parent to an identity transform "
                       "(e.g. gep::getIdentityTransform() in C++ or IdentityTransform in Lua).");
            m_pParent = parent;
        }

        virtual ITransform* getParent() override
        {
            return m_pParent;
        }

        virtual const ITransform* getParent() const override
        {
            return m_pParent;
        }

        virtual mat4 getTransformationMatrix() const override
        {
            return mat4::translationMatrix(m_position) * m_rotation.toMat4();
        }

        virtual vec3 getPosition() const override
        {
            return m_position;
        }

        virtual Quaternion getRotation() const override
        {
            return m_rotation;
        }

    protected:
        vec3 m_position;
        Quaternion m_rotation;
        Quaternion m_baseOrientation;
        ITransform* m_pParent;

    };
}
