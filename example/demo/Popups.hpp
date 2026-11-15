#pragma once

#include <memory>
#include <string>
#include <vector>
#include <psychic-ui/Div.hpp>
#include <psychic-ui/Window.hpp>
#include <psychic-ui/components/Box.hpp>
#include <psychic-ui/components/Button.hpp>
#include <psychic-ui/components/Dialog.hpp>
#include <psychic-ui/components/Label.hpp>
#include <psychic-ui/components/Menu.hpp>
#include <psychic-ui/components/Select.hpp>
#include <psychic-ui/components/Switch.hpp>
#include <psychic-ui/components/TextInput.hpp>
#include <psychic-ui/components/Toast.hpp>
#include <psychic-ui/components/Tooltip.hpp>

namespace psychic_ui {
    /**
     * Everything that floats above the page: menus, selects, tooltips, dialogs and toasts
     */
    class Popups : public Div {
    public:
        Popups();

        /**
         * Open one of the page's popups by name, for screenshots: "menu", "submenu", "context", "select", "dialog",
         * "confirm", "toasts" or "tooltip"; false for another name
         */
        bool openPopup(const std::string &name);

    private:
        // The page's items keep their state between openings (check marks, the radio group's value)
        MenuItems                       _menuItems{};
        std::shared_ptr<MenuRadioGroup> _density{};
        Button                          *_menuButton{nullptr};
        Div                             *_contextArea{nullptr};
        Select                          *_select{nullptr};
        Button                          *_tooltipButton{nullptr};
        Label                           *_status{nullptr};

        Div &group(const std::string &title, const std::string &caption);
        void openDialog();
        void openConfirm();
        void showToast(ToastVariant variant, bool sticky = false);
        void say(const std::string &text);
    };

    Div &Popups::group(const std::string &title, const std::string &caption) {
        auto &group = add<VBox>().addClassName("group");
        group.add<Label>(title).addClassName("h4");
        group.add<Label>(caption).role(TextRole::Tertiary);
        return group.add<HBox>().addClassName("row");
    }

    Popups::Popups() : Div() {
        tag("Popups")
            .classNames({"demo-panel"});

        _density = std::make_shared<MenuRadioGroup>("comfortable", [this](const std::string &value) { say("Density: " + value); });

        _menuItems = {
            Menu::group("Cue"),
            Menu::item("Go", [this]() { say("Go"); }, "Space", "play"),
            Menu::item("Pause", [this]() { say("Pause"); }, "P", "pause"),
            Menu::item("Rename…", [this]() { openDialog(); }, "F2", "pencil"),
            Menu::item(
                "Move to", {
                    Menu::item("Act 1", [this]() { say("Moved to act 1"); }, "", "folder"),
                    Menu::item("Act 2", [this]() { say("Moved to act 2"); }, "", "folder"),
                    Menu::item(
                        "Later", {
                            Menu::item("Encore", [this]() { say("Moved to the encore"); }),
                        }
                    ),
                }, "", "move"
            ),
            Menu::separator(),
            Menu::group("View"),
            Menu::check("Show fades", true, [this](bool on) { say(on ? "Fades shown" : "Fades hidden"); }),
            Menu::check("Show timecode", false, [this](bool on) { say(on ? "Timecode shown" : "Timecode hidden"); }, "T"),
            Menu::radio(_density, "Comfortable", "comfortable"),
            Menu::radio(_density, "Compact", "compact"),
            Menu::separator(),
            Menu::item("Archive", []() {}, "", "archive"),
            Menu::item("Delete", [this]() { openConfirm(); }, "Del", "trash"),
        };
        _menuItems[2]->enabled = false; // Pause: nothing is running
        _menuItems.back()->danger = true;

        // region Menus

        auto &menus = group("Menus", "Icons, shortcuts, groups, check marks, a radio group and submenus; arrows, Enter and Escape; the area's context menu by a right press, a long press or the Menu key");
        _menuButton = &menus.add<Button>("Cue actions").icon("more");
        _menuButton->onClick([this]() { window()->openMenu(_menuItems, *_menuButton); });

        _contextArea = &menus.add<Div>();
        _contextArea->addClassName("context-area")
                     .alignItems(Align::Center)
                     .justify(Justify::Center);
        _contextArea->add<Label>("Right-click or long-press here").role(TextRole::Tertiary);
        // Opens where it is asked for: a right press, a touch long press, or the Menu key (Shift+F10) once focused,
        // at the area's center and at its first item
        _contextArea->focusEnabled(true)
                     .tooltip("Right-click for the menu")
                     .contextMenu(
                         {
                             Menu::item("Cut", [this]() { say("Cut"); }, "Ctrl+X"),
                             Menu::item("Copy", [this]() { say("Copy"); }, "Ctrl+C"),
                             Menu::item("Paste", [this]() { say("Paste"); }, "Ctrl+V"),
                             Menu::separator(),
                             Menu::item("Select all", [this]() { say("Select all"); }, "Ctrl+A"),
                         }
                     );

        // endregion

        // region Selects

        auto &selects = group("Selects", "A closed field opening its options below it; Up, Down, Enter, Escape");
        _select = &selects.add<Select>(std::vector<std::string>{"Linear", "Ease in", "Ease out", "Ease in and out", "Snap"}, 3);
        _select->width(180);
        _select->onChange([this](int /*index*/) { say("Curve: " + _select->value()); });
        selects.add<Select>(std::vector<std::string>{"Universe 1", "Universe 2", "Universe 3"})
               .placeholder("Pick a universe…")
               .width(180);
        selects.add<Select>(std::vector<std::string>{"Locked"}, 0)
               .enabled(false)
               .width(120);

        // endregion

        // region Tooltips

        auto &tooltips = group("Tooltips", "After 300 ms of rest, at once for the next; a touch long press shows one when no menu opens");
        _tooltipButton = &tooltips.add<Button>("Hover me").tooltip("Tooltips sit above, flipped below without room");
        tooltips.add<Button>().icon("search").addClassName("icon-button").tooltip("Search");
        tooltips.add<Button>().icon("settings").addClassName("icon-button").tooltip("Settings");
        tooltips.add<Button>().icon("trash").addClassName("icon-button").tooltip("Delete the cue");

        // endregion

        // region Dialogs

        auto &dialogs = group("Dialogs", "Modal over a scrim; Tab stays inside, Escape or the backdrop closes, the focus comes back");
        dialogs.add<Button>("Rename cue…").icon("pencil").onClick([this]() { openDialog(); });
        dialogs.add<Button>("Delete cue…").icon("trash").skin("danger").onClick([this]() { openConfirm(); });

        // endregion

        // region Toasts

        auto &toasts = group("Toasts", "Stacked in the corner, gone after 5 s (held while the mouse is over them)");
        toasts.add<Button>("Info").icon("info").onClick([this]() { showToast(ToastVariant::Info); });
        toasts.add<Button>("Success").icon("circle-check").onClick([this]() { showToast(ToastVariant::Success); });
        toasts.add<Button>("Warning").icon("warning").onClick([this]() { showToast(ToastVariant::Warning); });
        toasts.add<Button>("Danger").icon("circle-alert").onClick([this]() { showToast(ToastVariant::Danger); });

        // endregion

        // region Motion

        auto &motion = group("Motion", "Popups fade and scale from their anchor as they open and close, a little quicker out; tooltips drift toward theirs, toasts slide in from the edge");
        // Ten times slower, to watch them (as with --slow-motion)
        motion.add<Switch>("Slow motion").onChange([this](bool on) {
            if (Window *w = window()) {
                w->animationSpeed(on ? 0.1f : 1.0f);
            }
        });

        // endregion

        _status = &add<Label>("Nothing chosen yet");
        _status->addClassName("status").role(TextRole::Secondary);
    }

    void Popups::say(const std::string &text) {
        _status->text(text);
    }

    void Popups::openDialog() {
        auto &dialog = window()->overlay().open<Dialog>("Rename cue");
        dialog.icon("pencil")
              .description("Cue names show in the cue list and on the stage manager's screen.");
        dialog.body().add<TextInput>("Opening — house to half");
        dialog.action("Cancel");
        dialog.action("Rename", [this]() { say("Renamed"); }, "primary");
    }

    void Popups::openConfirm() {
        auto &dialog = window()->overlay().open<Dialog>("Delete cue 12?");
        dialog.icon("trash")
              .description("Its fades and follow-ons go with it. This can't be undone.")
              .dismissible(false);
        dialog.action("Cancel");
        dialog.action("Delete", [this]() { say("Deleted"); }, "danger");
    }

    void Popups::showToast(const ToastVariant variant, const bool sticky) {
        ToastOptions options{};
        options.variant = variant;
        switch (variant) {
            case ToastVariant::Info:
                options.title       = "Show file saved";
                options.description = "Saved to Lights/Opening night.show";
                break;
            case ToastVariant::Success:
                options.title       = "Patch complete";
                options.description = "48 fixtures on universe 1.";
                break;
            case ToastVariant::Warning:
                options.title       = "Universe 2 is not responding";
                options.description = "Its node went quiet 30 s ago.";
                options.actions     = {
                    {"Retry", [this]() { say("Retrying universe 2"); }, true},
                    {"Later", nullptr},
                };
                break;
            case ToastVariant::Danger:
                options.title       = "Cue 12 deleted";
                options.description = "";
                options.timeout     = std::chrono::nanoseconds::zero(); // Until closed
                options.actions     = {{"Undo", [this]() { say("Cue 12 restored"); }, true}};
                break;
        }
        if (sticky) {
            options.timeout = std::chrono::nanoseconds::zero();
        }
        window()->toast(options);
    }

    bool Popups::openPopup(const std::string &name) {
        Window *w = window();
        if (!w) {
            return false;
        }
        if (name == "menu" || name == "submenu") {
            Menu &menu = w->openMenu(_menuItems, *_menuButton);
            if (name == "submenu") {
                w->updateStylesAndLayout(); // The submenu lines up with its row
                menu.highlight(3, true); // "Move to", the fourth row
            }
        } else if (name == "context") {
            // As a right press in the area, left of its center
            const MouseEvent center = _contextArea->mouseEventAtCenter();
            w->openContextMenu(_contextArea->contextMenu(), center.windowX - 60, center.windowY);
        } else if (name == "select") {
            _select->open();
        } else if (name == "dialog") {
            openDialog();
        } else if (name == "confirm") {
            openConfirm();
        } else if (name == "toasts") {
            showToast(ToastVariant::Info, true); // Until closed, for a screenshot
            showToast(ToastVariant::Warning, true);
            showToast(ToastVariant::Success, true);
        } else if (name == "tooltip") {
            auto &tooltip = w->overlay().open<Tooltip>(_tooltipButton->tooltip());
            tooltip.anchor(*_tooltipButton);
        } else {
            return false;
        }
        return true;
    }
}
