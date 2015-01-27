#pragma once

#include "gep/cameras.h"
#include "gpp/gameObjectSystem.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/quaternion.h"
#include "gpp/cameras.h"

namespace gpp
{
    class CameraComponent : public Component, public gep::ITransform
    {
    public:
        CameraComponent();

        virtual ~CameraComponent();

        virtual void initalize();
        virtual void update(float elapsedMS);
        virtual void destroy();

        virtual void setPosition(const gep::vec3& pos) override;

        virtual void setRotation(const gep::Quaternion& rot) override;

        virtual gep::vec3 getWorldPosition() const override;

        virtual gep::Quaternion getWorldRotation() const  override;

        virtual gep::mat4 getWorldTransformationMatrix() const override;

        virtual gep::Quaternion getRotation()const override;

        virtual gep::vec3 getPosition() const override;

        virtual gep::mat4 getTransformationMatrix() const override;

        void lookAt(const gep::vec3& target);

        gep::Ray getRayForNormalizedScreenPos(const gep::vec2 screenPos) { return m_pCamera->getRayForNormalizedScreenPos(screenPos);};
        gep::Ray getRayForAbsoluteScreenPos(const gep::uvec2 screenPos) { return m_pCamera->getRayForAbsoluteScreenPos(screenPos);};

        void setViewDirection(const gep::vec3& vector);

        void setUpDirection(const gep::vec3& vector);

        void setActive();

        float getViewAngle() const {return m_pCamera->getViewAngle();}

        void setViewAngle(float angle){m_pCamera->setViewAngle(angle);}

        void tilt(float amount){m_pCamera->tilt(amount);}

        void move(const gep::vec3& delta);

        void look(const gep::vec2& delta);

        void setNear(float nearValue){m_pCamera->setNear(nearValue);}

        float getNear(){return m_pCamera->getNear();}

        void setFar(float farValue){m_pCamera->setFar(farValue);}

        float getFar(){return m_pCamera->getFar();}

        float getAspectRatio(){return m_pCamera->getAspectRatio();}
        void setAspectRatio(float ratio){m_pCamera->setAspectRatio(ratio);}

        void setOrthographic(const bool orthographic);
        bool isOrthographic();
        void setWidth(const float width);
        float getWidth();
        void setHeight(const float height);
        float getHeight();

        virtual gep::vec3 getRightDirection() const override;

        virtual gep::vec3 getUpDirection() const override;

        virtual gep::vec3 getViewDirection() const override;

        virtual void setBaseViewDirection(const gep::vec3& direction) override;

        virtual void setBaseOrientation(const gep::Quaternion& orientation) override;

        virtual void setState(State::Enum state) override;

        virtual       gep::ITransform* getParent()       override;
        virtual const gep::ITransform* getParent() const override;
        virtual void setParent(gep::ITransform* parent) override;

        void setViewTarget(gep::ITransform* view);
        void setUpTarget(gep::ITransform* view);

        void unsetViewTarget() { m_viewTarget.setParent(nullptr); };
        void unsetUpTarget() { m_upTarget.setParent(nullptr); };

        void setViewTargetPositionOffset(const gep::vec3& positionOffset) {
            m_viewTarget.setPosition(positionOffset);
        };
        void setUpTargetPositionOffset(const gep::vec3& positionOffset) { m_upTarget.setPosition(positionOffset); };


        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(setPosition)
            LUA_BIND_FUNCTION(lookAt)
            LUA_BIND_FUNCTION(getViewDirection)
            LUA_BIND_FUNCTION(setViewDirection)
            LUA_BIND_FUNCTION(getRightDirection)
            LUA_BIND_FUNCTION(getUpDirection)
            LUA_BIND_FUNCTION(setUpDirection)
            LUA_BIND_FUNCTION(getPosition)
            LUA_BIND_FUNCTION(getWorldPosition)
            LUA_BIND_FUNCTION(getRotation)
            LUA_BIND_FUNCTION(getWorldRotation)
            LUA_BIND_FUNCTION(setViewTarget)
            LUA_BIND_FUNCTION(setUpTarget)
            LUA_BIND_FUNCTION(setViewTargetPositionOffset)
            LUA_BIND_FUNCTION(setUpTargetPositionOffset)
            LUA_BIND_FUNCTION(setRotation)
            LUA_BIND_FUNCTION(setBaseOrientation)
            LUA_BIND_FUNCTION(setBaseViewDirection)
            LUA_BIND_FUNCTION(setState)
            LUA_BIND_FUNCTION(getViewAngle)
            LUA_BIND_FUNCTION(setViewAngle)
            LUA_BIND_FUNCTION(tilt)
            LUA_BIND_FUNCTION(move)
            LUA_BIND_FUNCTION(look)
            LUA_BIND_FUNCTION(getRayForNormalizedScreenPos)
            LUA_BIND_FUNCTION(getRayForAbsoluteScreenPos)
            LUA_BIND_FUNCTION(setNear)
            LUA_BIND_FUNCTION(getNear)
            LUA_BIND_FUNCTION(setFar)
            LUA_BIND_FUNCTION(getFar)
            LUA_BIND_FUNCTION(setAspectRatio)
            LUA_BIND_FUNCTION(getAspectRatio)
            LUA_BIND_FUNCTION(isOrthographic)
            LUA_BIND_FUNCTION(setOrthographic)
            LUA_BIND_FUNCTION(setWidth)
            LUA_BIND_FUNCTION(getWidth)
            LUA_BIND_FUNCTION(setHeight)
            LUA_BIND_FUNCTION(getHeight)
            LUA_BIND_FUNCTION(unsetUpTarget)
            LUA_BIND_FUNCTION(unsetViewTarget)
        LUA_BIND_REFERENCE_TYPE_END ;

    private:
        CameraLookAtHorizon* m_pCamera;
        gep::Transform m_viewTarget;
        gep::Transform m_upTarget;

    };

    template<>
    struct ComponentMetaInfo<CameraComponent>
    {
        static const char* name(){ return "CameraComponent"; }
        static const gep::int32 initializationPriority() { return 0; }
        static const gep::int32 updatePriority() { return 23; }
    };
}
