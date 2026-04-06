#pragma once

#include "key_codes.h"

// Base class for all event args
class EventArgs
{
public:
    EventArgs() noexcept = default;
    virtual ~EventArgs() noexcept = default;
};

class KeyEventArgs : public EventArgs
{
public:
    enum KeyState
    {
        Released = 0,
        Pressed = 1
    };

    KeyEventArgs(KeyCode::Key key, unsigned int c, KeyState state, bool control, bool shift, bool alt) noexcept
        : key(key), character(c), state(state), control(control), shift(shift), alt(alt)
    {}

    KeyCode::Key key;        // The Key Code that was pressed or released.
    unsigned int character;  // The 32-bit character code pressed, 0 if non-printable.
    KeyState state;          // Pressed or Released state.
    bool control;            // Control modifier state.
    bool shift;              // Shift modifier state.
    bool alt;                // Alt modifier state.
};

class MouseMotionEventArgs : public EventArgs
{
public:
    MouseMotionEventArgs(bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y) noexcept
        : leftButton(leftButton), middleButton(middleButton), rightButton(rightButton),
          control(control), shift(shift), x(x), y(y), relX(0), relY(0)
    {}

    bool leftButton;
    bool middleButton;
    bool rightButton;
    bool control;
    bool shift;

    int x;
    int y;
    int relX; // How far mouse moved since last event (default 0)
    int relY;
};

class MouseButtonEventArgs : public EventArgs
{
public:
    enum MouseButton
    {
        None = 0,
        Left = 1,
        Right = 2,
        Middle = 3
    };

    enum ButtonState
    {
        Released = 0,
        Pressed = 1
    };

    MouseButtonEventArgs(MouseButton buttonID, ButtonState state, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y) noexcept
        : button(buttonID), state(state),
          leftButton(leftButton), middleButton(middleButton), rightButton(rightButton),
          control(control), shift(shift), x(x), y(y)
    {}

    MouseButton button;
    ButtonState state;

    bool leftButton;
    bool middleButton;
    bool rightButton;
    bool control;
    bool shift;

    int x;
    int y;
};

class MouseWheelEventArgs : public EventArgs
{
public:
    MouseWheelEventArgs(float wheelDelta, bool leftButton, bool middleButton, bool rightButton, bool control, bool shift, int x, int y) noexcept
        : wheelDelta(wheelDelta),
          leftButton(leftButton), middleButton(middleButton), rightButton(rightButton),
          control(control), shift(shift), x(x), y(y)
    {}

    float wheelDelta;

    bool leftButton;
    bool middleButton;
    bool rightButton;
    bool control;
    bool shift;

    int x;
    int y;
};

class ResizeEventArgs : public EventArgs
{
public:
    ResizeEventArgs(int width, int height) noexcept
        : width(width), height(height)
    {}

    int width;
    int height;
};

class UpdateEventArgs : public EventArgs
{
public:
    UpdateEventArgs(double elapsedTime, double totalTime) noexcept
        : elapsedTime(elapsedTime), totalTime(totalTime)
    {}

    double elapsedTime;
    double totalTime;
};

class RenderEventArgs : public EventArgs
{
public:
    RenderEventArgs(double elapsedTime, double totalTime) noexcept
        : elapsedTime(elapsedTime), totalTime(totalTime)
    {}

    double elapsedTime;
    double totalTime;
};

class UserEventArgs : public EventArgs
{
public:
    UserEventArgs(int code, void* data1, void* data2) noexcept
        : code(code), data1(data1), data2(data2)
    {}

    int code;
    void* data1;
    void* data2;
};
