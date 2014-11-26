#pragma once


#include "gep/interfaces/renderer.h"
#include "gep/gepmodule.h"
#include "gep/math3d/quaternion.h"
#include "gep/math3d/vec3.h"
#include "gep/interfaces/game.h"

namespace gep
{
    class GEP_API Camera : public ICamera
    {
    public:
        Camera();
        virtual ~Camera() {}

        virtual const mat4 getProjectionMatrix() const override;

        float getViewAngle() const { return m_viewAngleInDegrees; }
        void setViewAngle(float degrees) { m_viewAngleInDegrees = degrees; }

        float getAspectRatio() const { return m_aspectRatio; }
        void setAspectRatio(float value) { m_aspectRatio = value; }

        float getNear() const { return m_near; }
        void setNear(float value) { m_near = value; }

        float getFar() const { return m_far; }
        void setFar(float value) { m_far = value; }


    protected:
        float m_viewAngleInDegrees;
        float m_aspectRatio;
        float m_near;
        float m_far;
    };

    class GEP_API FreeCamera : public Camera
    {
    protected:
        Quaternion m_rotation;
        vec3 m_position;

    public:
        FreeCamera();
        virtual ~FreeCamera(){}

        /// \brief sets the camera position
        inline void setPosition(const vec3& position) { m_position = position; }

        /// \brief gets the camera position
        inline const vec3 getPosition() const { return m_position; }

        /// \brief makes the camera look around
        virtual void look(vec2 delta);

        /// \brief rotates the camera around the view axis
        virtual void tilt(float amount);

        /// \briefs moves the camera
        virtual void move(vec3 delta);

        virtual const mat4 getViewMatrix() const override;

    };

    class GEP_API FreeCameraHorizon : public FreeCamera
    {
    protected:
        vec3 m_viewDir;
        vec2 m_rotation;
        float m_tilt;

    public:
        FreeCameraHorizon();
        virtual ~FreeCameraHorizon(){}

        virtual void look(vec2 delta) override;

        virtual void tilt(float amount) override;

        virtual void move(vec3 delta) override;

        const mat4 getViewMatrix() const override;
    };

    class GEP_API ThirdPersonCamera : public Camera
    {
    public:
        enum FollowMode
        {
            Direct,
            Smooth
        };

    private:
        const vec3 m_offset;
        vec3 m_position;
        mat4 m_matrixToFollow;
        mat4 m_fixedRotation;
        FollowMode m_followMode;

    public:
        ThirdPersonCamera(vec3 offset);
        virtual ~ThirdPersonCamera(){}

        void follow(const mat4& matrixToFollow);

        FollowMode getFollowMode() const;
        void setFollowMode(FollowMode followMode);

        virtual const mat4 getViewMatrix() const override;
    };
}

