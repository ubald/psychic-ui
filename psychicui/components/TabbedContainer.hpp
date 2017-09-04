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
        TabbedContainer<T> * select(const T &item);
    protected:
        void onTabChanged(const T &item);
        ComponentCallback _getTabComponentCallback{nullptr};
        std::shared_ptr<Tabs<T>> _tabs;
        std::shared_ptr<Component> _panel;
    };

    template<class T>
    TabbedContainer<T>::TabbedContainer(const TabContainerData &data, ComponentCallback getComponent, LabelCallback getLabel) :
        Component(),
        _getTabComponentCallback(getComponent) {
        this->setTag("TabbedContainer");
        _defaults->set(overflow, "hidden");

        // Make the tabs component
        _tabs = this->add<Tabs<T>>(data, getLabel, [this](const T &data) { onTabChanged(data); });
    }

    template<class T>
    void TabbedContainer<T>::onTabChanged(const T &item) {
        if (!_getTabComponentCallback) {
            return;
        }

        if (_panel) {
            remove(_panel);
            _panel = nullptr;
        }

        _panel = _getTabComponentCallback(item);
        add(_panel);
    }

    template<class T>
    TabbedContainer<T> * TabbedContainer<T>::select(const T &item) {
        _tabs->select(item);
        onTabChanged(item);
        return this;
    }
}

