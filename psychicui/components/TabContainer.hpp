#pragma once

#include <memory>
#include <functional>
#include "psychicui/Div.hpp"
#include "DataContainer.hpp"
#include "Tabs.hpp"

namespace psychicui {

    template<class T = std::string>
    class TabContainer: public Div {
    public:
        using TabContainerData = typename Tabs<T>::TabData;
        using LabelCallback = typename Tabs<T>::LabelCallback;
        using ComponentCallback = std::function<std::shared_ptr<Div>(const T &)>;

        TabContainer(const TabContainerData &data, LabelCallback getLabel, ComponentCallback getComponent);
        TabContainer<T> * select(const T &item);
    protected:
        void onTabChanged(const T &item);
        ComponentCallback _getTabComponentCallback{nullptr};
        std::shared_ptr<Tabs<T>> _tabs;
        std::shared_ptr<Div> _panel;
    };

    template<class T>
    TabContainer<T>::TabContainer(const TabContainerData &data, LabelCallback getLabel, ComponentCallback getComponent) :
        Div(),
        _getTabComponentCallback(getComponent) {
        this->setTag("TabContainer");
        _defaultStyle->set(overflow, "hidden");

        // Make the tabs component
        _tabs = this->add<Tabs<T>>(data, getLabel, [this](const T &data) { onTabChanged(data); });
    }

    template<class T>
    void TabContainer<T>::onTabChanged(const T &item) {
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
    TabContainer<T> * TabContainer<T>::select(const T &item) {
        _tabs->select(item);
        onTabChanged(item);
        return this;
    }
}

