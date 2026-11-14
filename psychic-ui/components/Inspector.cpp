#include "Inspector.hpp"
#include "Icon.hpp"
#include "Label.hpp"

namespace psychic_ui {

    // region Field

    InspectorField::InspectorField(const std::string &label, const std::string &text) :
        Div() {
        tag("InspectorField");
        _defaultStyle.direction(Direction::Row)
            .alignItems(Align::Start)
            .shrink(0);
        _label = &add<Label>(label);
        _label->addClassName("name")
              .shrink(0);
        _value = &add<Div>();
        _value->addClassName("value")
              .direction(Direction::Row)
              .alignItems(Align::Center)
              .wrap(Wrap::Wrap)
              .grow(1)
              .shrink(1)
              .minWidth(0);
        _text = &_value->add<Label>();
        _text->addClassName("text");
        updateText(text);
    }

    const std::string &InspectorField::label() const {
        return _label->text();
    }

    const std::string &InspectorField::text() const {
        return _text->text();
    }

    void InspectorField::updateLabel(const std::string &label) {
        _label->text(label);
    }

    void InspectorField::updateText(const std::string &text) {
        _text->text(text)
             .display(text.empty() ? Display::None : Display::Flex);
    }

    // endregion

    // region Section

    InspectorSection::InspectorSection(const std::string &title) :
        Div() {
        tag("InspectorSection");
        _defaultStyle.direction(Direction::Column)
            .shrink(0);
        _header = &add<Div>();
        _header->addClassName("header")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .display(title.empty() ? Display::None : Display::Flex);
        _title = &_header->add<Label>(title);
        _title->addClassName("title")
              .grow(1);
        _body = &add<Div>();
        _body->addClassName("body")
             .direction(Direction::Column);
    }

    const std::string &InspectorSection::title() const {
        return _title->text();
    }

    InspectorField &InspectorSection::field(const std::string &label, const std::string &text) {
        return _body->add<InspectorField>(label, text);
    }

    // endregion

    // region Inspector

    Inspector::Inspector() :
        Div() {
        tag("Inspector");
        _defaultStyle.direction(Direction::Column);

        _heading = &add<Div>();
        _heading->addClassName("heading")
                .direction(Direction::Row)
                .alignItems(Align::Start)
                .shrink(0)
                .display(Display::None);
        _media = &_heading->add<Div>();
        _media->addClassName("media")
              .alignItems(Align::Center)
              .justify(Justify::Center)
              .shrink(0)
              .display(Display::None);
        _icon = &_media->add<Icon>();
        auto &titles = _heading->add<Div>();
        titles.addClassName("titles")
              .direction(Direction::Column)
              .grow(1)
              .shrink(1);
        _title = &titles.add<Label>();
        _title->addClassName("title");
        _subtitle = &titles.add<Label>();
        _subtitle->addClassName("subtitle")
                 .display(Display::None);
    }

    InspectorSection &Inspector::section(const std::string &title) {
        return add<InspectorSection>(title);
    }

    void Inspector::updateTitle(const std::string &title, const std::string &subtitle) {
        _title->text(title);
        _subtitle->text(subtitle)
                 .display(subtitle.empty() ? Display::None : Display::Flex);
        _heading->display(title.empty() && subtitle.empty() ? Display::None : Display::Flex);
    }

    void Inspector::updateIcon(const std::string_view icon) {
        _icon->name(icon);
        _media->display(icon.empty() ? Display::None : Display::Flex);
    }

    // endregion
}
