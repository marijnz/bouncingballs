#include "stdafx.h"
#include "gep/math3d/vec3.h"
#include "gep/math3d/vec4.h"

#include "gep/globalManager.h"
#include "gpp/cameras.h"


gpp::Camera::Camera() :
    m_aspectRatio(16.0f / 9.0f),
    m_viewAngleInDegrees(60),
    m_near(0.1f),
    m_far(10000.0f),
    m_orthographic(false)
{

}

const gep::mat4 gpp::Camera::getProjectionMatrix() const
{
    if(m_orthographic)
    {
        float distance = (m_far - m_near) / 8; // use 1/8 view distance as adjacent length to calculate frustum
        float viewAngleHalfInRad = gep::toRadians(m_viewAngleInDegrees/2);
        float tangens = gep::sin(viewAngleHalfInRad) / gep::cos(viewAngleHalfInRad);
        float topDist = tangens * distance; // tan * adjacent = opposite
        float rightDist = topDist / m_aspectRatio;
        return gep::mat4::ortho(-topDist, topDist, -rightDist, rightDist, m_near, m_far);
    }
    else
    {
        return gep::mat4::projectionMatrix(m_viewAngleInDegrees, m_aspectRatio, m_near, m_far);
    }
}


const gep::mat4 gpp::Camera::getViewMatrix() const
{
    return (gep::mat4::translationMatrix(m_position) * m_rotation.toMat4()).inverse();
}


gep::Ray gpp::Camera::getRayForNormalizedScreenPos(const gep::vec2& screenPos)
{
    using namespace gep;
    vec3 from;
    vec3 to;
    Ray result = Ray(from, to);

    if (isOrthographic())
    {
        float distance = (m_far - m_near) / 8; // use 1/8 view distance as adjacent length to calculate frustum
        float viewAngleHalfInRad = gep::toRadians(m_viewAngleInDegrees/2);
        float tangens = gep::sin(viewAngleHalfInRad) / gep::cos(viewAngleHalfInRad);
        float topDist = tangens * distance; // tan * adjacent = opposite
        float rightDist = m_aspectRatio * topDist;

        gep::vec3 offsetOnViewPlane = gep::vec3(0,0,0);
        offsetOnViewPlane.x = screenPos.x * rightDist;
        offsetOnViewPlane.y = screenPos.y * topDist; // screen coords have y upwards, but in gep, z is up direction

        from = getWorldPosition() + getRightDirection() * offsetOnViewPlane.x;
        from = from + getUpDirection() * offsetOnViewPlane.y;

        result = Ray(from, getViewDirection());
    }
    else
    {
        mat4 inverseViewProjeciton = getProjectionMatrix() * getViewMatrix();
        inverseViewProjeciton = inverseViewProjeciton.inverse();
        vec4 temp = inverseViewProjeciton * vec4(screenPos.x, screenPos.y, 1, 1);
        to = vec3(temp.x, temp.y, temp.z).normalized();

        result = Ray(getWorldPosition(), to);
    }

    return result;
}

gep::Ray gpp::Camera::getRayForAbsoluteScreenPos( const gep::uvec2& screenPos )
{
    gep::vec2 normalizedPos = g_globalManager.getRenderer()->toNormalizedScreenPosition(screenPos);
    return getRayForNormalizedScreenPos(normalizedPos);
}
///////////////////////////////////////////////////////////////////////

gpp::CameraLookAtHorizon::CameraLookAtHorizon() :
    m_viewDir(),
    m_tilt(0),
    m_upVector(0,0,1)
{
}

void gpp::CameraLookAtHorizon::tilt(float amount)
{
    m_tilt += amount;
}

const gep::mat4  gpp::CameraLookAtHorizon::getViewMatrix() const
{
    gep::vec3 up = gep::Quaternion(m_viewDir, -m_tilt).toMat3() * this->getUpDirection();
    auto pos = this->getWorldPosition();
    return gep::mat4::lookAtMatrix(pos, pos + m_viewDir, up);
}

void gpp::CameraLookAtHorizon::setViewVector(const gep::vec3& vector)
{
    m_viewDir = vector.normalized();
}


void gpp::CameraLookAtHorizon::lookAt(const gep::vec3& target)
{
    if (target.epsilonCompare(this->getWorldPosition()))
    {
        m_viewDir = gep::vec3(0, 0, 0);
    }
    else
    {
        m_viewDir = (target - this->getWorldPosition()).normalized();
    }
}



void gpp::CameraLookAtHorizon::look(const gep::vec2& delta)
{
    gep::vec3 right = this->getRightDirection();
    auto qy = gep::Quaternion(right, -delta.y);
    auto qx = gep::Quaternion(this->getUpDirection(), -delta.x);
    m_viewDir = (qy * qx).toMat3() * m_viewDir;


    gep::vec3 up = gep::Quaternion(m_viewDir, -m_tilt).toMat3() * this->getUpDirection();
    auto pos = this->getPosition();

    auto rot = gep::Quaternion( gep::mat4::lookAtMatrix(pos, pos + m_viewDir, up).rotationPart());
    this->setRotation(rot);

}

void gpp::CameraLookAtHorizon::move(const gep::vec3& delta)
{
    this->setPosition(getPosition() + delta);
}

void gpp::CameraLookAtHorizon::setUpVector( const gep::vec3& vector )
{
    m_upVector = vector.normalized();
}

gep::vec3 gpp::CameraLookAtHorizon::getRightDirection() const
{
    return m_viewDir.cross(m_upVector);
}

gep::vec3 gpp::CameraLookAtHorizon::getUpDirection() const
{
    return m_upVector;
}

gep::vec3 gpp::CameraLookAtHorizon::getViewDirection() const
{
    return m_viewDir;
}
