/**
 * A GLFW application that embeds Psychic UI: it owns GLFW, the window, its OpenGL context and the loop,
 * and hands the window to a `GLFWSystemWindow`, the way it would hand it to Dear ImGui's GLFW backend.
 */

#include <chrono>
#include <iostream>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/applications/GLFWSystemWindow.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/skins/pivotal/PivotalSkins.hpp>

using namespace psychic_ui;

namespace {
    /**
     * The UI the host shows
     */
    class HostedWindow : public Window {
    public:
        HostedWindow() : Window("Psychic UI in a GLFW host") {
            skinSet<PivotalSkins>();

            app->direction(Direction::Column)
               .alignItems(Align::Start) // "flex-start" before, which the string parser ignored (stretch)
               .padding(24.0f);

            app->add<Label>("This window, its context and its loop belong to the host. Escape quits.").marginBottom(12);
            _clicks = &app->add<Label>("Not clicked yet").marginBottom(12);
            app->add<Button>("Click me", [this]() {
                ++_count;
                _clicks->text("Clicked " + std::to_string(_count) + (_count == 1 ? " time" : " times"));
            }).marginBottom(12);
            app->add<TextInput>("Type here").marginBottom(12).width(320);
        }

    private:
        Label *_clicks{nullptr}; // A child of app, which the window owns
        int                    _count{0};
    };

    /**
     * The host's own key callback, which the adapter chains to: Escape quits
     */
    void hostKeyCallback(GLFWwindow *window, int key, int /*scancode*/, int action, int /*mods*/) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }
}

int main() {
    glfwSetErrorCallback([](int error, const char *description) {
        std::cerr << "GLFW error " << error << ": " << description << '\n';
    });
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);
    GLFWwindow *glfwWindow = glfwCreateWindow(800, 480, "Psychic UI in a GLFW host", nullptr, nullptr);
    if (!glfwWindow) {
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(glfwWindow);
    glfwSwapInterval(1);
    glfwSetKeyCallback(glfwWindow, hostKeyCallback);

    {
        auto ui = std::make_shared<HostedWindow>();
        GLFWSystemWindow adapter(ui, glfwWindow);
        // Nothing but Psychic UI draws in this window
        adapter.setSharesContext(false);

        // Draw, then wait for input unless the window has more to show (an animation, a timer); input wakes the
        // wait, and so does a frame asked for (the adapter posts an empty event)
        while (!glfwWindowShouldClose(glfwWindow)) {
            adapter.render();
            glfwSwapBuffers(glfwWindow);
            const auto wait = ui->untilNextFrame(adapter.gestureTime());
            if (!wait) {
                glfwWaitEvents();
            } else if (*wait > std::chrono::nanoseconds::zero()) {
                glfwWaitEventsTimeout(std::chrono::duration<double>(*wait).count());
            } else {
                glfwPollEvents();
            }
        }
    } // The adapter goes before the window

    glfwDestroyWindow(glfwWindow);
    glfwTerminate();
    return 0;
}
