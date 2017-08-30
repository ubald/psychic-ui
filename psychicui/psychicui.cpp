#include <iostream>
#include <thread>
#include <map>
#include "Window.hpp"
#include "psychicui.hpp"

namespace psychicui {

    #if defined(__APPLE__)
    extern void disable_saved_application_state_osx();
    #endif

    void init() {
        #if defined(__APPLE__)
        disable_saved_application_state_osx();
        #endif

        glfwSetErrorCallback(
            [](int error, const char *descr) {
                if (error == GLFW_NOT_INITIALIZED) {
                    return;
                }
                std::cerr << "GLFW error " << error << ": " << descr << std::endl;
            }
        );

        if (!glfwInit()) {
            throw std::runtime_error("Could not initialize GLFW!");
        }

        glfwSetTime(0);

        // Initialize Yoga
        YGConfigSetUseWebDefaults(YGConfigGetDefault(), true);
    }

    static bool mainloop_active = false;

    void mainloop() {
        if (mainloop_active) {
            throw std::runtime_error("Main loop is already running!");
        }

        mainloop_active = true;

        try {
            while (mainloop_active) {
                glfwPollEvents();

                int       numScreens = 0;
                for (auto kv : Window::windows) {
                    Window *window = kv.second;
                    if (!window->visible()) {
                        continue;
                    } else if (glfwWindowShouldClose(window->glfwWindow())) {
                        window->setVisible(false);
                        continue;
                    }
                    window->drawAll();
                    numScreens++;
                }

                if (numScreens == 0) {
                    mainloop_active = false;
                    break;
                }
            }
        } catch (const std::exception &e) {
            std::cerr << "Caught exception in main loop: " << e.what() << std::endl;
            mainloop_active = false;
        }
    }

    void shutdown() {
        glfwTerminate();
    }

}