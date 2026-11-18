#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>
#include <vector>
#include <catch2/catch_test_macros.hpp>
#include "fake_system_window.hpp"

using namespace psychic_ui;
using namespace psychic_ui_tests;

TEST_CASE("Posted tasks run in posting order at the next runPosted, not before", "[backend][frames][post]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow fake(window);
    fake.layout();

    std::vector<int> order{};
    window->post([&order] { order.push_back(1); });
    window->post([&order] { order.push_back(2); });
    CHECK(order.empty());
    CHECK(window->needsFrame());

    window->runPosted();
    CHECK(order == std::vector{1, 2});
    window->updateStylesAndLayout();
    CHECK_FALSE(window->needsFrame());
}

TEST_CASE("A task posted by a task runs at the next runPosted", "[backend][frames][post]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow fake(window);
    fake.layout();

    bool outer = false;
    bool inner = false;
    window->post([&] {
        outer = true;
        window->post([&] { inner = true; });
    });

    window->runPosted();
    CHECK(outer);
    CHECK_FALSE(inner);
    CHECK(window->needsFrame());

    window->runPosted();
    CHECK(inner);
}

TEST_CASE("Tasks posted from other threads all run, waking the system window once until the next frame", "[backend][frames][post]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow fake(window);
    fake.layout();
    fake.wakes = 0;

    int                      count = 0; // Tasks run on this thread
    std::vector<std::thread> threads{};
    threads.reserve(4);
    for (int round = 0; round < 4; ++round) {
        threads.emplace_back([&window, &count] {
            for (int i = 0; i < 1000; ++i) {
                window->post([&count] { ++count; });
            }
        });
    }
    for (auto &thread: threads) {
        thread.join();
    }
    CHECK(fake.wakes == 1);

    window->runPosted();
    CHECK(count == 4000);
}

TEST_CASE("A post after runPosted still needs a frame after the layout clears the request", "[backend][frames][post]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow fake(window);
    fake.layout();

    window->runPosted();
    window->post([] {}); // As drawAll could see it from another thread: after runPosted, before the layout
    window->updateStylesAndLayout();
    CHECK(window->needsFrame());
}

TEST_CASE("A task that throws leaves the rest of its batch for the next runPosted", "[backend][frames][post]") {
    auto             window = std::make_shared<TestWindow>();
    FakeSystemWindow fake(window);
    fake.layout();

    bool firstRan = false;
    bool thirdRan = false;
    window->post([&firstRan] { firstRan = true; });
    window->post([] { throw std::runtime_error("task"); });
    window->post([&thirdRan] { thirdRan = true; });

    CHECK_THROWS_AS(window->runPosted(), std::runtime_error);
    CHECK(firstRan);
    CHECK_FALSE(thirdRan);
    CHECK(window->needsFrame());

    window->runPosted();
    CHECK(thirdRan);
}

TEST_CASE("Tasks still queued when the window goes are destroyed without running", "[backend][frames][post]") {
    auto window = std::make_shared<TestWindow>();
    auto fake   = std::make_unique<FakeSystemWindow>(window);
    fake->layout();

    bool                     ran      = false;
    auto                     sentinel = std::make_shared<int>(0);
    const std::weak_ptr<int> weak     = sentinel;
    window->post([sentinel = std::move(sentinel), &ran] { ran = true; });

    fake.reset(); // It holds the window too
    window.reset();
    CHECK(weak.expired());
    CHECK_FALSE(ran);
}
