#pragma once

#include <memory>
#include <functional>
#include "../Component.hpp"
#include "DataContainer.hpp"
#include "Tabs.hpp"

namespace psychicui {

    template<class T = std::string>
    class TabbedContainer: public Component {
        using TabContainerData = std::vector<T>;
        using GetTabComponentCallback = std::function<std::shared_ptr<Component>(const T &data)>;

    public:
        TabbedContainer(const TabContainerData &data, GetTabComponentCallback getTabComponentCallback);
    protected:
        void onTabChanged(const T &data);
        GetTabComponentCallback _getTabComponentCallback{nullptr};
    };

    template<class T>
    TabbedContainer<T>::TabbedContainer(const TabbedContainer<T>::TabContainerData &data, TabbedContainer<T>::GetTabComponentCallback getTabComponentCallback) :
        Component(),
        _getTabComponentCallback(getTabComponentCallback) {

        TabbedContainer<T>::add(std::make_shared<Tabs<T>>(data, [this](const T &data) { onTabChanged(data); }));
    }

    template<class T>
    void TabbedContainer<T>::onTabChanged(const T &data) {
        if (!_getTabComponentCallback) {
            return;
        }

        auto component = _getTabComponentCallback(data);
    }
}

