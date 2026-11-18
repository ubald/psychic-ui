// The bold variant of the site, served at bold/ next to the captures in ../showcase/ (see
// tools/showcase/publish.sh in the repository). Swaps every capture for the skin set and theme picked on the
// console, and the page's own theme with them; brings the picked set's card to the front of the deck; opens a
// capture at full size; reveals the gallery's tiles as they scroll in.
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
        return `../showcase/${img.dataset.scene}-${set}-${scheme()}${still}.png`;
    }

    const deck = document.querySelector(".deck");

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
            if (button.classList.contains("card")) continue;
            button.setAttribute("aria-pressed", String(button.dataset.pickSet === state.set));
        }
        for (const button of document.querySelectorAll("[data-pick-theme]")) {
            button.setAttribute("aria-pressed", String(button.dataset.pickTheme === state.theme));
        }
        for (const label of document.querySelectorAll("[data-look]")) {
            label.textContent = `${setNames[state.set]} skin set, ${themeNames[state.set][scheme()]} theme`;
        }
        if (deck) deck.dataset.front = state.set;
    }

    // A capture at full size, in a dialog
    const zoom = document.querySelector("dialog.zoom");
    const zoomed = zoom && zoom.querySelector("img");
    function open(img) {
        if (!zoom || !img) return;
        zoomed.src = img.getAttribute("src");
        zoomed.alt = img.alt || (deck && deck.getAttribute("aria-label")) || "";
        zoom.showModal();
    }

    document.addEventListener("click", (event) => {
        const card = event.target.closest(".card");
        const pickSet = event.target.closest("[data-pick-set]");
        const pickTheme = event.target.closest("[data-pick-theme]");
        const zoomer = event.target.closest("[data-zoom]");
        if (card && card.dataset.pickSet === state.set) {
            open(card.querySelector("img"));
        } else if (pickSet) {
            state.set = pickSet.dataset.pickSet;
            store("psychic-ui.set", state.set);
            apply();
        } else if (pickTheme) {
            state.theme = pickTheme.dataset.pickTheme;
            store("psychic-ui.theme", state.theme);
            apply();
        } else if (event.target.closest("[data-zoom-front]")) {
            open(deck && deck.querySelector(`.card[data-pick-set="${state.set}"] img`));
        } else if (zoomer) {
            open(zoomer.querySelector("img"));
        } else if (zoom && event.target.closest("dialog.zoom")) {
            zoom.close();
        }
    });
    dark.addEventListener("change", apply);
    reduced.addEventListener("change", apply);

    // The gallery's tiles rise into place once, as they scroll in; without the observer, or with reduced motion,
    // they're simply there
    if ("IntersectionObserver" in window && !reduced.matches) {
        const tiles = document.querySelectorAll(".tiles .tile, .reel, .density .tile");
        const seen = new IntersectionObserver((entries) => {
            for (const entry of entries) {
                if (!entry.isIntersecting) continue;
                entry.target.classList.add("shown");
                seen.unobserve(entry.target);
            }
        }, {rootMargin: "0px 0px -8% 0px"});
        for (const tile of tiles) {
            tile.classList.add("hidden");
            seen.observe(tile);
        }
    }

    apply();
})();
