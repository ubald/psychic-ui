#!/usr/bin/env python3
"""
Generates the C++ tables of the bundled icon sets (psychic-ui/icons/*Icons.cpp).

    tools/icons/generate.py pivotal <path to Pivotal's packages/ui/src/icons/icons.tsx>
    tools/icons/generate.py lucide [--package <extracted lucide-static package>]

The Lucide subset is the names listed in tools/icons/lucide.txt: add a name there and run the script again.
Without --package, it downloads the pinned lucide-static release from npm and checks its integrity hash.

Every SVG element becomes path data (circles, rects and lines included), so the library only parses
path data (SkParsePath::FromSVGString). Run it again after changing the source; the output is
deterministic, so an unchanged source gives an unchanged file.
"""

import argparse
import base64
import hashlib
import io
import json
import re
import sys
import tarfile
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = ROOT / "psychic-ui" / "icons"


# region SVG elements to path data

def num(value):
    """A number as short as SVG path data allows"""
    value = round(float(value), 4)
    if value == int(value):
        return str(int(value))
    text = repr(value)
    if text.startswith("0."):
        text = text[1:]
    elif text.startswith("-0."):
        text = "-" + text[2:]
    return text


def circle(cx, cy, r):
    return ellipse(cx, cy, r, r)


def ellipse(cx, cy, rx, ry):
    cx, cy, rx, ry = map(float, (cx, cy, rx, ry))
    return (f"M{num(cx - rx)} {num(cy)}"
            f"a{num(rx)} {num(ry)} 0 1 0 {num(2 * rx)} 0"
            f"a{num(rx)} {num(ry)} 0 1 0 {num(-2 * rx)} 0Z")


def rect(x, y, width, height, rx=None, ry=None):
    x, y, w, h = map(float, (x, y, width, height))
    # SVG: a missing radius takes the other one
    if rx is None and ry is None:
        rx = ry = 0.0
    elif rx is None:
        rx = ry
    elif ry is None:
        ry = rx
    rx = min(float(rx), w / 2)
    ry = min(float(ry), h / 2)
    if rx == 0 or ry == 0:
        return f"M{num(x)} {num(y)}h{num(w)}v{num(h)}h{num(-w)}Z"
    return (f"M{num(x + rx)} {num(y)}h{num(w - 2 * rx)}"
            f"a{num(rx)} {num(ry)} 0 0 1 {num(rx)} {num(ry)}v{num(h - 2 * ry)}"
            f"a{num(rx)} {num(ry)} 0 0 1 {num(-rx)} {num(ry)}h{num(2 * rx - w)}"
            f"a{num(rx)} {num(ry)} 0 0 1 {num(-rx)} {num(-ry)}v{num(2 * ry - h)}"
            f"a{num(rx)} {num(ry)} 0 0 1 {num(rx)} {num(-ry)}Z")


def line(x1, y1, x2, y2):
    return f"M{num(x1)} {num(y1)}L{num(x2)} {num(y2)}"


def points(text, close):
    values = [num(v) for v in re.split(r"[\s,]+", text.strip()) if v]
    pairs = [f"{values[i]} {values[i + 1]}" for i in range(0, len(values) - 1, 2)]
    return "M" + "L".join(pairs) + ("Z" if close else "")


def to_path(tag, attrs):
    """The path data of an SVG element"""
    a = attrs.get
    if tag == "path":
        return " ".join(a("d").split())
    if tag == "circle":
        return circle(a("cx", 0), a("cy", 0), a("r"))
    if tag == "ellipse":
        return ellipse(a("cx", 0), a("cy", 0), a("rx"), a("ry"))
    if tag == "rect":
        return rect(a("x", 0), a("y", 0), a("width"), a("height"), a("rx"), a("ry"))
    if tag == "line":
        return line(a("x1", 0), a("y1", 0), a("x2", 0), a("y2", 0))
    if tag == "polyline":
        return points(a("points"), False)
    if tag == "polygon":
        return points(a("points"), True)
    raise ValueError(f"unsupported SVG element <{tag}>")

# endregion


# region Pivotal

PIVOTAL_GRID = 16
# Pivotal's --icon-stroke (packages/ui/src/styles/tokens.css), in grid units
PIVOTAL_STROKE = 1.3

ELEMENT = re.compile(r"<(path|circle|rect|ellipse|line|polyline|polygon)\b(.*?)/>", re.S)
ATTRIBUTE = re.compile(r'([\w-]+)=(?:"([^"]*)"|\{\s*(\'[^\']*\'|"[^"]*"|[^{}]*)\s*\})', re.S)
FUNCTION = re.compile(r"^export function (\w+)Icon\(", re.M)


def kebab(name):
    return re.sub(r"(?<=[a-z0-9])(?=[A-Z])", "-", name).lower()


def pivotal(source):
    """[(name, [(paint, d)])] from Pivotal's icons.tsx, in its order"""
    text = Path(source).read_text()
    starts = list(FUNCTION.finditer(text))
    icons = []
    for i, match in enumerate(starts):
        body = text[match.end():starts[i + 1].start() if i + 1 < len(starts) else len(text)]
        elements = []
        variant = []  # what the filled variant adds
        for element in ELEMENT.finditer(body):
            tag, rest = element.group(1), element.group(2)
            attrs = {m.group(1): (m.group(2) if m.group(2) is not None else m.group(3)) for m in ATTRIBUTE.finditer(rest)}
            d = to_path(tag, attrs)
            stroked = "{...iconStroke}" in rest
            fill = attrs.get("fill")
            if stroked:
                elements.append(("Stroke", d))
            if fill and "currentColor" in fill:
                if fill.strip().strip("'\"") == "currentColor":
                    elements.append(("Fill", d))
                else:
                    # A conditional fill (`filled ? 'currentColor' : 'none'`): a "-filled" variant
                    variant.append(("Fill", d))
            if not stroked and not fill:
                raise ValueError(f"{match.group(1)}Icon: an element neither stroked nor filled")
        if not elements:
            continue  # Icon itself, the frame
        name = kebab(match.group(1))
        icons.append((name, elements))
        if variant:
            icons.append((name + "-filled", elements + variant))
    return icons

# endregion


# region Lucide

LUCIDE_VERSION = "1.64.0"
LUCIDE_URL = f"https://registry.npmjs.org/lucide-static/-/lucide-static-{LUCIDE_VERSION}.tgz"
# The npm registry's `dist.integrity` of that release
LUCIDE_INTEGRITY = "sha512-UxWmMteOXbKHI5E4f2qVtxCAAsI9gsInYK1lNw0nrOA0w4FkJeOVZ+UOV6aGA2377FZe3qyiFYq3lqjBKZ6OuA=="
LUCIDE_GRID = 24
LUCIDE_STROKE = 2
LUCIDE_LIST = Path(__file__).resolve().parent / "lucide.txt"


def lucide_package(directory):
    """(icon-nodes.json, LICENSE) of the package: from an extracted one, or the pinned release, checked"""
    if directory:
        package = Path(directory)
        return json.loads((package / "icon-nodes.json").read_text()), (package / "LICENSE").read_text()
    with urllib.request.urlopen(LUCIDE_URL) as response:
        data = response.read()
    integrity = "sha512-" + base64.b64encode(hashlib.sha512(data).digest()).decode()
    if integrity != LUCIDE_INTEGRITY:
        raise SystemExit(f"{LUCIDE_URL}: integrity {integrity}, expected {LUCIDE_INTEGRITY}")
    # Only read the two files needed, never extracted to disk
    with tarfile.open(fileobj=io.BytesIO(data), mode="r:gz") as archive:
        nodes = archive.extractfile("package/icon-nodes.json").read().decode()
        license = archive.extractfile("package/LICENSE").read().decode()
    return json.loads(nodes), license


def lucide_names():
    names = []
    for raw in LUCIDE_LIST.read_text().splitlines():
        name = raw.split("#", 1)[0].strip()
        if name:
            names.append(name)
    return names


def lucide(directory):
    """[(name, [(paint, d)])] of the listed icons, and the package's license"""
    nodes, license = lucide_package(directory)
    pivotal_names = set(re.findall(r'^\s*icon\("([^"]+)", ', (OUT_DIR / "PivotalIcons.cpp").read_text(), re.M))
    icons = []
    for name in lucide_names():
        if name not in nodes:
            raise SystemExit(f"lucide-static {LUCIDE_VERSION} has no icon \"{name}\"")
        if name in pivotal_names:
            raise SystemExit(f"\"{name}\" is in Pivotal's set already: remove it from {LUCIDE_LIST.name}")
        # Lucide only strokes (fill="none" on the svg)
        icons.append((name, [("Stroke", to_path(tag, attrs)) for tag, attrs in nodes[name]]))
    return icons, license

# endregion


# region C++ output

def cpp_string(text):
    return '"' + text.replace("\\", "\\\\").replace('"', '\\"') + '"'


def identifier(name):
    return "icon_" + re.sub(r"\W", "_", name)


def write(path, symbol, header, icons, grid, stroke):
    lines = [
        "// Generated by tools/icons/generate.py: don't edit, run it again.",
    ]
    lines += ["// " + h if h else "//" for h in header]
    lines += [
        "",
        "#include <array>",
        "#include <span>",
        "#include <string_view>",
        '#include "psychic-ui/icons/IconSet.hpp"',
        "",
        "namespace psychic_ui::icons {",
        "",
        "    namespace {",
    ]
    # Only the paints the icons use, so the table compiles without an unused function
    used = {paint for _, elements in icons for paint, _ in elements}
    for paint in ("Stroke", "Fill"):
        if paint in used:
            lines.append(f"        constexpr IconElement {paint.lower()}(std::string_view path) {{ return {{.paint = IconPaint::{paint}, .d = path}}; }}")
    lines += [
        "",
        f"        constexpr float grid        = {float(grid)!r}f;",
        f"        constexpr float strokeWidth = {float(stroke)!r}f;",
        "",
        "        constexpr IconDefinition icon(std::string_view name, std::span<const IconElement> elements) {",
        "            return {.name = name, .grid = grid, .stroke = strokeWidth, .elements = elements};",
        "        }",
        "",
    ]
    for name, elements in icons:
        lines.append(f"        constexpr std::array {identifier(name)}{{")
        for paint, d in elements:
            lines.append(f"            {paint.lower()}({cpp_string(d)}),")
        lines.append("        };")
    lines += [
        "",
        "        constexpr std::array definitions{",
    ]
    for name, _ in icons:
        lines.append(f"            icon({cpp_string(name)}, {identifier(name)}),")
    lines += [
        "        };",
        "    }",
        "",
        f"    constinit const std::span<const IconDefinition> {symbol}{{definitions}};",
        "}",
        "",
    ]
    path.write_text("\n".join(lines))
    print(f"{path.relative_to(ROOT)}: {len(icons)} icons")

# endregion


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sets = parser.add_subparsers(dest="set", required=True)
    p = sets.add_parser("pivotal", help="Pivotal's icons, from its icons.tsx")
    p.add_argument("source", help="packages/ui/src/icons/icons.tsx in the Pivotal repository")
    l = sets.add_parser("lucide", help="the Lucide icons listed in lucide.txt")
    l.add_argument("--package", help="an extracted lucide-static package, instead of downloading the pinned release")
    args = parser.parse_args()

    if args.set == "pivotal":
        icons = pivotal(args.source)
        write(OUT_DIR / "PivotalIcons.cpp", "pivotalIcons", [
            "Pivotal's icon set (packages/ui/src/icons/icons.tsx), by the same author as Psychic UI.",
            "A 16-unit grid, stroked 1.3 units wide with round caps and joins (Pivotal's --icon-stroke).",
            "Names are Pivotal's component names in kebab case, without \"Icon\" (ChevronDownIcon is \"chevron-down\");",
            "StarIcon's filled form is \"star-filled\".",
        ], icons, PIVOTAL_GRID, PIVOTAL_STROKE)

    if args.set == "lucide":
        icons, license = lucide(args.package)
        (OUT_DIR / "LICENSE-Lucide.txt").write_text(license)
        write(OUT_DIR / "LucideIcons.cpp", "lucideIcons", [
            f"Lucide icons (https://lucide.dev), lucide-static {LUCIDE_VERSION}: the ones listed in tools/icons/lucide.txt,",
            "for what Pivotal's set lacks, under Lucide's names. A 24-unit grid, stroked 2 units wide with round caps and",
            "joins; drawn with the same weight as Pivotal's (see IconDefinition).",
            "",
            "ISC License, Copyright (c) 2026 Lucide Icons and Contributors; some icons derive from Feather",
            "(MIT License, Copyright (c) 2013-present Cole Bemis). Full text: LICENSE-Lucide.txt, next to this file.",
            "",
            "Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is",
            "hereby granted, provided that the above copyright notice and this permission notice appear in all copies.",
        ], icons, LUCIDE_GRID, LUCIDE_STROKE)


if __name__ == "__main__":
    sys.exit(main())

