// Swaps the captures for the skin set and theme picked, and the page's own theme with them.
// Every capture is showcase/<scene>-<set>-<scheme>.png, next to this page once published (see showcase/ and
// tools/showcase/publish.sh in the repository).
(function () {
    const root = document.documentElement;
    const dark = window.matchMedia("(prefers-color-scheme: dark)");
    const reduced = window.matchMedia("(prefers-reduced-motion: reduce)");
    const themeNames = {
        default: {dark: "Pivotal Dark", light: "Pivotal Light"},
        desk: {dark: "Desk Dark", light: "Desk Light"},
        float: {dark: "Float Dark", light: "Float Light"},
    };
    const setNames = {default: "Default", desk: "Desk", float: "Float"};

    const stored = (key, fallback) => {
        try {
            return localStorage.getItem(key) || fallback;
        } catch (e) {
            return fallback;
        }
    };
    const store = (key, value) => {
        try {
            localStorage.setItem(key, value);
        } catch (e) {
            // Private windows: the choice lasts the visit
        }
    };

    const state = {
        set: stored("psychic-ui.set", "default"),
        theme: stored("psychic-ui.theme", "auto"),
    };
    if (!setNames[state.set]) state.set = "default";
    if (!["auto", "light", "dark"].includes(state.theme)) state.theme = "auto";

    const scheme = () => state.theme === "auto" ? (dark.matches ? "dark" : "light") : state.theme;

    function source(img) {
        const set = img.dataset.set || state.set;
        const still = img.dataset.still !== undefined && reduced.matches ? "-still" : "";
        return `showcase/${img.dataset.scene}-${set}-${scheme()}${still}.png`;
    }

    function apply() {
        root.dataset.set = state.set;
        if (state.theme === "auto") {
            delete root.dataset.theme;
        } else {
            root.dataset.theme = state.theme;
        }
        for (const img of document.querySelectorAll("img[data-scene]")) {
            const next = source(img);
            if (img.getAttribute("src") === next) continue;
            img.classList.add("loading");
            img.addEventListener("load", () => img.classList.remove("loading"), {once: true});
            img.addEventListener("error", () => img.classList.remove("loading"), {once: true});
            img.setAttribute("src", next);
        }
        for (const button of document.querySelectorAll("[data-pick-set]")) {
            button.setAttribute("aria-pressed", String(button.dataset.pickSet === state.set));
        }
        for (const button of document.querySelectorAll("[data-pick-theme]")) {
            button.setAttribute("aria-pressed", String(button.dataset.pickTheme === state.theme));
        }
        for (const label of document.querySelectorAll("[data-look]")) {
            label.textContent = `${setNames[state.set]} skin set, ${themeNames[state.set][scheme()]} theme`;
        }
    }

    document.addEventListener("click", (event) => {
        const pickSet = event.target.closest("[data-pick-set]");
        const pickTheme = event.target.closest("[data-pick-theme]");
        if (pickSet) {
            state.set = pickSet.dataset.pickSet;
            store("psychic-ui.set", state.set);
            apply();
        } else if (pickTheme) {
            state.theme = pickTheme.dataset.pickTheme;
            store("psychic-ui.theme", state.theme);
            apply();
        }
    });
    dark.addEventListener("change", apply);
    reduced.addEventListener("change", apply);

    // A capture at full size, in a dialog
    const zoom = document.querySelector("dialog.zoom");
    const zoomed = zoom && zoom.querySelector("img");
    document.addEventListener("click", (event) => {
        const opener = event.target.closest("[data-zoom]");
        if (opener && zoom) {
            const img = opener.querySelector("img");
            zoomed.src = img.getAttribute("src");
            zoomed.alt = img.alt;
            zoom.showModal();
        } else if (zoom && event.target.closest("dialog.zoom")) {
            zoom.close();
        }
    });

    apply();
})();
