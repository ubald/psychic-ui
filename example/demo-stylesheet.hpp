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
        const Theme   &theme = manager->theme();
        const Palette &p     = theme.palette;
        const Metrics &m     = theme.metrics;

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

        // region Popups

        manager->style("Popups")
               .gap(m.space(8));

        manager->style("Popups .group")
               .shrink(0)
               .gap(m.space(1));

        manager->style("Popups .group .row")
               .wrap(Wrap::Wrap)
               .alignItems(Align::Center)
               .gap(m.space(2))
               .marginTop(m.space(2));

        // Where a context menu opens
        manager->style("Popups .context-area")
               .width(280)
               .height(64)
               .marginLeft(m.space(4))
               .border(m.border)
               .borderRadius(m.radius.surface)
               .borderColor(p.lineStrong);

        manager->style("Popups .context-area:focus-visible")
               .outlineColor(p.accent)
               .outlineWidth(m.focusRing)
               .outlineOffset(m.focusRingOffset);

        manager->style("Popups .status")
               .marginTop(-m.space(4));

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
               .maxWidth((10 * 20) + (9 * 4));

        manager->style("Inputs .event")
               .marginTop(m.space(1));

        // endregion

        // region Motion

        manager->style("Motion")
               .gap(m.space(6));

        manager->style("Motion .group")
               .shrink(0)
               .gap(m.space(1));

        manager->style("Motion .row")
               .wrap(Wrap::Wrap)
               .alignItems(Align::Center)
               .gap(m.space(4))
               .marginTop(m.space(2));

        manager->style("Motion .lane-row")
               .alignItems(Align::Center)
               .gap(m.space(3))
               .marginTop(m.space(1.5f));

        manager->style("Motion .lane-row .name")
               .width(200)
               .shrink(0);

        // A track, its rail the track less the puck, the puck moving along the rail
        manager->style("Motion .lane")
               .width(320)
               .height(24)
               .padding(2)
               .borderRadius(m.radius.button)
               .backgroundColor(p.raised)
               .color(p.accent);

        manager->style("Motion .rail")
               .grow(1)
               .marginRight(20);

        manager->style("Motion .puck")
               .position(Position::Absolute)
               .top(0)
               .width(20)
               .height(20)
               .borderRadius(m.radius.button)
               .backgroundColor(p.accent);

        manager->style("Motion .shuffle")
               .width(320)
               .marginTop(m.space(2));

        manager->style("Motion .curves")
               .wrap(Wrap::Wrap)
               .gap(m.space(4))
               .marginTop(m.space(2));

        manager->style("Motion .curve-cell")
               .alignItems(Align::Center)
               .gap(m.space(1));

        manager->style("Motion .curve")
               .width(96)
               .height(96)
               .borderRadius(m.radius.surface)
               .backgroundColor(p.surface)
               .borderColor(p.line)
               .color(p.accent);

        manager->style("Motion .tokens")
               .marginTop(m.space(2));

        manager->style("Motion .slider")
               .width(200);

        manager->style("Motion TextInput")
               .width(200);

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

        // region Canvas

        // The page is the canvas: the frame's color, dotted every 16 px
        manager->style("CanvasPage")
               .position(Position::Relative)
               .overflow(Overflow::Hidden)
               .backgroundColor(p.frame)
               .dotGridColor(p.canvasDot)
               .dotGridSpacing(16)
               .dotGridRadius(1);

        // Panels float over it with the theme's panel elevation
        const Shadow &soft    = theme.elevation.panel;
        const Shadow &contact = theme.elevation.panelContact;
        manager->style("CanvasPage .floating")
               .backgroundColor(p.surface)
               .borderRadius(m.radius.surface)
               .shadowColor(soft.color)
               .shadowBlur(soft.blur)
               .shadowOffsetY(soft.offsetY)
               .shadowSpread(soft.spread)
               .shadow2Color(contact.color)
               .shadow2Blur(contact.blur)
               .shadow2OffsetY(contact.offsetY)
               .shadow2Spread(contact.spread);

        // Layers fill the canvas to place what they hold (the stage in the middle, the tool bar at the bottom)
        manager->style("CanvasPage .fill")
               .position(Position::Absolute)
               .left(0).top(0).right(0).bottom(0)
               .alignItems(Align::Center)
               .justify(Justify::Center);

        manager->style("CanvasPage .left")
               .position(Position::Absolute)
               .left(m.space(4))
               .top(m.space(4))
               .width(232)
               .gap(m.space(2));

        manager->style("CanvasPage .document")
               .alignSelf(Align::Start)
               .alignItems(Align::Center)
               .gap(m.space(2))
               .height(44)
               .paddingLeft(m.space(2.5f))
               .paddingRight(m.space(3))
               .iconSize(m.iconSm)
               .color(p.textMuted);

        manager->style("CanvasPage .document .mark")
               .width(24)
               .height(24)
               .textAlign(TextAlign::Center)
               .lineHeight(24)
               .borderRadius(m.radius.small)
               .fontFamily(theme.type.semiBold.name)
               .fontSize(theme.type.xs.size)
               .backgroundColor(p.accent)
               .color(p.onAccent);

        manager->style("CanvasPage .document .name")
               .fontFamily(theme.type.semiBold.name)
               .fontSize(theme.type.md.size)
               .lineHeight(theme.type.md.leading)
               .color(p.text);

        manager->style("CanvasPage .layers")
               .padding(m.space(2))
               .paddingTop(m.space(1));

        // The layers' tree sits on the panel, not on a page: its sliding nodes hide what they pass over in its color
        manager->style("CanvasPage .layers TreeNode.sliding")
               .backgroundColor(p.surface);

        manager->style("CanvasPage .layers .heading")
               .fontSize(theme.type.xs.size)
               .lineHeight(theme.type.xs.leading)
               .paddingHorizontal(m.space(2))
               .paddingTop(m.space(2))
               .paddingBottom(m.space(1))
               .color(p.textMuted);

        manager->style("CanvasPage .inspector")
               .position(Position::Absolute)
               .right(m.space(4))
               .top(m.space(4))
               .width(300)
               .overflow(Overflow::Hidden); // Its sections' hairlines end at its corners

        manager->style("CanvasPage .inspector .actions")
               .gap(m.space(2))
               .padding(m.gutter)
               .borderTop(m.border)
               .borderColor(p.line);

        manager->style("CanvasPage .inspector InspectorField")
               .alignItems(Align::Center);

        manager->style("CanvasPage .inspector InspectorField .name")
               .width(80);

        manager->style("CanvasPage .inspector NumberStepper")
               .width(112);

        manager->style("CanvasPage .dock")
               .position(Position::Absolute)
               .left(0).right(0).bottom(m.space(5))
               .justify(Justify::Center);

        // The tool bar: a floating strip of square quiet tools, not a band under a header
        manager->style("CanvasPage ToolBar")
               .height(Auto)
               .padding(m.space(1.5f))
               .gap(m.space(1))
               .borderBottom(0);

        manager->style("CanvasPage ToolBar Button")
               .width(36)
               .height(36)
               .paddingHorizontal(0)
               .iconSize(theme.metrics.iconMd + 3)
               .borderRadius(m.radius.control);

        // The stage: a lighting plot in the middle of the canvas
        manager->style("CanvasPage .stage")
               .position(Position::Relative)
               .width(468)
               .height(300)
               .border(m.border)
               .borderColor(p.lineStrong)
               .borderRadius(m.radius.control)
               .backgroundColor(p.page.withAlpha(0x99)); // The canvas shows through, a little

        manager->style("CanvasPage .truss")
               .position(Position::Absolute)
               .left(40).right(40).top(67)
               .height(2)
               .backgroundColor(p.lineStrong);

        manager->style("CanvasPage .fixture")
               .position(Position::Absolute)
               .width(28)
               .height(28)
               .borderRadius(999)
               .border(2)
               .borderColor(p.lineStrong);

        manager->style("CanvasPage .selection")
               .position(Position::Absolute)
               .left(284).top(46)
               .width(124).height(44)
               .border(1.5f)
               .borderColor(p.accent)
               .borderRadius(2)
               .alignItems(Align::Center);

        manager->style("CanvasPage .selection-label")
               .position(Position::Absolute)
               .top(-30)
               .paddingHorizontal(m.space(2))
               .height(24)
               .borderRadius(m.radius.small)
               .fontFamily(theme.type.medium.name)
               .fontSize(theme.type.sm.size)
               .lineHeight(24)
               .backgroundColor(p.accent)
               .color(p.onAccent);

        manager->style("CanvasPage .caption")
               .position(Position::Absolute)
               .left(m.space(3))
               .bottom(m.space(2))
               .fontSize(theme.type.sm.size)
               .color(p.textMuted);

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
