#include "Input/Input.h"
#include "Core/Application.h"
#include "Core/ServiceLocator.h"

#include <GLFW/glfw3.h>
#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>

namespace Boon
{
    struct Input::Impl
    {
        // -------- Constants --------
        static constexpr int MAX_KEYS = 512;
        static constexpr int MAX_MOUSE_BUTTONS = 8;
        static constexpr int MAX_CONTROLLERS = 4;

        // -------- Key/Mouse State --------
        uint64_t keyStates[(MAX_KEYS + 31) / 32]{};
        uint64_t mouseStates[1]{};

        double mouseX = 0.0, mouseY = 0.0;
        double scrollX = 0.0, scrollY = 0.0;

        std::unordered_map<std::string, std::vector<int>> actionMap;

        // -------- Gamepad Data --------
        struct Controller
        {
            bool connected = false;
            uint64_t buttonStates[(Gamepad::ButtonLast + 31) / 32]{};
            float axes[Gamepad::AxisLast + 1]{};
            float deadZone = 0.15f;
            float axisSensitivity = 1.0f;
        } controllers[MAX_CONTROLLERS];

        float defaultDeadZone = 0.15f;
        float defaultAxisSensitivity = 1.0f;

        // -------- Constructor / Destructor --------
        Impl()
        {
            GLFWwindow* window = (GLFWwindow*)Application::Get().GetWindow().GetApiWindow();
            glfwSetKeyCallback(window, KeyCallbackStatic);
            glfwSetMouseButtonCallback(window, MouseButtonCallbackStatic);
            glfwSetCursorPosCallback(window, CursorPosCallbackStatic);
            glfwSetScrollCallback(window, ScrollCallbackStatic);
        }

        ~Impl() = default;

        // -------- Main Update --------
        void Update()
        {
            // Update key and mouse states
            for (int i = 0; i < MAX_KEYS; ++i)
                UpdateKeyState(i, true);
            for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
                UpdateMouseState(i, true);

            // Reset scroll
            scrollX = scrollY = 0.0;

            // Update controllers
            for (int jid = 0; jid < MAX_CONTROLLERS; ++jid)
                UpdateController(jid);
        }

        // -------- State Helpers --------
        void UpdateKeyState(int key, bool transition)
        {
            KeyState s = GetKeyState(key);
            if (s == KeyState::Pressed) SetKeyState(key, KeyState::Held);
            else if (s == KeyState::Released) SetKeyState(key, KeyState::Up);
        }

        void UpdateMouseState(int button, bool transition)
        {
            KeyState s = GetMouseState(button);
            if (s == KeyState::Pressed) SetMouseState(button, KeyState::Held);
            else if (s == KeyState::Released) SetMouseState(button, KeyState::Up);
        }

        // -------- Gamepad Update --------
        void UpdateController(int jid)
        {
            if (!glfwJoystickPresent(jid))
            {
                controllers[jid].connected = false;
                return;
            }

            controllers[jid].connected = true;

            GLFWgamepadstate state;
            if (glfwGetGamepadState(jid, &state))
            {
                // Buttons
                for (int b = 0; b <= Gamepad::ButtonLast; ++b)
                {
                    KeyState current = GetGamepadButtonState(jid, b);
                    if (state.buttons[b] == GLFW_PRESS)
                    {
                        if (current == KeyState::Up || current == KeyState::Released)
                            SetGamepadButtonState(jid, b, KeyState::Pressed);
                        else
                            SetGamepadButtonState(jid, b, KeyState::Held);
                    }
                    else
                    {
                        if (current == KeyState::Pressed || current == KeyState::Held)
                            SetGamepadButtonState(jid, b, KeyState::Released);
                        else
                            SetGamepadButtonState(jid, b, KeyState::Up);
                    }
                }

                // Axes (apply deadzone and sensitivity)
                for (int a = 0; a <= Gamepad::AxisLast; ++a)
                {
                    float v = state.axes[a];
                    float dz = controllers[jid].deadZone;
                    float sens = controllers[jid].axisSensitivity;

                    if (std::fabs(v) < dz)
                        v = 0.0f;
                    else
                        v = (std::fabs(v) - dz) / (1.0f - dz) * (v > 0 ? 1 : -1);

                    controllers[jid].axes[a] = v * sens;
                }
            }
        }

        // -------- Bit Operations --------
        KeyState GetKeyState(KeyCode key) const
        {
            int idx = key / 32;
            int shift = (key % 32) * 2;
            return (KeyState)((keyStates[idx] >> shift) & 0b11);
        }

        void SetKeyState(KeyCode key, KeyState state)
        {
            int idx = key / 32;
            int shift = (key % 32) * 2;
            keyStates[idx] &= ~(0b11ULL << shift);
            keyStates[idx] |= ((uint64_t)state << shift);
        }

        KeyState GetMouseState(MouseCode button) const
        {
            return (KeyState)((mouseStates[0] >> (button * 2)) & 0b11);
        }

        void SetMouseState(MouseCode button, KeyState state)
        {
            mouseStates[0] &= ~(0b11ULL << (button * 2));
            mouseStates[0] |= ((uint64_t)state << (button * 2));
        }

        KeyState GetGamepadButtonState(int jid, GamepadButtonCode button) const
        {
            const auto& c = controllers[jid];
            int idx = button / 32;
            int shift = (button % 32) * 2;
            return (KeyState)((c.buttonStates[idx] >> shift) & 0b11);
        }

        void SetGamepadButtonState(int jid, GamepadButtonCode button, KeyState state)
        {
            auto& c = controllers[jid];
            int idx = button / 32;
            int shift = (button % 32) * 2;
            c.buttonStates[idx] &= ~(0b11ULL << shift);
            c.buttonStates[idx] |= ((uint64_t)state << shift);
        }

        // -------- Action Queries --------
        bool IsActionPressed(const std::string& action) const
        {
            auto it = actionMap.find(action);
            if (it == actionMap.end()) return false;
            for (auto k : it->second)
                if (GetKeyState(k) == KeyState::Pressed)
                    return true;
            return false;
        }

        bool IsActionHeld(const std::string& action) const
        {
            auto it = actionMap.find(action);
            if (it == actionMap.end()) return false;
            for (auto k : it->second)
                if (GetKeyState(k) == KeyState::Held)
                    return true;
            return false;
        }

        // -------- Callbacks --------
        static void KeyCallbackStatic(GLFWwindow*, int key, int, int action, int)
        {
            ServiceLocator::Get<Input>().m_pImpl->KeyCallback(key, action);
        }

        static void MouseButtonCallbackStatic(GLFWwindow*, int button, int action, int)
        {
            ServiceLocator::Get<Input>().m_pImpl->MouseButtonCallback(button, action);
        }

        static void CursorPosCallbackStatic(GLFWwindow*, double xpos, double ypos)
        {
            ServiceLocator::Get<Input>().m_pImpl->CursorPosCallback(xpos, ypos);
        }

        static void ScrollCallbackStatic(GLFWwindow*, double xoffset, double yoffset)
        {
            ServiceLocator::Get<Input>().m_pImpl->ScrollCallback(xoffset, yoffset);
        }

        void KeyCallback(int key, int action)
        {
            if (key < 0 || key >= MAX_KEYS) return;
            if (action == GLFW_PRESS) SetKeyState(key, KeyState::Pressed);
            else if (action == GLFW_RELEASE) SetKeyState(key, KeyState::Released);
        }

        void MouseButtonCallback(int button, int action)
        {
            if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;
            if (action == GLFW_PRESS) SetMouseState(button, KeyState::Pressed);
            else if (action == GLFW_RELEASE) SetMouseState(button, KeyState::Released);
        }

        void CursorPosCallback(double xpos, double ypos)
        {
            mouseX = xpos;
            mouseY = ypos;
        }

        void ScrollCallback(double xoffset, double yoffset)
        {
            scrollX += xoffset;
            scrollY += yoffset;
        }
    };

    // -------------------------------------------------------
    // Public interface — only forwards to Impl (no logic)
    // -------------------------------------------------------
    Input::Input() : m_pImpl(new Impl()) {}
    Input::~Input() { delete m_pImpl; }

    void Input::Update() { m_pImpl->Update(); }

    bool Input::IsKeyPressed(KeyCode key) const { return m_pImpl->GetKeyState(key) == KeyState::Pressed; }
    bool Input::IsKeyHeld(KeyCode key) const { return m_pImpl->GetKeyState(key) == KeyState::Held; }
    bool Input::IsKeyReleased(KeyCode key) const { return m_pImpl->GetKeyState(key) == KeyState::Released; }

    bool Input::IsMousePressed(MouseCode button) const { return m_pImpl->GetMouseState(button) == KeyState::Pressed; }
    bool Input::IsMouseHeld(MouseCode button) const { return m_pImpl->GetMouseState(button) == KeyState::Held; }
    bool Input::IsMouseReleased(MouseCode button) const { return m_pImpl->GetMouseState(button) == KeyState::Released; }

    double Input::GetMouseX() const { return m_pImpl->mouseX; }
    double Input::GetMouseY() const { return m_pImpl->mouseY; }
    double Input::GetScrollX() const { return m_pImpl->scrollX; }
    double Input::GetScrollY() const { return m_pImpl->scrollY; }

    void Input::BindAction(const std::string& action, int key) { m_pImpl->actionMap[action].push_back(key); }
    bool Input::IsActionPressed(const std::string& action) const { return m_pImpl->IsActionPressed(action); }
    bool Input::IsActionHeld(const std::string& action) const { return m_pImpl->IsActionHeld(action); }

    bool Input::IsControllerConnected(int id) const { return m_pImpl->controllers[id].connected; }
    bool Input::IsControllerButtonPressed(int id, GamepadButtonCode button) const { return m_pImpl->GetGamepadButtonState(id, button) == KeyState::Pressed; }
    bool Input::IsControllerButtonHeld(int id, GamepadButtonCode button) const { return m_pImpl->GetGamepadButtonState(id, button) == KeyState::Held; }
    bool Input::IsControllerButtonReleased(int id, GamepadButtonCode button) const { return m_pImpl->GetGamepadButtonState(id, button) == KeyState::Released; }
    float Input::GetControllerAxis(int id, GamepadAxisCode axis) const { return m_pImpl->controllers[id].axes[axis]; }

    void Input::KeyCallback(int key, int action) { m_pImpl->KeyCallback(key, action); }
    void Input::MouseButtonCallback(int button, int action) { m_pImpl->MouseButtonCallback(button, action); }
    void Input::CursorPosCallback(double xpos, double ypos) { m_pImpl->CursorPosCallback(xpos, ypos); }
    void Input::ScrollCallback(double xoffset, double yoffset) { m_pImpl->ScrollCallback(xoffset, yoffset); }
}