#pragma once

#include <iostream>
#include <functional>
#include <string>
#include "../Button.hpp"
#include "DataContainer.hpp"

namespace psychicui {

    template<class T = std::string> class Tabs : public DataContainer<T> {
        using TabData = std::vector<T>;
        using TabChangedCallback = std::function<void(const T &data)>;

    public:
        Tabs(const TabData &data, TabChangedCallback tabChangedCallback);
    protected:
//        void updateTabs();
        TabChangedCallback _tabChangedCallback{nullptr};
    };

    template<class T>
    Tabs<T>::Tabs(const Tabs::TabData &data, Tabs::TabChangedCallback tabChangedCallback) :
        DataContainer<T>(data),
        _tabChangedCallback(tabChangedCallback) {

//        for (const auto &d: DataContainer<T>::_data) {
//            try {
//                auto label = dynamic_cast<std::string>(d);
//                Component::add(std::make_shared<Button>(label));
//            }
//            catch (const std::bad_cast &e) {
//                std::cout << "na" << std::endl;
//            }
//        }
    }

}

