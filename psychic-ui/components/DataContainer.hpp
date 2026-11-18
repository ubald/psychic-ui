#pragma once

#include <utility>
#include "include/core/SkCanvas.h"
#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    template<class T>
    class DataContainer : public Div {
    public:
        using ContainerData = std::vector<T>;
        using DivCallback = std::function<std::shared_ptr<Div>(const T &)>;

        explicit DataContainer(ContainerData data, DivCallback getDiv);

        const ContainerData &data() const;

        /**
         * Replace the data: the children are rebuilt before the next paint
         */
        template<class Self>
        Self &data(this Self &self, const ContainerData &data) {
            static_cast<DataContainer &>(self).updateData(data);
            return self;
        }



    protected:
        virtual void updateData(const ContainerData &data);
        void render(SkCanvas *canvas) override;
        ContainerData _data{};
        DivCallback   _getDiv{nullptr};
        bool          _dataChanged{true};

        // Make some of div's stuff protected since we manage our content
        using Div::add;
        using Div::remove;
        using Div::removeAll;
    };

    template<class T>
    DataContainer<T>::DataContainer(typename DataContainer<T>::ContainerData data, DivCallback getDiv) :
        Div(),
        _data(std::move(data)),
        _getDiv(std::move(getDiv)) {
        tag("DataContainer");
    }

    template<class T>
    const typename DataContainer<T>::ContainerData &DataContainer<T>::data() const {
        return _data;
    }

    template<class T>
    void DataContainer<T>::updateData(const typename DataContainer<T>::ContainerData &data) {
        if (data != _data) {
            _data = data;
            _dataChanged = true;
        }
    }

    template<class T>
    void DataContainer<T>::render(SkCanvas *canvas) {
        if (_dataChanged) {
            removeAll();
            for (const auto &item: _data) {
                auto div = _getDiv(item);
                if (div) {
                    add(div);
                }
            }
            _dataChanged = false;
        }

        Div::render(canvas);
    }


}

