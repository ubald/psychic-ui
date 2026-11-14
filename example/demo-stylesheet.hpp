#pragma once

#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/StyleSheet.hpp>

using namespace psychic_ui;

/**
 * The demo's own rules, over the skin set's, from the same theme
 */
class DemoStyleSheet : public StyleSheet {
public:
    void load(StyleManager *manager) override {
        const Palette &p = manager->theme().palette;
        const Metrics &m = manager->theme().metrics;

        manager->loadFont("stan0755", "fonts/stan0755.ttf");

        manager->style(".demo-panel")
               .padding(m.space(6));

        // The menus after the page's title, at the same place on every page
        manager->style("PageHeader .title")
               .width(96);

        // region Buttons

        manager->style("Buttons")
               .gap(m.space(8));

        manager->style("Buttons .group")
               .shrink(0)
               .gap(m.space(1));

        manager->style("Buttons .group .row")
               .wrap(Wrap::Wrap)
               .alignItems(Align::Center)
               .gap(m.space(2))
               .marginTop(m.space(2));

        manager->style("Buttons .group .row CheckBox")
               .marginRight(m.space(4));

        // endregion

        // region Ranges

        manager->style("Ranges .columns")
               .gap(m.space(12));

        manager->style("Ranges .column")
               .width(360)
               .gap(m.space(1));

        manager->style("Ranges .field")
               .alignItems(Align::Center)
               .gap(m.space(3))
               .marginTop(m.space(2));

        manager->style("Ranges .field .name")
               .width(72);

        manager->style("Ranges .verticals")
               .height(160)
               .gap(m.space(6))
               .marginTop(m.space(3));

        // endregion

        // region Inputs

        manager->style("Inputs")
               .gap(m.space(8));

        manager->style("Inputs .group")
               .shrink(0)
               .gap(m.space(1));

        manager->style("Inputs .group .row")
               .wrap(Wrap::Wrap)
               .alignItems(Align::Center)
               .gap(m.space(4))
               .marginTop(m.space(2));

        manager->style("Inputs SearchField")
               .width(240);

        // Ten chips a row: a grid
        manager->style("Inputs ColorSwatches")
               .maxWidth(10 * 20 + 9 * 4);

        manager->style("Inputs .event")
               .marginTop(m.space(1));

        // endregion

        // region Divs and scrollers

        manager->style("Divs .demo-div")
               .margin(12)
               .width(128.0f)
               .height(64.0f)
               .backgroundColor(p.raised)
               .borderColor(p.lineStrong);

        manager->style("Scrollers .box")
               .shrink(0)
               .margin(12)
               .width(128.0f)
               .height(64.0f)
               .borderRadius(m.radius.control)
               .backgroundColor(p.raised);

        manager->style("Scrollers Scroller")
               .margin(12)
               .border(m.border)
               .borderRadius(m.radius.surface)
               .borderColor(p.line);

        // endregion

        manager->style("TextInputs TextArea")
               .height(120);

        // region Colors

        manager->style("Colors .swatch")
               .height(32.0f)
               .borderRadius(m.radius.small);

        manager->style("Colors .swatches")
               .direction(Direction::Row)
               .gap(2);

        manager->style("Colors .color-schemes .title")
               .marginTop(12);

        manager->style("Colors .swatches .swatch")
               .grow(1);

        manager->style("Colors .column Range")
               .shrink(0);

        // endregion

        // region Lists

        manager->style("Lists .scroller")
               .padding(m.space(6))
               .gap(m.space(8));

        manager->style("Lists .columns")
               .shrink(0)
               .alignItems(Align::Start)
               .gap(m.space(8));

        manager->style("Lists .group")
               .shrink(0)
               .gap(m.space(1));

        manager->style("Lists .group .row")
               .wrap(Wrap::Wrap)
               .alignItems(Align::Center)
               .gap(m.space(2))
               .marginTop(m.space(2));

        // Lists and grids sit edge to edge in a card, as on a page
        manager->style("Lists .card")
               .shrink(0)
               .marginTop(m.space(3))
               .border(m.border)
               .borderColor(p.line)
               .borderRadius(m.radius.surface)
               .backgroundColor(p.page)
               .overflow(Overflow::Hidden);

        manager->style("Lists .card List")
               .borderTop(0);

        manager->style("Lists .tree-card")
               .padding(m.space(2));

        manager->style("Lists .grid-card")
               .height(360)
               .alignItems(Align::Stretch);

        manager->style("Lists .dot-item")
               .alignItems(Align::Center)
               .gap(m.space(1.5f))
               .marginRight(m.space(2));

        // endregion

        // region Icons

        // In a scroller: keep their content height
        manager->style("Icons .inter-samples")
               .shrink(0)
               .direction(Direction::Row)
               .gap(32)
               .marginBottom(16);

        manager->style("Icons .inter-samples .column")
               .gap(4);

        manager->style("Icons .icon-grid")
               .shrink(0)
               .gap(4);

        manager->style("Icons .icon-row")
               .direction(Direction::Row)
               .gap(8);

        manager->style("Icons .icon-cell")
               .width(116.0f)
               .padding(4)
               .gap(4)
               .alignItems(Align::Center);

        manager->style("Icons .icon-cell .icons")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .gap(12);

        manager->style("Icons .icon-cell Icon")
               .color(p.text);

        manager->style("Icons .icon-cell Label")
               .fontSize(10)
               .color(p.textMuted);

        // endregion
    }
};
