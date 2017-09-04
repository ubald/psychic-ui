#include <string>
#include <iostream>

//#include <rxcpp/rx.hpp>
//
//namespace rx {
//    using namespace rxcpp;
//    using namespace rxcpp::sources;
//    using namespace rxcpp::operators;
//    using namespace rxcpp::util;
//}
//using namespace rx;

class Test {
public:
    std::string &text() {
        return _text;
    }

    void setText(const std::string &text) {
        _text = text;
    }

protected:
    std::string _text{};
};

int main() {
    std::cout << "Playground" << std::endl;

    Test        t;
    std::string pouet{"Poupou"};
    t.setText(pouet);
    std::cout << t.text() << std::endl;
    pouet = "coucou";
    std::cout << t.text() << std::endl;
    t.text() = "allo";
    std::cout << t.text() << std::endl;
}