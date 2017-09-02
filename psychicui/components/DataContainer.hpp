#pragma once

#include <typeinfo>
#include "../Component.hpp"

namespace psychicui {

    template<class T>
    class DataContainer : public Component {
    public:

        using ContainerData = std::vector<T>;
        using LabelCallback = std::function<std::string(const T &)>;

        DataContainer(const ContainerData &data, LabelCallback getLabel = nullptr) :
            Component(),
            _data(data),
            _getLabel(getLabel) {
            this->setTag("DataContainer");
        }

        std::string label(const T &data) const;

    protected:
        ContainerData _data{};
        LabelCallback _getLabel{nullptr};
    };

    template<class T>
    std::string DataContainer<T>::label(const T &data) const {
        if (_getLabel) {
            return _getLabel(data);
        } else {
            return typeid(data).name();
        }
    }

    template<>
    inline std::string DataContainer<std::string>::label(const std::string &data) const {
        return _getLabel ? _getLabel(data) : data;
    }

    // BASE

//    template<class T>
//    class DataContainerBase : public Component {
//    public:
//        DataContainerBase(const ContainerData<T> &data, LabelCallback<T> getLabel) :
//            Component(),
//            _data(data),
//            _getLabel(getLabel) {
//        }
//
//    protected:
//        ContainerData<T> _data{};
//        LabelCallback<T> _getLabel{};
//    };

    // GENERIC

//    template<class T>
//    class DataContainer : public DataContainerBase<T> {
//    public:
//        DataContainer(const ContainerData<T> &data, LabelCallback<T> getLabel) :
//            DataContainerBase<T>(data, getLabel) {
//        }
//    };

    // STRING

//    template<>
//    class DataContainer<std::string> : public DataContainerBase<std::string> {
//    public:
//        DataContainer(const std::vector<std::string> &data) :
//            DataContainerBase<std::string>(data, [](const auto &item) { return item; }) {
//        }
//    };

}

