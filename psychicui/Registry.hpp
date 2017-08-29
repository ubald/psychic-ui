#pragma once

#include <memory>

namespace psychicui {
    class Registry {
    public:
        static std::unique_ptr <Registry> &getInstance() {
            if (!_instance) {
                _instance = std::make_unique<Registry>();
            }
            return _instance;
        }

        void registerComponentType(const std::string &componentName);

    private:
        static std::unique_ptr <Registry> _instance;
//        std::map <std::string, QuiddityDocumentation>  _componentRegistry{};
    };
}
