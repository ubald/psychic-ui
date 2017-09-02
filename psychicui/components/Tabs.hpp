#pragma once

#include <iostream>
#include <functional>
#include <string>
#include "../Button.hpp"
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
        std::vector<Button *> _buttons{};
        TabChanged            _tabChanged{nullptr};
    };

    template<class T>
    Tabs<T>::Tabs(const TabData &data, LabelCallback getLabel, TabChanged tabChanged) :
        DataContainer<T>(data, getLabel),
        _tabChanged(tabChanged) {
        this->setTag("Tabs");

        for (const auto &d: DataContainer<T>::_data) {
            _buttons.push_back(
                this->template add<Button>(
                    this->label(d),
                    [&]() {
                        select(d);
                        if (_tabChanged) {
                            _tabChanged(d);
                        }
                    }
                ).get()
            );
        }
    }

    template<class T>
    void Tabs<T>::select(const T &item) {
        auto     pos = std::distance(this->_data.begin(), std::find(this->_data.begin(), this->_data.end(), item));
        for (int i  = 0; i < this->_children.size(); ++i) {
            _buttons[i]->setSelected(i == pos);
        }
    }

}

