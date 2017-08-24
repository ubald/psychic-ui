#include <iostream>
#include <thread>
#include <map>
#include <psychicui/Screen.hpp>
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
                for (auto kv : Screen::screens) {
                    Screen *screen = kv.second;
                    if (!screen->visible()) {
                        continue;
                    } else if (glfwWindowShouldClose(screen->window())) {
                        screen->setVisible(false);
                        continue;
                    }
                    screen->drawAll();
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