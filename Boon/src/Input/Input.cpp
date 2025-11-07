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
#include <cstring>

namespace Boon
{
    struct Input::Impl
    {
        // -------- Constants --------
        static constexpr int MAX_KEYS = 512;
        static constexpr int MAX_MOUSE_BUTTONS = 8;
        static constexpr int MAX_CONTROLLERS = 4;

        // -------- Key/Mouse State --------
        uint64_t keyStates[(MAX_KEYS + 31) / 32]{};      // current keys down
        uint64_t pressedKeys[(MAX_KEYS + 31) / 32]{};    // pressed this frame
        uint64_t releasedKeys[(MAX_KEYS + 31) / 32]{};   // released this frame

        uint64_t mouseStates[1]{};                       // current mouse buttons
        uint64_t pressedMouse[1]{};                      // pressed this frame
        uint64_t releasedMouse[1]{};                     // released this frame

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

        // -------- Constructor --------
        Impl()
        {
            GLFWwindow* window = static_cast<GLFWwindow*>(Application::Get().GetWindow().GetApiWindow());
            glfwSetKeyCallback(window, KeyCallbackStatic);
            glfwSetMouseButtonCallback(window, MouseButtonCallbackStatic);
            glfwSetCursorPosCallback(window, CursorPosCallbackStatic);
            glfwSetScrollCallback(window, ScrollCallbackStatic);
        }

        ~Impl() = default;

        // -------- Utility Bit Ops --------
        inline void SetBit(uint64_t* arr, int index, bool value)
        {
            const uint64_t mask = 1ULL << (index % 64);
            if (value) arr[index / 64] |= mask;
            else       arr[index / 64] &= ~mask;
        }

        inline bool GetBit(const uint64_t* arr, int index) const
        {
            return (arr[index / 64] >> (index % 64)) & 1ULL;
        }

        // -------- Update --------
        void Update()
        {
            // Clear transient press/release buffers
            std::memset(pressedKeys, 0, sizeof(pressedKeys));
            std::memset(releasedKeys, 0, sizeof(releasedKeys));
            std::memset(pressedMouse, 0, sizeof(pressedMouse));
            std::memset(releasedMouse, 0, sizeof(releasedMouse));

            // Reset scroll delta
            scrollX = scrollY = 0.0;

            // Update controllers
            for (int jid = 0; jid < MAX_CONTROLLERS; ++jid)
                UpdateController(jid);
        }

        // -------- Controller Update --------
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
                    bool pressed = (state.buttons[b] == GLFW_PRESS);
                    const int idx = b / 64;
                    const uint64_t mask = 1ULL << (b % 64);
                    if (pressed)
                        controllers[jid].buttonStates[idx] |= mask;
                    else
                        controllers[jid].buttonStates[idx] &= ~mask;
                }

                // Axes (apply deadzone + sensitivity)
                for (int a = 0; a <= Gamepad::AxisLast; ++a)
                {
                    float v = state.axes[a];
                    const float dz = controllers[jid].deadZone;
                    const float sens = controllers[jid].axisSensitivity;

                    if (std::fabs(v) < dz)
                        v = 0.0f;
                    else
                        v = ((std::fabs(v) - dz) / (1.0f - dz)) * (v > 0 ? 1.0f : -1.0f);

                    controllers[jid].axes[a] = v * sens;
                }
            }
        }

        // -------- Getters --------
        KeyState GetKeyState(KeyCode key) const
        {
            const int idx = key / 64;
            const uint64_t mask = 1ULL << (key % 64);

            const bool down = keyStates[idx] & mask;
            const bool pressed = pressedKeys[idx] & mask;
            const bool released = releasedKeys[idx] & mask;

            if (pressed)  return KeyState::Pressed;
            if (released) return KeyState::Released;
            if (down)     return KeyState::Held;
            return KeyState::Up;
        }

        KeyState GetMouseState(MouseCode button) const
        {
            const int idx = button / 64;
            const uint64_t mask = 1ULL << (button % 64);

            const bool down = mouseStates[idx] & mask;
            const bool pressed = pressedMouse[idx] & mask;
            const bool released = releasedMouse[idx] & mask;

            if (pressed)  return KeyState::Pressed;
            if (released) return KeyState::Released;
            if (down)     return KeyState::Held;
            return KeyState::Up;
        }

        KeyState GetGamepadButtonState(int jid, GamepadButtonCode button) const
        {
            const auto& c = controllers[jid];
            const int idx = button / 64;
            const uint64_t mask = 1ULL << (button % 64);
            const bool pressed = (c.buttonStates[idx] & mask) != 0;
            return pressed ? KeyState::Held : KeyState::Up;
        }

        // -------- GLFW Callbacks --------
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

        // -------- Event Handlers --------
        void KeyCallback(int key, int action)
        {
            if (key < 0 || key >= MAX_KEYS) return;

            const int idx = key / 64;
            const uint64_t mask = 1ULL << (key % 64);

            if (action == GLFW_PRESS)
            {
                pressedKeys[idx] |= mask;
                keyStates[idx] |= mask;
            }
            else if (action == GLFW_RELEASE)
            {
                releasedKeys[idx] |= mask;
                keyStates[idx] &= ~mask;
            }
        }

        void MouseButtonCallback(int button, int action)
        {
            if (button < 0 || button >= MAX_MOUSE_BUTTONS) return;

            const int idx = button / 64;
            const uint64_t mask = 1ULL << (button % 64);

            if (action == GLFW_PRESS)
            {
                pressedMouse[idx] |= mask;
                mouseStates[idx] |= mask;
            }
            else if (action == GLFW_RELEASE)
            {
                releasedMouse[idx] |= mask;
                mouseStates[idx] &= ~mask;
            }
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
    };

    // -------------------------------------------------------
    // Public Interface
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
