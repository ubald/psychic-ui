#pragma once

#include <string>
#include <chrono>
#include <memory>
#include <unordered_map>
#include <unicode/unistr.h>
#include "GrContext.h"
#include "SkSurface.h"
#include "SkCanvas.h"
#include "Div.hpp"
#include "Modal.hpp"
#include "style/StyleManager.hpp"
#include "style/Style.hpp"
#include "components/Menu.hpp"
#include "signals/Signal.hpp"
#include "Application.hpp"

namespace psychic_ui {

    class Window : public Div {
    public:
        explicit Window(const std::string &title);
        virtual ~Window();

        Window *window() override;

        const std::string getTitle() const;
        void setTitle(const std::string &title);

        bool getResizable() const;
        void setResizable(bool resizable);

        bool getDecorated() const;
        void setDecorated(bool decorated);

        void toggleMinimized();
        bool minimized() const;
        void setMinimized(bool minimized);

        void toggleMaximized();
        bool maximized() const;
        void setMaximized(bool minimized);

        void toggleFullscreen();
        bool getFullscreen() const;
        void setFullscreen(bool fullscreen);

        void setVisible(bool value) override;

        void startDrag();
        void stopDrag();

        int windowX() const;
        int windowY() const;
        void setWindowPosition(int x, int y);

        int windowWidth() const;
        int windowHeight() const;
        void setWindowSize(int width, int height);

        void open(SystemWindow *systemWindow);
        void close();
        void drawAll();
        virtual void drawContents();
        void drawComponents();

        void openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, int x, int y);
        void closeMenu();

        std::shared_ptr<Div> appContainer() const {
            return app;
        };

        std::shared_ptr<Modal> modalContainer() const {
            return modal;
        }

        std::shared_ptr<Modal> menuContainer() const {
            return menu;
        }

        void setCursor(int cursor);

        // region Signals

        using MenuOpenedSlot = std::shared_ptr<Slot<>>;
        using MenuClosedSlot = std::shared_ptr<Slot<>>;

        Signal<> onMenuOpened{};
        Signal<> onMenuClosed{};

        // endregion

        // region Style

        /**
         * Helper to forward the stylesheet loading to the style manager
         * and refresh our children's runtime styles
         * @tparam T :StyleSheet
         */
        template<typename T>
        void loadStyleSheet(bool reset = false) {
            styleManager()->loadStyleSheet<T>(reset);
            updateRuntimeStyles();
        }

        // endregion

        // region Focus

        void requestFocus(Div *component);

        // endregion



        // region Window Delegate

        virtual void windowMoved(int x, int y);
        virtual void windowResized(int width, int height);
        virtual void windowActivated();
        virtual void windowDeactivated();
        virtual void windowMinimized();
        virtual void windowRestored();
        virtual bool windowShouldClose();


        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }

        // endregion

        // region Mouse
        MouseEventStatus mouseButton(int mouseX, int mouseY, int button, bool down, int modifiers) override;
        std::chrono::time_point<std::chrono::high_resolution_clock> _lastClick{};
        // endregion

        // region Keyboard

        void startTextInput();
        void stopTextInput();

        bool keyDown(Key key, Mod mod);
        bool keyRepeat(Key key, Mod mod);
        bool keyUp(Key key, Mod mod);
        bool keyboardCharacterEvent(const UnicodeString &character);

        // endregion


    protected:

        // region Default DIVs

        std::shared_ptr<Div>   app{};
        std::shared_ptr<Modal> modal{};
        std::shared_ptr<Modal> menu{};

        // endregion

        // region Rendering

        SystemWindow *_systemWindow{nullptr};
        GrContext    *_sk_context{nullptr};
        SkSurface    *_sk_surface{nullptr};
        SkCanvas     *_sk_canvas{nullptr};

        // endregion

        // region Window

        std::string _title;
        bool        _fullscreen{false};
        bool        _resizable{true};
        bool        _decorated{true};

        //int   _fbWidth{0};
        //int   _fbHeight{0};
        float _pixelRatio;

        // endregion

        // region Mouse

        /**
         * Cursor to use when this component has the mouse focus
         */
        int _cursor{Cursor::Arrow};

        //int _mouseX{0};
        //int _mouseY{0};

        // endregion

        // region Initialization

        void initSkia();
        void getSkiaSurface();

        // endregion

        // region Focus

        std::vector<Div *> _focusPath{};

        // endregion

        // region Events

        // endregion

        // Performances
        double fps{0.0f};

    private:
        // Dont allow direct manipulation by others than the window itself
        using Div::add;
        using Div::remove;
        using Div::removeAll;

        // Performance
        std::chrono::time_point<std::chrono::high_resolution_clock> lastReport;
        int                                                         frames = 0;
    };
}

