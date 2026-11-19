#pragma once

namespace psychic_ui {

    /**
     * How a native window's units map to the logical units the UI is laid out in, and where the
     * user's display scale (125 %, 150 %, 200 %…) goes.
     *
     * Platforms report window sizes and the mouse in their own units: points on macOS and Wayland,
     * where the framebuffer is larger than the window by the scale (the pixel density), and pixels on
     * Windows and X11, where the density is 1 and the scale is apart (SDL's display scale, GLFW's content
     * scale). Logical units are window units divided by the part of the scale the density doesn't
     * already carry: the same as window units on macOS and Wayland, half of them on Windows at 200 %.
     * The paint ratio is then the framebuffer's pixels per logical unit, whatever the platform.
     */
    struct WindowScale {
        /**
         * Logical units per window unit: 1 where window units already are logical, 1 / scale where
         * they are pixels
         */
        float logicalPerWindowUnit{1.0f};

        /**
         * From SDL's pixel density (pixels per window unit, `SDL_GetWindowPixelDensity`) and display scale
         * (pixels per logical unit, `SDL_GetWindowDisplayScale`). Either unknown (0, SDL's failure) leaves
         * window units as they are.
         */
        [[nodiscard]] static WindowScale fromDisplay(float pixelDensity, float displayScale);

        /**
         * From GLFW's window and framebuffer widths (their ratio is the density) and content scale
         * (`glfwGetWindowContentScale`, pixels per logical unit). A window without a size (minimized)
         * leaves window units as they are.
         */
        [[nodiscard]] static WindowScale fromFramebuffer(int windowWidth, int framebufferWidth, float contentScale);

        /**
         * Framebuffer pixels per logical unit, what the window is painted at: 1 without a logical width
         */
        [[nodiscard]] static float pixelRatio(int framebufferWidth, int logicalWidth);

        /**
         * A coordinate (the mouse's, a touch's) from window units to logical units
         */
        [[nodiscard]] float toLogical(float windowUnits) const;

        /**
         * A size or a point from window units to logical units, to the nearest unit
         */
        [[nodiscard]] int toLogical(int windowUnits) const;

        /**
         * A size or a point from logical units to window units (a size the app asks for), to the nearest unit
         */
        [[nodiscard]] int toWindow(int logical) const;
    };
}
