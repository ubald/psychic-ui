#pragma once

#include <SkCanvas.h>
#include "psychicui/Div.hpp"

namespace psychicui {

    template<class T>
    class DataContainer : public Div {
    public:
        DataContainer() = delete;
        using ContainerData = std::vector<T>;
        using DivCallback = std::function<std::shared_ptr<Div>(const T &)>;

        explicit DataContainer(const ContainerData &data, DivCallback getDiv);

        ContainerData &data() const;
        virtual DataContainer<T> *setData(const ContainerData &data);


    protected:
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
    DataContainer<T>::DataContainer(const typename DataContainer<T>::ContainerData &data, DivCallback getDiv) :
        Div(),
        _data(data),
        _getDiv(getDiv) {
        setTag("DataContainer");
    }

    template<class T>
    typename DataContainer<T>::ContainerData &DataContainer<T>::data() const {
        return _data;
    }

    template<class T>
    DataContainer<T> *DataContainer<T>::setData(const typename DataContainer<T>::ContainerData &data) {
        if (data != _data) {
            _data = data;
            _dataChanged = true;
        }
        return this;
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

