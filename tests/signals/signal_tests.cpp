#include <catch2/catch_test_macros.hpp>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>
#include <psychic-ui/signals/Observer.hpp>
#include <psychic-ui/signals/Signal.hpp>

using namespace psychic_ui;

static_assert(!std::is_copy_constructible_v<Signal<int>>);
static_assert(!std::is_copy_assignable_v<Signal<int>>);
static_assert(!std::is_move_constructible_v<Signal<int>>);
static_assert(!std::is_move_assignable_v<Signal<int>>);
static_assert(!std::is_copy_constructible_v<Observer>);
static_assert(!std::is_copy_assignable_v<Observer>);
static_assert(!std::is_copy_constructible_v<Slot<int>>);
static_assert(!std::is_copy_constructible_v<ScopedSlot>);
static_assert(std::is_nothrow_move_constructible_v<ScopedSlot>);
static_assert(std::has_virtual_destructor_v<SlotBase>);

namespace {
    ScopedSlot takeFrom(ScopedSlot &holder) {
        return std::move(holder);
    }

    struct Listener : Observer {
        template<class... T, class F>
        std::shared_ptr<Slot<T...>> listen(Signal<T...> &signal, F &&callback) {
            return subscribeTo(signal, std::forward<F>(callback));
        }

        std::size_t slotCount() const {
            return slots.size();
        }
    };

    // Holds a signal whose handler captures the owner, like a Div capturing itself in its own onClick
    struct Owner {
        Signal<> onClick{};
        int      *destroyed;

        explicit Owner(int *destroyed) : destroyed(destroyed) {}

        ~Owner() {
            ++*destroyed;
        }
    };
}

TEST_CASE("Signal basics", "[signals]") {
    Signal<int, int> signal;
    int              sum = 0;

    REQUIRE_FALSE(signal.hasSubscriptions());
    REQUIRE(signal.subscriptionCount() == 0);

    auto slot = signal.subscribe([&](int a, int b) { sum += a + b; });
    REQUIRE(signal.hasSubscriptions());
    REQUIRE(signal.subscriptionCount() == 1);
    REQUIRE(slot->connected());

    SECTION("emits rvalues and lvalues") {
        signal.emit(1, 2);
        int x = 10;
        signal.emit(x, x);
        REQUIRE(sum == 23);
    }

    SECTION("callbacks may take the arguments by const reference") {
        signal.subscribe([&](const int &a, const int &) { sum += a * 100; });
        signal.emit(1, 0);
        REQUIRE(sum == 101);
    }

    SECTION("disconnect") {
        slot->disconnect();
        REQUIRE_FALSE(slot->connected());
        REQUIRE_FALSE(signal.hasSubscriptions());
        signal.emit(1, 2);
        REQUIRE(sum == 0);
    }

    SECTION("double disconnect is a no-op") {
        slot->disconnect();
        slot->disconnect();
        signal.emit(1, 2);
        REQUIRE(sum == 0);
        REQUIRE(signal.subscriptionCount() == 0);
    }
}

TEST_CASE("Signal arguments are converted once and passed by reference", "[signals]") {
    Signal<std::string> signal;
    std::vector<const std::string *> seen{};
    signal.subscribe([&](const std::string &s) { seen.push_back(&s); });
    signal.subscribe([&](const std::string &s) { seen.push_back(&s); });
    signal.emit("converted from const char *");
    REQUIRE(seen.size() == 2);
    REQUIRE(seen[0] == seen[1]);
}

TEST_CASE("Disconnecting during emission", "[signals]") {
    Signal<int>                    signal;
    std::vector<std::string>       calls{};
    std::shared_ptr<Slot<int>>     first, second, third;

    SECTION("a slot disconnecting itself doesn't skip the next one") {
        first  = signal.subscribe([&](int) { calls.emplace_back("first"); first->disconnect(); });
        second = signal.subscribe([&](int) { calls.emplace_back("second"); });
        signal.emit(0);
        signal.emit(0);
        REQUIRE(calls == std::vector<std::string>{"first", "second", "second"});
        REQUIRE(signal.subscriptionCount() == 1);
    }

    SECTION("a slot disconnecting the next one stops it from running") {
        first  = signal.subscribe([&](int) { calls.emplace_back("first"); second->disconnect(); });
        second = signal.subscribe([&](int) { calls.emplace_back("second"); });
        third  = signal.subscribe([&](int) { calls.emplace_back("third"); });
        signal.emit(0);
        REQUIRE(calls == std::vector<std::string>{"first", "third"});
    }

    SECTION("a slot disconnecting the previous one") {
        first  = signal.subscribe([&](int) { calls.emplace_back("first"); });
        second = signal.subscribe([&](int) { calls.emplace_back("second"); first->disconnect(); });
        third  = signal.subscribe([&](int) { calls.emplace_back("third"); });
        signal.emit(0);
        signal.emit(0);
        REQUIRE(calls == std::vector<std::string>{"first", "second", "third", "second", "third"});
    }

    SECTION("a self-disconnecting slot whose handle is dropped keeps its callback alive while it runs") {
        auto marker = std::make_shared<int>(42);
        first = signal.subscribe([&, marker](int) {
            first->disconnect();
            first = nullptr; // Drops the last outside reference
            calls.emplace_back(std::to_string(*marker));
        });
        marker = nullptr;
        signal.emit(0);
        REQUIRE(calls == std::vector<std::string>{"42"});
        REQUIRE_FALSE(signal.hasSubscriptions());
    }

    SECTION("nested emissions") {
        first = signal.subscribe([&](int depth) {
            calls.push_back("first" + std::to_string(depth));
            if (depth == 0) {
                second->disconnect();
                signal.emit(1);
            }
        });
        second = signal.subscribe([&](int depth) { calls.push_back("second" + std::to_string(depth)); });
        third  = signal.subscribe([&](int depth) { calls.push_back("third" + std::to_string(depth)); });
        signal.emit(0);
        REQUIRE(calls == std::vector<std::string>{"first0", "first1", "third1", "third0"});
        REQUIRE(signal.subscriptionCount() == 2);
    }
}

TEST_CASE("Connecting during emission fires from the next emission", "[signals]") {
    Signal<> signal;
    int      added = 0;
    int      ran   = 0;
    signal.subscribe([&]() {
        // Enough to reallocate the slot list under the running emission
        for (int i = 0; i < 64; ++i) {
            signal.subscribe([&]() { ++added; });
        }
        ++ran;
    });
    signal.emit();
    REQUIRE(ran == 1);
    REQUIRE(added == 0);
    signal.emit();
    REQUIRE(ran == 2);
    REQUIRE(added == 64);
}

TEST_CASE("Signal destroyed by its own handler", "[signals]") {
    auto signal = std::make_unique<Signal<int>>();
    int  ran    = 0;
    auto marker = std::make_shared<int>(7);
    signal->subscribe([&, marker](int value) {
        ++ran;
        signal.reset();
        REQUIRE(*marker == 7); // The running callback survives
        REQUIRE(value == 1);
    });
    auto next = signal->subscribe([&](int) { ++ran; }); // Not called: its signal is gone
    Signal<int> *raw = signal.get();
    raw->emit(1);
    REQUIRE(ran == 1);
    REQUIRE(signal == nullptr);
    REQUIRE_FALSE(next->connected());
}

TEST_CASE("Slots outliving their signal", "[signals]") {
    std::shared_ptr<Slot<>> slot;
    ScopedSlot              scoped;
    Listener                listener;
    {
        Signal<> signal;
        slot   = signal.subscribe([]() {});
        scoped = signal.subscribe([]() {});
        listener.listen(signal, []() {});
        REQUIRE(slot->connected());
        REQUIRE(scoped.connected());
    }
    REQUIRE_FALSE(slot->connected());
    REQUIRE_FALSE(scoped.connected());
    slot->disconnect();
    scoped.disconnect();
    // ~Listener disconnects from the dead signal at the end of the scope
}

TEST_CASE("Observer", "[signals]") {
    Signal<int> signal;
    int         sum = 0;

    SECTION("disconnects when destroyed") {
        {
            Listener listener;
            listener.listen(signal, [&](int v) { sum += v; });
            signal.emit(1);
        }
        signal.emit(1);
        REQUIRE(sum == 1);
        REQUIRE_FALSE(signal.hasSubscriptions());
    }

    SECTION("a slot disconnected directly") {
        Listener listener;
        auto     slot = listener.listen(signal, [&](int v) { sum += v; });
        slot->disconnect();
        slot->disconnect();
        signal.emit(1);
        REQUIRE(sum == 0);
        listener.listen(signal, [](int) {}); // Forgets the disconnected one
        REQUIRE(listener.slotCount() == 1);
    }

    SECTION("observer destroyed by the handler it's running") {
        auto listener = std::make_unique<Listener>();
        listener->listen(signal, [&](int v) { sum += v; listener.reset(); });
        signal.subscribe([&](int v) { sum += 10 * v; });
        signal.emit(1);
        signal.emit(1);
        REQUIRE(sum == 21);
    }

    SECTION("forgets slots that ended on their own") {
        Listener listener;
        for (int i = 0; i < 4; ++i) {
            Signal<> temporary;
            listener.listen(temporary, []() {});
        }
        listener.listen(signal, [](int) {});
        REQUIRE(listener.slotCount() == 1);
    }
}

TEST_CASE("ScopedSlot", "[signals]") {
    Signal<int> signal;
    int         sum = 0;

    SECTION("disconnects the previous slot when reassigned") {
        ScopedSlot scoped;
        scoped = signal.subscribe([&](int v) { sum += v; });
        scoped = signal.subscribe([&](int v) { sum += 10 * v; });
        signal.emit(1);
        REQUIRE(sum == 10);
        REQUIRE(signal.subscriptionCount() == 1);
    }

    SECTION("disconnects when destroyed") {
        {
            ScopedSlot scoped = signal.subscribe([&](int v) { sum += v; });
        }
        signal.emit(1);
        REQUIRE(sum == 0);
    }

    SECTION("a handler replacing its own scoped slot") {
        ScopedSlot scoped;
        scoped = signal.subscribe([&](int v) {
            sum += v;
            scoped = signal.subscribe([&](int w) { sum += 100 * w; });
        });
        signal.emit(1);
        signal.emit(1);
        REQUIRE(sum == 101);
    }

    SECTION("move keeps a single owner") {
        // The slot moves out of a holder that stays in use, as a member would: what the holder keeps
        // is what this checks, an empty slot whose disconnect leaves the new owner's subscription alone
        ScopedSlot a = signal.subscribe([&](int v) { sum += v; });
        ScopedSlot b = takeFrom(a);
        REQUIRE_FALSE(a.connected());
        a.disconnect();
        signal.emit(1);
        REQUIRE(sum == 1);
    }
}

TEST_CASE("Disconnected and orphaned slots release their callbacks", "[signals]") {
    auto marker = std::make_shared<int>(0);

    SECTION("on disconnect, even while the handle is kept") {
        Signal<> signal;
        auto     slot = signal.subscribe([marker]() {});
        REQUIRE(marker.use_count() == 2);
        slot->disconnect();
        REQUIRE(marker.use_count() == 1);
    }

    SECTION("when the signal dies, even while the handle is kept") {
        std::shared_ptr<Slot<>> slot;
        {
            Signal<> signal;
            slot = signal.subscribe([marker]() {});
        }
        REQUIRE(marker.use_count() == 1);
    }

    SECTION("an owner capturing itself through a raw pointer is freed") {
        int destroyed = 0;
        {
            auto owner = std::make_shared<Owner>(&destroyed);
            owner->onClick.subscribe([raw = owner.get()]() { (void) raw; });
        }
        REQUIRE(destroyed == 1);
    }
}

TEST_CASE("A callback that takes nothing ignores the arguments", "[signals]") {
    Signal<int, std::string> signal;
    int ran = 0;
    signal.subscribe([&] { ++ran; });
    int sum = 0;
    signal.subscribe([&](int value, const std::string &) { sum += value; });
    signal.emit(3, "three");
    CHECK(ran == 1);
    CHECK(sum == 3);
}
