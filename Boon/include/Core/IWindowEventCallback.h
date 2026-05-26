#pragma once

namespace Boon
{
    class IWindowEventCallback
    {
    public:
        virtual ~IWindowEventCallback() = default;

        virtual void KeyCallback(int key, int action) = 0;
        virtual void MouseButtonCallback(int button, int action) = 0;
        virtual void CursorPosCallback(double xpos, double ypos) = 0;
        virtual void ScrollCallback(double xoffset, double yoffset) = 0;
    };
}