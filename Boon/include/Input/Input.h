#pragma once
#include <memory>
#include <string>
#include <cstdint>
#include <unordered_map>
#include <vector>

#include "KeyCodes.h"
#include "MouseCodes.h"
#include "GamepadCodes.h"

namespace Boon
{
    enum class KeyState : uint8_t
    {
        Up = 0,
        Pressed,
        Held,
        Released
    };

    class Input
    {
    public:
        Input();
        ~Input();

        // Per-frame update
        void Update();

        // Keyboard
        bool IsKeyPressed(KeyCode key) const;
        bool IsKeyHeld(KeyCode key) const;
        bool IsKeyReleased(KeyCode key) const;

        // Mouse
        bool IsMousePressed(MouseCode button) const;
        bool IsMouseHeld(MouseCode button) const;
        bool IsMouseReleased(MouseCode button) const;

        double GetMouseX() const;
        double GetMouseY() const;
        double GetScrollX() const;
        double GetScrollY() const;

        // Actions (key binding system)
        void BindAction(const std::string& action, int key);
        bool IsActionPressed(const std::string& action) const;
        bool IsActionHeld(const std::string& action) const;

        // Controller (multi-device)
        bool IsControllerConnected(int controller) const;
        bool IsControllerButtonPressed(int controller, GamepadButtonCode button) const;
        bool IsControllerButtonHeld(int controller, GamepadButtonCode button) const;
        bool IsControllerButtonReleased(int controller, GamepadButtonCode button) const;
        float GetControllerAxis(int controller, GamepadAxisCode axis) const;

        // Controller tuning (global)
        void SetDefaultDeadZone(float deadzone);
        void SetDefaultAxisSensitivity(float sensitivity);
        float GetDefaultDeadZone() const;
        float GetDefaultAxisSensitivity() const;

        // Controller tuning (per-controller)
        void SetControllerDeadZone(int controller, float deadzone);
        void SetControllerAxisSensitivity(int controller, float sensitivity);
        float GetControllerDeadZone(int controller) const;
        float GetControllerAxisSensitivity(int controller) const;

        // GLFW callbacks (called from window)
        void KeyCallback(int key, int action);
        void MouseButtonCallback(int button, int action);
        void CursorPosCallback(double xpos, double ypos);
        void ScrollCallback(double xoffset, double yoffset);

    private:
        struct Impl;
        Impl* m_pImpl;
    };
}