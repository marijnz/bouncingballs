#pragma once

#include "gep/types.h"
#include "gep/interfaces/subsystem.h"
#include "gep/math3d/vec3.h"

// For convenience
#include "gep/input/windowsVirtualKeyCodes.h"

namespace gep
{
    class IGamepad
    {
    public:
        virtual ~IGamepad() {}

        /// \brief returns if the gamepad is connected
        virtual bool isConnected() const = 0;
        /// \brief returns the buttons that are pressed, triggered, or released
        virtual int buttonsPressed() const = 0;
        virtual int buttonsTriggered() const = 0;
        virtual int buttonsReleased() const = 0;
        /// \brief returns the left and right trigger positions
        virtual float leftTrigger() const = 0;
        virtual float rightTrigger() const = 0;
        /// \brief returns the left and right analog stick positions
        virtual vec2 leftStick() const = 0;
        virtual vec2 rightStick() const = 0;
        /// \brief controls the left and right rumble motors
        virtual void rumbleLeft(float intensity) = 0;
        virtual void rumbleLeftFor(float intensity, float time) = 0;
        virtual void rumbleRight(float intensity) = 0;
        virtual void rumbleRightFor(float intensity, float time) = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(isConnected)
            LUA_BIND_FUNCTION(buttonsPressed)
            LUA_BIND_FUNCTION(buttonsTriggered)
            LUA_BIND_FUNCTION(buttonsReleased)
            LUA_BIND_FUNCTION(leftTrigger)
            LUA_BIND_FUNCTION(rightTrigger)
            LUA_BIND_FUNCTION(leftStick)
            LUA_BIND_FUNCTION(rightStick)
            LUA_BIND_FUNCTION(rumbleLeft)
            LUA_BIND_FUNCTION(rumbleLeftFor)
            LUA_BIND_FUNCTION(rumbleRight)
            LUA_BIND_FUNCTION(rumbleRightFor)
        LUA_BIND_REFERENCE_TYPE_END;
    };

    class IInputHandler : public ISubsystem
    {
    public:
        /// \brief returns if the given key is still pressed
        virtual bool isPressed(uint8 keyCode) = 0;
        /// \brief returns if the given key was pressed this frame
        virtual bool wasTriggered(uint8 keyCode) = 0;
        /// \brief gets the mouse delta for this frame
        /// \return true if there was mouse movement, false otherwise
        virtual bool getMouseDelta(vec3& mouseDelta) = 0;

        virtual vec2 getMouseNormalizedScreenPosition() = 0;
        virtual uvec2 getMouseAbsoluteScreenPosition() = 0;

        /// \return the gamepad connected to the given user index
        virtual IGamepad* gamepad(int userIndex) = 0;

        inline vec3 getMouseDelta()
        {
            vec3 result;
            getMouseDelta(result);
            return result;
        }

        inline bool hasMouseDelta()
        {
            vec3 unused;
            return getMouseDelta(unused);
        }

        virtual bool isAnyPressed() const = 0;
        virtual bool wasAnyTriggered() const = 0;

        LUA_BIND_REFERENCE_TYPE_BEGIN
            LUA_BIND_FUNCTION(isPressed)
            LUA_BIND_FUNCTION(isAnyPressed)
            LUA_BIND_FUNCTION(wasTriggered)
            LUA_BIND_FUNCTION(wasAnyTriggered)
            LUA_BIND_FUNCTION_PTR(static_cast<vec3(IInputHandler::*)()>(&getMouseDelta), "getMouseDelta")
            LUA_BIND_FUNCTION(hasMouseDelta)
            LUA_BIND_FUNCTION(getMouseNormalizedScreenPosition)
            LUA_BIND_FUNCTION(getMouseAbsoluteScreenPosition)
            LUA_BIND_FUNCTION(gamepad)
        LUA_BIND_REFERENCE_TYPE_END
    };
}
