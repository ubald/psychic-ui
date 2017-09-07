#pragma once

#include <iostream>
#include <algorithm>
#include <functional>
#include <string>
#include "Button.hpp"
#include "DataContainer.hpp"

namespace psychicui {

    template<class T = std::string>
    class Tabs : public DataContainer<T> {
    public:
        using TabData = typename DataContainer<T>::ContainerData;
        using LabelCallback = std::function<std::string(const T &)>;
        using TabChanged = std::function<void(const T &)>;

        Tabs(const TabData &data, LabelCallback getLabel, TabChanged tabChanged = nullptr);

        void select(const T &item);
        std::string label(const T &data) const;

        Tabs<T> *setData(const typename DataContainer<T>::ContainerData &data) override;

    protected:
        std::shared_ptr<Button> getTab(const T &item);
        void updateSelection();
        LabelCallback         _getLabel{nullptr};
        const T               *_selected{nullptr};
        std::vector<Button *> _buttons{};
        TabChanged            _tabChanged{nullptr};
    };

    template<class T>
    Tabs<T>::Tabs(const TabData &data, LabelCallback getLabel, TabChanged tabChanged) :
        DataContainer<T>(data, [this](const T &item) { return getTab(item); }),
        _getLabel(getLabel),
        _tabChanged(tabChanged) {
        this->setTag("Tabs");
        this->_defaultStyle->set(shrink, 0);
    }

    template<class T>
    std::shared_ptr<Button> Tabs<T>::getTab(const T &item) {
        auto tab = std::make_shared<Button>(
            label(item),
            [&]() {
                if (!_selected || item != *_selected) {
                    select(item);
                    if (_tabChanged) {
                        _tabChanged(item);
                    }
                }
            }
        );
        tab->setToggle(true)
           ->setAutoToggle(false);
        _buttons.push_back(tab.get());

        // TODO: This is here for now, until we have a clearer DataContainer lifecycle
        updateSelection();

        return tab;
    }

    template<class T>
    std::string Tabs<T>::label(const T &data) const {
        return _getLabel(data);
    }

    template<class T>
    void Tabs<T>::select(const T &item) {
        // Find item in data since (pardon my C++ knowledge at this time) when initializing with a temporary data source
        // and selecting from this data source, the pointer becomes invalid once the original data source expired.
        auto res = std::find(this->_data.begin(), this->_data.end(), item);
        if (res != this->_data.end()) {
            _selected = &(*res);
        } else {
            _selected = nullptr;
        }
        updateSelection();
    }

    template<class T>
    void Tabs<T>::updateSelection() {
        auto     pos = std::distance(this->_data.begin(), std::find(this->_data.begin(), this->_data.end(), *_selected));
        for (int i   = 0; i < _buttons.size(); ++i) {
            _buttons[i]->setSelected(i == pos);
        }
    }

    template<class T>
    Tabs<T> *Tabs<T>::setData(const typename DataContainer<T>::ContainerData &data) {
        _selected = nullptr;
        _buttons.clear();
        DataContainer<T>::setData(data);
        return this;
    }

}
