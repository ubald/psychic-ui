#pragma once

#include <string>
#include <vector>
#include "psychic-ui/Div.hpp"

namespace psychic_ui {
    /**
     * @class Kbd
     *
     * A keyboard shortcut as key caps (the `.key` parts), one per key: the keys are split on spaces, so
     * `Kbd("⌘ K")` is two caps and `Kbd("Ctrl Shift P")` three
     */
    class Kbd : public Div {
    public:
        explicit Kbd(const std::string &keys = "");

        /**
         * The keys, as given
         */
        const std::string &keys() const { return _keys; }

        template<class Self>
        Self &keys(this Self &self, const std::string &keys) {
            static_cast<Kbd &>(self).updateKeys(keys);
            return self;
        }

        /**
         * The caps' labels, in order
         */
        std::vector<std::string> caps() const;

    protected:
        std::string _keys{};

        void updateKeys(const std::string &keys);
    };
}
