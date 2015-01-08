#pragma once

#include "gep/interfaces/inputHandler.h"
#include "gep/container/hashmap.h"

namespace gep
{
    class XInputGamepad : public IGamepad
    {
        friend class InputHandler;

    public:
        XInputGamepad();
        virtual ~XInputGamepad();

        virtual bool	isConnected() const override { return m_checkStateTimer <= 0; };
        virtual int		buttonsPressed() const override { return m_buttonsPressed; };
        virtual int		buttonsTriggered() const override { return m_buttonsTriggered; };
        virtual int		buttonsReleased() const override { return m_buttonsReleased; };
        virtual float	leftTrigger() const override { return m_leftTrigger; }
        virtual float	rightTrigger() const override { return m_rightTrigger; }
        virtual vec2	leftStick() const override { return m_leftStick; }
        virtual vec2	rightStick() const override { return m_rightStick; }
        virtual void	rumbleLeft(float intensity) override;
        virtual void	rumbleLeftFor(float intensity, float time) override;
        virtual void	rumbleRight(float intensity) override;
        virtual void	rumbleRightFor(float intensity, float time) override;

    private:
        void			update(float elapsedTime, DWORD dwUserIndex);
        vec2			clampStick(SHORT sThumbX, SHORT sThumbY, int deadzone, int maximum);
        float			clampStickAxis(SHORT sThumbAxis, int deadzone, int maximum);
        float			clampTrigger(BYTE trigger, int threshold, int maximum);
        void			updateRumble(float elapsedTime);

        XINPUT_STATE	m_XInputState;
        DWORD			m_dwUserIndex;
        float			m_checkStateTimer;
        WORD			m_buttonsPressed;
        WORD			m_buttonsPressedOld;
        WORD			m_buttonsTriggered;
        WORD			m_buttonsReleased;
        float			m_leftTrigger;
        float			m_rightTrigger;
        vec2			m_leftStick;
        vec2			m_rightStick;
        float			m_leftMotorIntensity;
        float			m_leftMotorTimer;
        float			m_rightMotorIntensity;
        float			m_rightMotorTimer;
    };

    class InputHandler : public IInputHandler
    {
    private:
        vec3 m_mouseDelta;

        vec3 m_mouseSensitivity;
        uint32 m_currentFrame;

        struct KeyInfo
        {
            bool isPressed;
            uint32 keyDownFrame;

            inline KeyInfo() : isPressed(false), keyDownFrame(0) {}
        };
        enum { MAX_NUM_KEYS = VK_OEM_CLEAR };
        KeyInfo m_keyMap[MAX_NUM_KEYS];

        XInputGamepad m_pXInputGamepads[XUSER_MAX_COUNT];

        bool m_isAnyPressed;    ///< Whether any key is pressed down at the moment.
        bool m_wasAnyTriggered; ///< Whether any key was triggered this frame.

    public:
        InputHandler();

        virtual void initialize() override;
        virtual void destroy() override;
        virtual void update(float elapsedTime) override;

        /// \brief returns if the given virtual key (VK_) is still pressed
        virtual bool isPressed(uint8 keyCode) override;
        /// \brief returns if the given virtual key (VK_) was pressed this frame
        virtual bool wasTriggered(uint8 keyCode) override;
        /// \brief gets the mouse delta for this frame
        /// \return true if there was mouse movement, false otherwise
        virtual bool getMouseDelta(vec3& mouseDelta) override;

        virtual vec2 getMouseNormalizedScreenPosition() override;
        virtual uvec2 getMouseAbsoluteScreenPosition() override;

        /// \return the gamepad connected to the given user index
        virtual IGamepad* gamepad(int userIndex) override;

        virtual bool isAnyPressed() const override { return m_isAnyPressed; }
        virtual bool wasAnyTriggered() const override { return m_wasAnyTriggered; }
    };
}
