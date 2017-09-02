#include <string>
#include <iostream>

#include <rxcpp/rx.hpp>

namespace rx {
    using namespace rxcpp;
    using namespace rxcpp::sources;
    using namespace rxcpp::operators;
    using namespace rxcpp::util;
}
using namespace rx;

int main() {
    std::cout << "Playground" << std::endl;

    ::rxcpp::subjects::subject<int> subject;
    auto                            subscriber1 = subject.get_subscriber();
    auto                            subscriber2 = subject.get_subscriber();
    auto                            observable  = subject.get_observable();

    rxcpp::observable<int> *x;
    {
        auto z = observable.map([](int message) { return message * 2; }).as_dynamic();
        auto y = &z;
        x = &z;
    }
    auto s = observable
        | filter([](int message) { return message > 2; })
        | map([](int message) { return message * 2; });
    s.subscribe(
        [](int message) {
            std::cout << message << std::endl;
        }
    );

    subscriber1.on_next(1);
    subscriber1.on_next(2);
    subscriber1.on_next(3);

    subscriber2.on_next(4);
    subscriber2.on_next(5);
    subscriber2.on_next(6);

    std::cout << "ID1: " << subscriber1.get_id() << " ID2: " << subscriber1.get_id() << std::endl;
}