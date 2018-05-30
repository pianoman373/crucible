#pragma once

#include <crucible/Math.hpp>
#include <string>

struct GLFWwindow;

class Window {
private:


public:
    static GLFWwindow *window;


    static void create(vec2i resolution, std::string title, bool fullscreen);

    static bool isOpen();

    static void begin();

    static void end();

    static void terminate();

    /**
     * Returns the time in seconds that the window has been open.
     */
    static float getTime();

    static float deltaTime();

    /**
     * Returns the window size in pixels.
     */
    static vec2i getWindowSize();

    static float getAspectRatio();

    static void setMouseGrabbed(bool grabbed);
};
