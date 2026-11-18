#include <memory>
#include <string>
#include <string_view>
#include "List.hpp"
#include "Avatar.hpp"
#include "Icon.hpp"
#include "psychic-ui/Div.hpp"
#include "Label.hpp"
#include "Rows.hpp"
#include "psychic-ui/style/Values.hpp"

namespace psychic_ui {

    // region Row

    ListRow::ListRow(const std::string &title, const std::string &secondary) :
        SelectableRow() {
        tag("ListRow");
        _mouseChildren = false; // The row takes the presses, not its parts

        auto &text = add<Div>();
        text.addClassName("text")
            .grow(1)
            .shrink(1);
        _title = &text.add<Label>(title);
        _title->addClassName("title");
        _secondary = &text.add<Label>();
        _secondary->addClassName("secondary")
                  .display(Display::None);
        updateSecondary(secondary);
    }

    Div &ListRow::leading() {
        if (!_leading) {
            _leading = &add(0, std::make_shared<Div>());
            _leading->addClassName("leading");
        }
        return *_leading;
    }

    Div &ListRow::trailing() {
        if (!_trailing) {
            _trailing = &add<Div>();
            _trailing->addClassName("trailing");
        }
        return *_trailing;
    }

    void ListRow::updateSecondary(const std::string &secondary) {
        _secondary->text(secondary)
                  .display(secondary.empty() ? Display::None : Display::Flex);
    }

    void ListRow::updateIcon(const std::string_view icon) {
        Div &lead = leading();
        lead.removeAll();
        lead.add<Icon>(icon).addClassName("icon");
    }

    void ListRow::updateAvatar(const std::string &name) {
        Div &lead = leading();
        lead.removeAll();
        lead.add<Avatar>(name);
    }

    void ListRow::updateMeta(const std::string &meta) {
        if (!_meta) {
            _meta = &trailing().add(0, std::make_shared<Label>());
            _meta->addClassName("meta");
        }
        _meta->text(meta)
             .display(meta.empty() ? Display::None : Display::Flex);
    }

    // endregion

    // region List

    List::List() :
        RowGroup() {
        tag("List");
        _defaultStyle.direction(Direction::Column)
            .shrink(0);
    }

    ListRow *List::selectedRow() const {
        return dynamic_cast<ListRow *>(_selection);
    }

    void List::selectionUpdated() {
        selectionChanged.emit(selectedRow());
    }

    // endregion
}
