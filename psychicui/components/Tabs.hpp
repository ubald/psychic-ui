#pragma once

#include <iostream>
#include <functional>
#include <string>
#include "Button.hpp"
#include "DataContainer.hpp"

namespace psychicui {

    template<class T = std::string>
    class Tabs : public DataContainer<T> {
    public:
        using TabData = typename DataContainer<T>::ContainerData;
        using LabelCallback = typename DataContainer<T>::LabelCallback;
        using TabChanged = std::function<void(const T &)>;

        Tabs(const TabData &data, LabelCallback getLabel = nullptr, TabChanged tabChanged = nullptr);

        void select(const T &item);
    protected:
        const T               *_selected{nullptr};
        std::vector<Button *> _buttons{};
        TabChanged            _tabChanged{nullptr};
    };

    template<class T>
    Tabs<T>::Tabs(const TabData &data, LabelCallback getLabel, TabChanged tabChanged) :
        DataContainer<T>(data, getLabel),
        _tabChanged(tabChanged) {
        this->setTag("Tabs");
        this->_defaults->set(shrink, 0);

        for (const auto &d: DataContainer<T>::_data) {
            _buttons.push_back(
                this->template add<Button>(
                    this->label(d),
                    [&]() {
                        if (!_selected || d != *_selected) {
                            select(d);
                            if (_tabChanged) {
                                _tabChanged(d);
                            }
                        }
                    }
                )->setToggle(true)
            );
        }
    }

    template<class T>
    void Tabs<T>::select(const T &item) {
        _selected = &item;
        auto     pos = std::distance(this->_data.begin(), std::find(this->_data.begin(), this->_data.end(), item));
        for (int i   = 0; i < this->_buttons.size(); ++i) {
            _buttons[i]->setSelected(i == pos);
        }
    }

}

