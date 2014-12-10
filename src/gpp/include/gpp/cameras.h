#pragma once


#include "gep/interfaces/renderer.h"
#include "gep/gepmodule.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/transform.h"
#include "gep/interfaces/game.h"
#include "gep/cameras.h"

namespace gpp
{
    
    class GPP_API Camera : public gep::ICamera, public gep::Transform
    {
    public:
        Camera();
        virtual ~Camera() {}

        virtual const gep::mat4 getProjectionMatrix() const override;
        virtual const gep::mat4 getViewMatrix() const override; 

        float getViewAngle() const { return m_viewAngleInDegrees; }
        void setViewAngle(float degrees) { m_viewAngleInDegrees = degrees; }

        float getAspectRatio() const { return m_aspectRatio; }
        void setAspectRatio(float value) { m_aspectRatio = value; }

        float getNear() const { return m_near; }
        void setNear(float value) { m_near = value; }

        float getFar() const { return m_far; }
        void setFar(float value) { m_far = value; }

        float getWidth() const { return m_width; }
        void setWidth(float value) { m_width = value; m_height = m_width / m_aspectRatio; }

        float getHeight() const { return m_height; }
        void setHeight(float value) { m_height = value; m_width = m_height * m_aspectRatio; }

        void setPosition(const gep::vec3& pos){m_position = pos;}

        void setOrthographic(const bool orthographic) { m_orthographic = orthographic; };
        bool isOrthographic() { return m_orthographic; };

        gep::Ray getRayForNormalizedScreenPos(const gep::vec2& screenPos);
        gep::Ray getRayForAbsoluteScreenPos(const gep::uvec2& screenPos);

    protected:
        float m_viewAngleInDegrees;
        float m_aspectRatio;
        float m_near;
        float m_far;
        bool m_orthographic;
        float m_width;
        float m_height;
        gep::Quaternion m_rotation;
        gep::vec3 m_position;
    };


    class GPP_API CameraLookAtHorizon : public Camera
    {
    public:
        CameraLookAtHorizon();
        virtual void tilt(float amount);

        const gep::mat4 getViewMatrix() const override;

        void lookAt(const gep::vec3& target);

        void setViewVector(const gep::vec3& vector);

        void setUpVector(const gep::vec3& vector);

        virtual gep::vec3 getViewDirection() const override;
        
        virtual gep::vec3 getRightDirection() const override;

        virtual gep::vec3 getUpDirection() const override ;

        virtual void setPosition(const gep::vec3& pos) override{gep::Transform::setPosition(pos);}

        void look(const gep::vec2& delta);

        void move(const gep::vec3& delta);


    protected:
        gep::vec3 m_upVector;
        gep:: vec3 m_viewDir;

        float m_tilt;
    };

}

