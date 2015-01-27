#include "stdafx.h"
#include "gepimpl/subsystems/inputHandler.h"
#include "gep/globalManager.h"
#include "gep/interfaces/updateFramework.h"
#include "gepimpl/subsystems/renderer/renderer.h"

gep::XInputGamepad::XInputGamepad() :
    m_dwUserIndex(0),
    m_checkStateTimer(0.f),
    m_buttonsPressed(0),
    m_buttonsPressedOld(0),
    m_buttonsTriggered(0),
    m_buttonsReleased(0),
    m_leftTrigger(0.f),
    m_rightTrigger(0.f),
    m_leftMotorIntensity(0.f),
    m_leftMotorTimer(-1.f),
    m_rightMotorIntensity(0.f),
    m_rightMotorTimer(-1.f)
{
}

gep::XInputGamepad::~XInputGamepad()
{
    if (isConnected())
    {
        // stop rumble motors
        rumbleLeft(0.f);
        rumbleRight(0.f);
        updateRumble(0.f);
    }
}

void gep::XInputGamepad::rumbleLeft(float intensity)
{
    m_leftMotorIntensity = clamp(intensity, 0.f, 1.f);
    m_leftMotorTimer = -1.f;
}

void gep::XInputGamepad::rumbleLeftFor(float intensity, float time)
{
    rumbleLeft(intensity);
    m_leftMotorTimer = clamp(time, 0.f, FLT_MAX);
}

void gep::XInputGamepad::rumbleRight(float intensity)
{
    m_rightMotorIntensity = clamp(intensity, 0.f, 1.f);
    m_rightMotorTimer = -1.f;
}

void gep::XInputGamepad::rumbleRightFor(float intensity, float time)
{
    rumbleRight(intensity);
    m_rightMotorTimer = clamp(time, 0.f, FLT_MAX);
}

void gep::XInputGamepad::update(float elapsedTime, DWORD dwUserIndex)
{
    m_dwUserIndex = dwUserIndex;
    if (m_checkStateTimer <= 0.f)
    {
        ZeroMemory(&m_XInputState, sizeof(XINPUT_STATE));
        DWORD dwResult = XInputGetState(dwUserIndex, &m_XInputState);
        if (dwResult == ERROR_SUCCESS)
        {
            XINPUT_GAMEPAD& gamepad = m_XInputState.Gamepad;

            // digital buttons
            m_buttonsPressedOld = m_buttonsPressed;
            m_buttonsPressed = gamepad.wButtons;
            m_buttonsTriggered = (m_buttonsPressedOld ^ m_buttonsPressed) & m_buttonsPressed;
            m_buttonsReleased = (m_buttonsPressedOld ^ m_buttonsPressed) & m_buttonsPressedOld;

            // analog sticks
            m_leftStick = clampStick(gamepad.sThumbLX, gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE, SHRT_MAX - 5);
            m_rightStick = clampStick(gamepad.sThumbRX, gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE, SHRT_MAX - 5);

            // shoulder buttons
            m_leftTrigger = clampTrigger(gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, UCHAR_MAX - 5);
            m_rightTrigger = clampTrigger(gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD, UCHAR_MAX - 5);

            // motors
            updateRumble(elapsedTime);

            m_checkStateTimer = 0.f;
        }
        else
        {
            m_checkStateTimer = 2.f;	// wait 2 seconds
        }
    }
    else
    {
        m_checkStateTimer -= elapsedTime;
    }
}

gep::vec2 gep::XInputGamepad::clampStick(SHORT sThumbX, SHORT sThumbY, int deadzone, int maximum)
{
    vec2 stick;
    stick.x = clampStickAxis(sThumbX, deadzone, maximum);
    stick.y = clampStickAxis(sThumbY, deadzone, maximum);
    return stick;
}

float gep::XInputGamepad::clampStickAxis(SHORT sThumbAxis, int deadzone, int maximum)
{
    float fThumbAxis = (float)sThumbAxis;
    if (fThumbAxis > 0.f)
        fThumbAxis = clamp(fThumbAxis - deadzone, 0.f, (float)maximum);
    if (fThumbAxis < 0.f)
        fThumbAxis = clamp(fThumbAxis + deadzone, (float)-maximum, 0.f);
    return clamp(fThumbAxis / (float)(maximum - deadzone), -1.f, 1.f);
}

float gep::XInputGamepad::clampTrigger(BYTE bTrigger, int threshold, int maximum)
{
    if (bTrigger > maximum)
        bTrigger = maximum;
    float fTrigger = ((int)bTrigger - threshold) / (float)(maximum - threshold);
    return clamp(fTrigger, 0.f, 1.f);
}

void gep::XInputGamepad::updateRumble(float elapsedTime)
{
    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(XINPUT_VIBRATION));
    if (m_leftMotorTimer >= 0.f)
    {
        m_leftMotorTimer -= elapsedTime;
        if (m_leftMotorTimer <= 0)
        {
            m_leftMotorTimer = -1;
            m_leftMotorIntensity = 0.f;
        }
    }
    vibration.wLeftMotorSpeed = (USHORT)(m_leftMotorIntensity * USHRT_MAX);
    if (m_rightMotorTimer >= 0.f)
    {
        m_rightMotorTimer -= elapsedTime;
        if (m_rightMotorTimer <= 0)
        {
            m_rightMotorTimer = -1;
            m_rightMotorIntensity = 0.f;
        }
    }
    vibration.wRightMotorSpeed = (USHORT)(m_rightMotorIntensity * USHRT_MAX);
    XInputSetState(m_dwUserIndex, &vibration);
}

gep::InputHandler::InputHandler()
    : m_mouseSensitivity(0.5f, 0.5f, 1.0f / 120.0f),
    m_mouseDelta(0.0f),
    m_currentFrame(1)
    , m_isAnyPressed(false)
    , m_wasAnyTriggered(false)
{
}

void gep::InputHandler::initialize()
{
    m_currentFrame = 1;

    // adds HID mouse and also ignores legacy mouse messages
    RAWINPUTDEVICE Rid;
    Rid.usUsagePage = 0x01;
    Rid.usUsage = 0x02;
    Rid.dwFlags = 0;
    Rid.hwndTarget = 0;
    BOOL r = RegisterRawInputDevices(&Rid, 1, sizeof(Rid));
    GEP_ASSERT(r, "getting raw mouse input failed");
}

void gep::InputHandler::destroy()
{
}

void gep::InputHandler::update(float elapsedTime)
{
    m_mouseDelta = vec3(0.0f, 0.0f, 0.0f);
    m_wasAnyTriggered = false;
    m_currentFrame++;
    MSG msg = {0};
    while( PeekMessage(&msg,NULL,0,0,PM_REMOVE) )
    {
        switch( msg.message )
        {
        case WM_QUIT:
            g_globalManager.getUpdateFramework()->stop();
            break;
        case WM_KEYDOWN: //Handle keyboard key down messages
            {
                m_wasAnyTriggered = true;
                //uint8 keyCode = (msg.lParam >> 16) & 0xFF;
                uint8 keyCode = uint8(msg.wParam);
                auto& info = m_keyMap[keyCode];

                if(!info.isPressed)
                {
                    info.isPressed = true;
                    info.keyDownFrame = m_currentFrame;
                }
            }
            break;
        case WM_KEYUP: //Handle keyboard key up messages
            {
                //uint8 keyCode = (msg.lParam >> 16) & 0xFF;
                uint8 keyCode = uint8(msg.wParam);
                m_keyMap[keyCode].isPressed = false;
            }
            break;
        case WM_INPUT: //Handle remaining raw input
            {
                UINT dwSize = 0;

                GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));
                if(dwSize > 0)
                {
                    uint8* buffer = (uint8*)alloca(dwSize);
                    UINT bytesWritten = GetRawInputData((HRAWINPUT)msg.lParam, RID_INPUT, buffer, &dwSize, sizeof(RAWINPUTHEADER));
                    GEP_ASSERT(dwSize == bytesWritten);
                    RAWINPUT* raw = (RAWINPUT*)buffer;
                    if(raw->header.dwType == RIM_TYPEMOUSE)
                    {
                        m_mouseDelta.x = (float)raw->data.mouse.lLastX * m_mouseSensitivity.x;
                        m_mouseDelta.y = (float)raw->data.mouse.lLastY * m_mouseSensitivity.y;

                        switch(raw->data.mouse.usButtonFlags)
                        {
                        case RI_MOUSE_LEFT_BUTTON_DOWN:
                            m_keyMap[VK_LBUTTON].isPressed = true;
                            m_keyMap[VK_LBUTTON].keyDownFrame = m_currentFrame;
                            break;
                        case RI_MOUSE_LEFT_BUTTON_UP:
                            m_keyMap[VK_LBUTTON].isPressed = false;
                            break;
                        case RI_MOUSE_MIDDLE_BUTTON_DOWN:
                            m_keyMap[VK_MBUTTON].isPressed = true;
                            m_keyMap[VK_MBUTTON].keyDownFrame = m_currentFrame;
                            break;
                        case RI_MOUSE_MIDDLE_BUTTON_UP:
                            m_keyMap[VK_MBUTTON].isPressed = false;
                            break;
                        case RI_MOUSE_RIGHT_BUTTON_DOWN:
                            m_keyMap[VK_RBUTTON].isPressed = true;
                            m_keyMap[VK_RBUTTON].keyDownFrame = m_currentFrame;
                            break;
                        case RI_MOUSE_RIGHT_BUTTON_UP:
                            m_keyMap[VK_RBUTTON].isPressed = false;
                            break;
                        case RI_MOUSE_BUTTON_4_DOWN:
                            m_keyMap[VK_XBUTTON1].isPressed = true;
                            m_keyMap[VK_XBUTTON1].keyDownFrame = m_currentFrame;
                            break;
                        case RI_MOUSE_BUTTON_4_UP:
                            m_keyMap[VK_XBUTTON1].isPressed = false;
                            break;
                        case RI_MOUSE_BUTTON_5_DOWN:
                            m_keyMap[VK_XBUTTON2].isPressed = true;
                            m_keyMap[VK_XBUTTON2].keyDownFrame = m_currentFrame;
                            break;
                        case RI_MOUSE_BUTTON_5_UP:
                            m_keyMap[VK_XBUTTON2].isPressed = false;
                            break;
                        case RI_MOUSE_WHEEL:
                            m_mouseDelta.z = float(SHORT(raw->data.mouse.usButtonData)) * m_mouseSensitivity.z;
                            break;
                        }
                    }

                }
            }
            break;
        default:
            TranslateMessage( &msg );
            DispatchMessage( &msg );
            break;
        }
    }

    // update XInput gamepads
    for (DWORD dwUserIndex=0; dwUserIndex<XUSER_MAX_COUNT; ++dwUserIndex)
    {
        m_pXInputGamepads[dwUserIndex].update(elapsedTime/1000.f, dwUserIndex);
    }

    m_isAnyPressed = false;
    for (auto& keyInfo : m_keyMap)
    {
        if (keyInfo.isPressed)
        {
            m_isAnyPressed = true;
            break;
        }
    }
}

bool gep::InputHandler::isPressed(uint8 keyCode)
{
    return m_keyMap[keyCode].isPressed;
}

bool gep::InputHandler::wasTriggered(uint8 keyCode)
{
    return m_keyMap[keyCode].keyDownFrame == m_currentFrame;
}

bool gep::InputHandler::getMouseDelta(vec3& mouseDelta)
{
    mouseDelta = m_mouseDelta;
    return !m_mouseDelta.epsilonCompare(vec3(0.0f));
}

gep::vec2 gep::InputHandler::getMouseNormalizedScreenPosition()
{
    auto renderer = g_globalManager.getRenderer();
    POINT point;
    GetCursorPos(&point);
    if (ScreenToClient(renderer->getWindowHandle(), &point))
    {
        point.x = std::max(0l, point.x);
        point.x = std::min((long) renderer->getScreenWidth(), point.x);
        point.y = std::max(0l, point.y);
        point.y = std::min((long) renderer->getScreenHeight(), point.y);

        uvec2 temp = uvec2(point.x, point.y);
        vec2 result = renderer->toNormalizedScreenPosition(temp);
        return result;
    }
    return vec2();
}

gep::uvec2 gep::InputHandler::getMouseAbsoluteScreenPosition()
{
    uvec2 absolutePos = g_globalManager.getRenderer()->toAbsoluteScreenPosition(getMouseNormalizedScreenPosition());
    return absolutePos;
}

gep::IGamepad* gep::InputHandler::gamepad(int userIndex)
{
    GEP_ASSERT(userIndex>=0 && userIndex<XUSER_MAX_COUNT);
    return &m_pXInputGamepads[userIndex];
}
