#include <memory>
#include <vector>
#include <iostream>
#include <string>

class A{
public:
    std::string a{"prout"};
};

int main() {
    std::vector<std::shared_ptr<A>> vec{};
    vec.push_back(std::make_shared<A>());
    vec.push_back(std::make_shared<A>());
    vec.push_back(std::make_shared<A>());

    int i = 0;
    for (auto &item: vec) {
        ++i;
        if (i == 2) {
            vec.push_back(std::make_shared<A>());
        }
        std::cout << i << " " << item->a << std::endl;
    }

    i = 0;
    bool added = false;
    for (auto item = vec.rbegin(); item != vec.rend(); ++item) {
        ++i;
        std::cout << i << " " << added << std::endl;
        if (i == 2 && !added) {
            added = true;
            vec.push_back(std::make_shared<A>());
        }
        //std::cout << i << " " << (*item)->a << std::endl;
    }
}
//#include <iostream>
//#include <Signal.hpp>
//#include <Observer.hpp>
//
//using namespace psychicui;
//
//class A: public Observer {
//public:
//    using Observer::subscribeTo;
//    using Observer::slots;
//};
//
//int main() {
//    std::cout << "Playground" << std::endl;
//
//    Signal<> s1{};
//    auto s1s = s1.subscribe([](){
//        std::cout << "No args" << std::endl;
//    });
//    s1.emit();
//    s1.unsubscribe(s1s);
//    s1.emit();
//
//    Signal<> s2{};
//    {
//        auto s2s = s2.subscribe(
//            []() {
//                std::cout << "No args, should die" << std::endl;
//            }
//        );
//        s2.emit();
//    }
//    s2.emit();
//
//    Signal<> s2b{};
//    {
//        s2b.subscribe(
//            []() {
//                std::cout << "No args, not saved, should die" << std::endl;
//            }
//        );
//        s2b.emit();
//    }
//    s2b.emit();
//
//    Signal<> s3{};
//    {
//        A a{};
//        a.subscribeTo(
//            s3, []() {
//                std::cout << "In observer, should die" << std::endl;
//            }
//        );
//        s3.emit();
//    }
//    s3.emit();
//
//    {
//        Signal<>* s4 = new Signal<>();
//        A a{};
//        a.subscribeTo(
//            *s4, []() {
//                std::cout << "In observer, should die" << std::endl;
//            }
//        );
//        s4->emit();
//        delete s4;
//        std::cout << a.slots.size() << std::endl;
//    }
//}