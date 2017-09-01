#pragma once

#include "../Component.hpp"

namespace psychicui {

    template<class T>
    class DataContainer : public Component {
        using ContainerData = std::vector<T>;
    public:
        DataContainer(const ContainerData &data);
    protected:
        ContainerData _data;
    };

    template<class T>
    DataContainer<T>::DataContainer(const ContainerData &data) :
        Component(),
        _data(data) {

    }
}

