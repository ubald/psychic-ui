#include <chrono>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <iostream>
#include <memory>
#include <set>
#include <span>
#include <string>
#include <utility>
#include <vector>
#include <psychic-ui/Window.hpp>
#include "PngWriter.hpp"
#include "Scene.hpp"
#include "Stage.hpp"

#ifdef PSYCHIC_UI_SHOWCASE_LIVE
#include <psychic-ui/applications/SDL3Application.hpp>
#endif

using namespace psychic_ui;
using namespace psychic_ui::showcase;

namespace {
    constexpr float pixelRatio = 2.0f; // Crisp on high density screens

    std::shared_ptr<Window> makeWindow(const Scene &scene) {
        auto window = scene.window ? scene.window(scene.look) : stageWindow(scene.look);
        if (scene.build) {
            scene.build(*window);
        }
        return window;
    }

    /**
     * Run a scene on a headless stage and write its PNG (animated for motion)
     */
    bool capture(const Scene &scene, const std::filesystem::path &directory) {
        Stage stage(makeWindow(scene), scene.width, scene.height, pixelRatio);
        stage.settle(); // Laid out, what enters has entered
        if (scene.pose) {
            scene.pose(stage);
        }

        std::vector<std::uint8_t> png{};
        if (scene.motion.beats.empty()) {
            stage.settle();
            png = PngWriter::encode(stage.snapshot());
        } else {
            // A frame every step, the beats played at their times; a frame like the one before only makes that one
            // show longer, so what holds still costs nothing
            const Sequence       &motion = scene.motion;
            std::vector<PngFrame> frames{};
            std::vector<int>      delays{};
            const auto            start = stage.now();
            std::size_t           beat  = 0;
            for (int time = 0; time < motion.duration; time += motion.step) {
                while (stage.now() - start < std::chrono::milliseconds(time)) {
                    while (beat < motion.beats.size() && stage.now() - start >= std::chrono::milliseconds(motion.beats[beat].at)) {
                        motion.beats[beat++].play(stage);
                    }
                    stage.frame();
                }
                PngFrame frame = stage.snapshot();
                if (time == motion.still) {
                    const std::filesystem::path still = directory / (scene.name + "-still.png");
                    if (!PngWriter::write(still.string(), PngWriter::encode(frame))) {
                        std::cerr << "Could not write " << still << '\n';
                        return false;
                    }
                }
                if (!frames.empty() && frames.back().rgba == frame.rgba) {
                    delays.back() += motion.step;
                } else {
                    frames.push_back(std::move(frame));
                    delays.push_back(motion.step);
                }
            }
            png = PngWriter::encode(frames, delays);
        }
        const std::filesystem::path path = directory / (scene.name + ".png");
        if (!PngWriter::write(path.string(), png)) {
            std::cerr << "Could not write " << path << '\n';
            return false;
        }
        return true;
    }

    #ifdef PSYCHIC_UI_SHOWCASE_LIVE
    /**
     * Show a scene in a real SDL window, posed, to work on it: never part of a capture
     */
    void live(const Scene &scene) {
        SDL3Application app;
        auto            window = makeWindow(scene);
        window->setWindowSize(scene.width, scene.height);
        app.open(window);
        Stage stage(window);
        stage.frame();
        if (scene.pose) {
            scene.pose(stage);
        }
        app.run();
    }
    #endif

    void usage() {
        std::cerr << "psychic-ui-showcase --out <directory> [--scene <name>]...   write the scenes' PNGs\n"
                  << "psychic-ui-showcase --list                                  list the scenes\n"
                  #ifdef PSYCHIC_UI_SHOWCASE_LIVE
                  << "psychic-ui-showcase --live <name>                           show a scene in a window\n"
                  #endif
                  ;
    }
}

int main(int argc, char **argv) {
    std::filesystem::path out{};
    std::set<std::string> only{};
    std::string           liveScene{};
    bool                  list = false;
    const std::span<char *> args{argv, static_cast<std::size_t>(argc)};
    for (std::size_t i = 1; i < args.size(); ++i) {
        const std::string arg  = args[i];
        const bool        more = i + 1 < args.size();
        if (arg == "--out" && more) {
            out = args[++i];
        } else if (arg == "--scene" && more) {
            only.insert(args[++i]);
        } else if (arg == "--live" && more) {
            liveScene = args[++i];
        } else if (arg == "--list") {
            list = true;
        } else {
            usage();
            return 2;
        }
    }

    try {
        const std::vector<Scene> all = scenes();

        if (list) {
            for (const Scene &scene: all) {
                std::cout << scene.name << "\t" << scene.width << "×" << scene.height << "\t" << scene.title << "\n";
            }
            return 0;
        }

        if (!liveScene.empty()) {
            #ifdef PSYCHIC_UI_SHOWCASE_LIVE
            for (const Scene &scene: all) {
                if (scene.name == liveScene) {
                    live(scene);
                    return 0;
                }
            }
            std::cerr << "Unknown scene: " << liveScene << '\n';
            #else
            std::cerr << "Built without SDL3: no live window" << '\n';
            #endif
            return 1;
        }

        if (out.empty()) {
            usage();
            return 2;
        }
        std::filesystem::create_directories(out);

        int        written = 0;
        bool       failed  = false;
        const auto begin   = std::chrono::steady_clock::now();
        for (const Scene &scene: all) {
            if (!only.empty() && !only.contains(scene.name)) {
                continue;
            }
            const auto start = std::chrono::steady_clock::now();
            if (capture(scene, out)) {
                ++written;
            } else {
                failed = true;
            }
            const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start);
            std::cout << scene.name << ".png  " << ms.count() << " ms" << '\n';
        }
        const auto total = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - begin);
        std::cout << written << " scenes in " << total.count() << " ms" << '\n';
        if (!only.empty() && std::cmp_not_equal(written, only.size())) {
            std::cerr << "Some scenes named weren't found" << '\n';
            return 1;
        }
        return failed ? 1 : 0;
    } catch (const std::exception &e) {
        std::cerr << "Caught a fatal error: " << e.what() << '\n';
        return 1;
    }
}
