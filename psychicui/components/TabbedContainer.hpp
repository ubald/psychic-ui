#pragma once

#include <memory>
#include <functional>
#include "../Component.hpp"
#include "DataContainer.hpp"
#include "Tabs.hpp"

namespace psychicui {

    template<class T = std::string>
    class TabbedContainer: public Component {
    public:
        using TabContainerData = typename Tabs<T>::TabData;
        using LabelCallback = typename Tabs<T>::LabelCallback;
        using ComponentCallback = std::function<std::shared_ptr<Component>(const T &)>;

        TabbedContainer(const TabContainerData &data, ComponentCallback getComponent, LabelCallback getLabel = nullptr);
    protected:
        void onTabChanged(const T &data);
        ComponentCallback _getTabComponentCallback{nullptr};
    };

    template<class T>
    TabbedContainer<T>::TabbedContainer(const TabContainerData &data, ComponentCallback getComponent, LabelCallback getLabel) :
        Component(),
        _getTabComponentCallback(getComponent) {
        this->setTag("TabbedContainer");

        // Make the tabs component
        this->add(std::make_shared<Tabs<T>>(data, getLabel, [this](const T &data) { onTabChanged(data); }));
    }

    template<class T>
    void TabbedContainer<T>::onTabChanged(const T &data) {
        if (!_getTabComponentCallback) {
            return;
        }

        auto component = _getTabComponentCallback(data);
    }
}

