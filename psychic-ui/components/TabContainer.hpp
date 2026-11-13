#pragma once

#include <memory>
#include <functional>
#include "psychic-ui/Div.hpp"
#include "DataContainer.hpp"
#include "Tabs.hpp"

namespace psychic_ui {

    template<class T = std::string>
    class TabContainer: public Div {
    public:
        using TabContainerData = typename Tabs<T>::TabData;
        using LabelCallback = typename Tabs<T>::LabelCallback;
        using ComponentCallback = std::function<std::shared_ptr<Div>(const T &)>;

        TabContainer(const TabContainerData &data, LabelCallback getLabel, ComponentCallback getComponent);
        template<class Self>
        Self &select(this Self &self, const T &item) {
            static_cast<TabContainer &>(self).updateSelected(item);
            return self;
        }

    protected:
        void updateSelected(const T &item);
        void onTabChanged(const T &item);
        ComponentCallback _getTabComponentCallback{nullptr};
        Tabs<T>              *_tabs{nullptr}; // A child, which we own
        std::shared_ptr<Div> _panel;
    };

    template<class T>
    TabContainer<T>::TabContainer(const TabContainerData &data, LabelCallback getLabel, ComponentCallback getComponent) :
        Div(),
        _getTabComponentCallback(getComponent) {
        this->tag("TabContainer");
        _defaultStyle.overflow(Overflow::Hidden);

        // Make the tabs component
        _tabs = &this->template add<Tabs<T>>(data, getLabel, [this](const T &data) { onTabChanged(data); });
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
    void TabContainer<T>::updateSelected(const T &item) {
        _tabs->select(item);
        onTabChanged(item);
    }
}

