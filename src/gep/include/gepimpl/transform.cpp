#include "stdafx.h"
#include "gep/math3d/transform.h"

#include "gep/globalManager.h"
#include "gep/interfaces/logging.h"

namespace gep
{
    class DefaultIdentityTransform : public gep::ITransform
    {
    public:
        virtual void setPosition(const vec3& pos) override
        {
            g_globalManager.getLogging()->logWarning("Ignoring attempt to set the position of an identity transform.");
        }

        virtual vec3 getPosition() const override
        {
            return vec3();
        }

        virtual vec3 getWorldPosition() const override
        {
            return vec3();
        }

        virtual void setRotation(const Quaternion& rot) override
        {
            g_globalManager.getLogging()->logWarning("Ignoring attempt to set the rotation of an identity transform.");
        }

        virtual Quaternion getRotation() const override
        {
            return Quaternion();
        }

        virtual Quaternion getWorldRotation() const override
        {
            return Quaternion();
        }

        virtual vec3 getViewDirection() const override
        {
            return vec3(0, 1, 0);
        }

        virtual vec3 getUpDirection() const override
        {
            return vec3(0, 0, 1);
        }

        virtual vec3 getRightDirection() const override
        {
            return vec3(1, 0, 0);
        }

        virtual void setBaseOrientation(const Quaternion& orientation) override
        {
            g_globalManager.getLogging()->logWarning("Ignoring attempt to set the base orientation of an identity transform.");
        }

        virtual void setBaseViewDirection(const vec3& direction) override
        {
            g_globalManager.getLogging()->logWarning("Ignoring attempt to set the base view direction of an identity transform.");
        }

        virtual ITransform* getParent() override
        {
            return this;
        }

        virtual const ITransform* getParent() const override
        {
            return this;
        }

        virtual mat4 getTransformationMatrix() const override
        {
            return mat4::identity();
        }

        virtual mat4 getWorldTransformationMatrix() const override
        {
            return mat4::identity();
        }

        virtual void setParent(ITransform* parent) override
        {
            g_globalManager.getLogging()->logWarning("Ignoring attempt to set the parent of an identity transform.");
        }
    };
}

gep::ITransform* gep::getIdentityTransform()
{
    static DefaultIdentityTransform instance;
    return &instance;
}
