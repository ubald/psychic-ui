#pragma once

#include <string>
#include <chrono>
#include <unordered_map>
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

        const int windowX() const;
        const int windowY() const;
        void setWindowPosition(const int x, const int y);

        const int windowWidth() const;
        const int windowHeight() const;
        void setWindowSize(const int width, const int height);

        void open(SystemWindow *systemWindow);
        void close();
        void drawAll();
        virtual void drawContents();
        void drawComponents();

        void openMenu(const std::vector<std::shared_ptr<MenuItem>> &items, const int x, const int y);
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

        int cursor() const;
        void setCursor(int cursor);

        // region Signals

        using MenuOpenedSlot = std::shared_ptr<Slot<>>;
        using MenuClosedSlot = std::shared_ptr<Slot<>>;

        Signal<> onMenuOpened{};
        Signal<> onMenuClosed{};

        // endregion

        // Style

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


        // Window Delegate
        virtual void windowMoved(const int x, const int y);
        virtual void windowResized(const int width, const int height);
        virtual void windowActivated();
        virtual void windowDeactivated();
        virtual void windowMinimized();
        virtual void windowRestored();
        virtual bool windowShouldClose();


        virtual bool dropEvent(const std::vector<std::string> & /* filenames */) { return false; /* To be overridden */ }
        bool keyboardEvent(int key, int scancode, int action, int modifiers) override;
        bool keyboardCharacterEvent(unsigned int codepoint) override;


    protected:

        // region Default DIVs

        std::shared_ptr<Div>   app{};
        std::shared_ptr<Modal> modal{};
        std::shared_ptr<Modal> menu{};

        // endregion

        // region Rendering

        SystemWindow *_systemWindow{nullptr};
        GrContext   *_sk_context{nullptr};
        SkSurface   *_sk_surface{nullptr};
        SkCanvas    *_sk_canvas{nullptr};

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

        int _mouseX{0};
        int _mouseY{0};


        /**
         * Mouse focus path
         */
        std::vector<Div *> _focusPath{};

        // endregion

        // region Initialization

        void initSkia();
        void getSkiaSurface();

        // endregion

        // region Events

        double _lastInteraction{0};



        // Event handlers


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

