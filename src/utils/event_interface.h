#pragma once
#include "utils/events.h"

class IWindowEventHandler {
public:
    virtual ~IWindowEventHandler() = default;

    /**
     *  Update the game logic.
     */
    virtual void onUpdate(UpdateEventArgs& e) = 0;

    /**
     *  Render stuff.
     */
    virtual void onRender(RenderEventArgs& e) = 0;

    /**
     * Invoked by the registered window when a key is pressed
     * while the window has focus.
     */
    virtual void onKeyPressed(KeyEventArgs& e) = 0;

    /**
     * Invoked when a key on the keyboard is released.
     */
    virtual void onKeyReleased(KeyEventArgs& e) = 0;

    /**
     * Invoked when the mouse is moved over the registered window.
     */
    virtual void onMouseMoved(MouseMotionEventArgs& e) = 0;

    /**
     * Invoked when a mouse button is pressed over the registered window.
     */
    virtual void onMouseButtonPressed(MouseButtonEventArgs& e) = 0;

    /**
     * Invoked when a mouse button is released over the registered window.
     */
    virtual void onMouseButtonReleased(MouseButtonEventArgs& e) = 0;

    /**
     * Invoked when the mouse wheel is scrolled while the registered window has focus.
     */
    virtual void onMouseWheel(MouseWheelEventArgs& e) = 0;

    /**
     * Invoked when the attached window is resized.
     */
    virtual void onResize(ResizeEventArgs& e) = 0;

    /**
     * Invoked when the registered window instance is destroyed.
     */
    virtual void onWindowDestroy() = 0;
};
