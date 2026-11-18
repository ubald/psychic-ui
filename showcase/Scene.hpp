#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <psychic-ui/psychic-ui.hpp>

namespace psychic_ui {
    class Window;
}

namespace psychic_ui::showcase {
    class Stage;

    /**
     * How a scene looks: a skin set and one of the themes by their keys (see `Looks.hpp`), and the input mode, whose
     * density the style manager follows
     */
    struct Look {
        std::string skinSet{"default"};
        std::string theme{}; // Empty: the set's default theme
        InputMode   input{InputMode::Pointer};
    };

    /**
     * Something done to a scene at a time, in ms after its pose
     */
    struct Beat {
        int                          at{0};
        std::function<void(Stage &)> play{};
    };

    /**
     * What makes a scene an animated PNG: beats played over a duration, a frame taken every step (ms), looping
     */
    struct Sequence {
        std::vector<Beat> beats{}; // None: a still
        int               duration{0};
        int               step{40};
        int               still{-1}; // The time of a frame also written alone as <name>-still.png, for reduced motion
    };

    /**
     * One capture of the showcase: a window of a given size under a look, the UI it holds, and what is done to it
     * before the capture (a menu opened, keys pressed, time passed)
     */
    struct Scene {
        std::string name{};   // The file name, without its extension
        std::string title{};  // What it shows, for `--list`
        int         width{960};
        int         height{600};
        Look        look{};

        /**
         * Makes the window under the look; `stageWindow` by default
         */
        std::function<std::shared_ptr<Window>(const Look &)> window{};

        /**
         * Builds the UI, before the window's first frame
         */
        std::function<void(Window &)> build{};

        /**
         * Puts the UI in the state to show, once laid out; the capture comes when nothing moves any more
         */
        std::function<void(Stage &)> pose{};

        /**
         * For an animated PNG rather than a still
         */
        Sequence motion{};
    };

    /**
     * A window with the showcase's rules over the skin set's, under a look: what most scenes build in
     */
    std::shared_ptr<Window> stageWindow(const Look &look);

    /**
     * Every scene, in capture order
     */
    std::vector<Scene> scenes();

    // The scenes by subject, each in its own file under scenes/
    std::vector<Scene> heroScenes();
    std::vector<Scene> controlScenes();
    std::vector<Scene> popupScenes();
    std::vector<Scene> collectionScenes();
    std::vector<Scene> foundationScenes();
}
