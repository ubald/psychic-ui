#pragma once

#include <psychicui/components/DataContainer.hpp>
#include <psychicui/components/Button.hpp>
#include <psychicui/components/Menu.hpp>

namespace psychicui {
    class MenuBar : public DataContainer<std::shared_ptr<MenuItem>> {
    public:
        explicit MenuBar(const std::vector<std::shared_ptr<MenuItem>> &items);
    protected:
        std::shared_ptr<Div> getItemDiv(const std::shared_ptr<MenuItem> &item);
    };



}
