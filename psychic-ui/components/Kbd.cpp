#include <sstream>
#include "Kbd.hpp"
#include "Label.hpp"

namespace psychic_ui {
    Kbd::Kbd(const std::string &keys) :
        Div() {
        tag("Kbd");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Center)
            .alignSelf(Align::Center)
            .shrink(0);
        updateKeys(keys);
    }

    std::vector<std::string> Kbd::caps() const {
        std::vector<std::string> caps{};
        for (const auto &child: _children) {
            if (auto label = std::dynamic_pointer_cast<Label>(child)) {
                caps.push_back(label->text());
            }
        }
        return caps;
    }

    void Kbd::updateKeys(const std::string &keys) {
        _keys = keys;
        removeAll();
        std::istringstream stream{keys};
        std::string        key;
        while (stream >> key) {
            add<Label>(key).addClassName("key");
        }
    }
}
