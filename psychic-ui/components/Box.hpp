#pragma once

#include "psychic-ui/Div.hpp"

namespace psychic_ui {

    class Box : public Div {
    public:
        int getGap() const;
        void setGap(int gap);
    protected:
        Box() = default;
        explicit Box(int gap);
        int _gap{0};
        virtual void updateRules() = 0;
        void styleUpdated() override;
        void createStyles() override;
    };

    /**
     * Helper container for quick & easy horizontal layouts
     */
    class HBox : public Box {
    public:
        HBox();
        explicit HBox(int gap);
    protected:
        void updateRules() override ;
    };


    /**
     * Helper container for quick & easy vertical layouts
     */
    class VBox : public Box {
    public:
        VBox();
        explicit VBox(int gap);
    protected:
        void updateRules() override;
    };
}
