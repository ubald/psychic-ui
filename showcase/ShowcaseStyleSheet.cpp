#include "ShowcaseStyleSheet.hpp"

#include <psychic-ui/style/Length.hpp>
#include <psychic-ui/style/StyleManager.hpp>
#include <psychic-ui/style/Values.hpp>
#include <psychic-ui/themes/Theme.hpp>

namespace psychic_ui::showcase {
    using namespace psychic_ui::literals;

    void ShowcaseStyleSheet::load(StyleManager *manager) {
        const Theme      &theme   = manager->theme();
        const Palette    &palette = theme.palette;
        const Typography &t       = theme.type;
        const Metrics    &metrics = theme.metrics;
        const Shadow     &soft    = theme.elevation.panel;
        const Shadow     &ring    = theme.elevation.panelContact;

        // The scene fills the window edge to edge: no window border
        manager->style("Window")
               .border(0);

        // The backdrop: the app frame's color, the scene's panels centered on it
        manager->style(".backdrop")
               .backgroundColor(palette.frame)
               .alignItems(Align::Center)
               .justify(Justify::Center)
               .gap(metrics.space(6))
               .padding(metrics.space(6));

        manager->style(".backdrop.row")
               .direction(Direction::Row)
               .gap(metrics.space(4));

        // Panels from the top, leaving room under them for what opens below
        manager->style(".backdrop.top")
               .justify(Justify::Start)
               .paddingTop(metrics.space(8));

        // Panels from the left, leaving room beside them for what opens there
        manager->style(".backdrop.start")
               .alignItems(Align::Start)
               .paddingLeft(metrics.space(8));

        // A canvas behind the scene, as the Float set's app frame
        manager->style(".backdrop.dotted")
               .dotGridColor(palette.canvasDot)
               .dotGridSpacing(16)
               .dotGridRadius(1);

        // A panel floating over the backdrop, with the theme's panel elevation
        manager->style(".panel")
               .direction(Direction::Column)
               .gap(metrics.space(4))
               .padding(metrics.space(5))
               .backgroundColor(palette.surface)
               .borderRadius(metrics.radius.surface)
               .shadowColor(soft.color)
               .shadowBlur(soft.blur)
               .shadowOffsetY(soft.offsetY)
               .shadowSpread(soft.spread)
               .shadow2Color(ring.color)
               .shadow2Blur(ring.blur)
               .shadow2OffsetY(ring.offsetY)
               .shadow2Spread(ring.spread);

        // A panel with nothing between its content and its edges (a list, a grid)
        manager->style(".panel.flush")
               .padding(0)
               .gap(0)
               .overflow(Overflow::Hidden);

        // A panel's heading: a title, a line under it
        manager->style(".heading")
               .direction(Direction::Column)
               .gap(metrics.space(0.5f));

        manager->style(".heading .title")
               .fontFamily(t.medium.name)
               .fontSize(t.md.size)
               .lineHeight(t.md.leading);

        manager->style(".heading .caption")
               .color(palette.textMuted);

        // A labelled line of controls
        manager->style(".line")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .gap(metrics.space(2));

        manager->style(".line.wrap")
               .wrap(Wrap::Wrap);

        manager->style(".stack")
               .direction(Direction::Column)
               .gap(metrics.space(3));

        manager->style(".stack.tight")
               .gap(metrics.space(2));

        // A form's field: its name in a column, its control after
        manager->style(".field")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .gap(metrics.space(3))
               .minHeight(metrics.space(8));

        manager->style(".field .name")
               .width(88)
               .shrink(0)
               .color(palette.textSecondary);

        manager->style(".field .control")
               .direction(Direction::Row)
               .alignItems(Align::Center)
               .gap(metrics.space(2))
               .grow(1)
               .shrink(1);

        manager->style(".divider")
               .height(metrics.border)
               .backgroundColor(palette.line)
               .alignSelf(Align::Stretch);

        manager->style(".panel.flush .heading")
               .padding(metrics.space(4))
               .paddingBottom(metrics.space(2));

        // Lists side by side
        manager->style(".people")
               .width(360);

        manager->style(".files")
               .width(240)
               .paddingBottom(metrics.space(2));

        manager->style(".files Tree")
               .paddingLeft(metrics.space(2))
               .paddingRight(metrics.space(2));

        // A data grid beside its inspector
        manager->style(".grid-card")
               .direction(Direction::Row)
               .width(Full)
               .height(Full);

        manager->style(".grid-inspector")
               .width(280)
               .shrink(0)
               .borderLeft(metrics.border)
               .borderColor(palette.line);

        // The cue list the popups float over: its heading and actions in a bar over its rows
        manager->style(".cues")
               .width(440);

        manager->style(".cues .toolbar")
               .paddingLeft(metrics.space(5))
               .paddingRight(metrics.space(4))
               .paddingTop(metrics.space(4))
               .paddingBottom(metrics.space(4))
               .borderBottom(metrics.border)
               .borderColor(palette.line);

        manager->style(".cues .toolbar .heading")
               .padding(0);

        // A stage plot: a tool bar over a dotted canvas of fixtures
        manager->style(".plot-panel")
               .width(520);

        manager->style(".plot")
               .height(260)
               .backgroundColor(palette.frame)
               .dotGridColor(palette.canvasDot)
               .dotGridSpacing(16)
               .dotGridRadius(1)
               .borderTop(metrics.border)
               .borderColor(palette.line);

        manager->style(".plot .fixture")
               .position(Position::Absolute)
               .width(28)
               .height(28)
               .borderRadius(14)
               .border(2)
               .borderColor(palette.surface);

        // Typography: the scale beside text that wraps
        manager->style(".type-scale")
               .width(240)
               .gap(metrics.space(2.5f));

        manager->style(".type-prose")
               .width(340);

        manager->style(".type-prose .ellipsis")
               .textOverflow(TextOverflow::Ellipsis)
               .shrink(1);

        // Icons in a grid of tiles
        manager->style(".icon-grid")
               .direction(Direction::Row)
               .wrap(Wrap::Wrap)
               .gap(metrics.space(1.5f))
               .width((12 * 36) + (11 * metrics.space(1.5f)));

        manager->style(".icon-tile")
               .width(36)
               .height(36)
               .alignItems(Align::Center)
               .justify(Justify::Center)
               .borderRadius(metrics.radius.small)
               .backgroundColor(palette.raised)
               .color(palette.textSecondary)
               .iconSize(18);

        // An app's chrome over an empty page
        manager->style(".chrome")
               .width(600);

        manager->style(".chrome-body EmptyState")
               .paddingVertical(metrics.space(10));

        manager->style(".chrome-body")
               .alignItems(Align::Center)
               .justify(Justify::Center)
               .backgroundColor(palette.page);

        // The same panel at both densities
        manager->style(".density")
               .width(380);

        // What moves, alone
        manager->style(".motion-grid")
               .width(400)
               .height(300);
    }
}
