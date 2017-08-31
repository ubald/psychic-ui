#pragma once

#include <psychicui/Component.hpp>

namespace psychicui {
    class MenuBar : public Component {
    public:
        MenuBar();
    protected:
        void styleUpdated() override;
    };
}
