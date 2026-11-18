#!/usr/bin/env sh
# Capture the website's screenshots from the showcase program's scenes.
#
#     tools/showcase/capture.sh [preset] [--out <directory>] [--scene <name>]...
#
# Builds psychic-ui-showcase with the CMake preset (debug by default; it must already be configured) and runs it
# into the directory (build/<preset>/showcase/captures by default). Without --scene, the old captures there go
# first, so that scenes renamed or removed leave nothing behind. Everything renders offscreen on the CPU: no
# window, no GL, no display. The captures are never committed: tools/showcase/publish.sh puts them on the
# gh-pages branch. Outside direnv, run it in the flake's shell: nix develop -c tools/showcase/capture.sh
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
preset=debug
if [ $# -gt 0 ] && [ "${1#--}" = "$1" ]; then
    preset=$1
    shift
fi
out=
if [ $# -gt 1 ] && [ "$1" = "--out" ]; then
    out=$2
    shift 2
fi
[ -n "$out" ] || out="$root/build/$preset/showcase/captures"

cd "$root"
cmake --build --preset "$preset" --target psychic-ui-showcase -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

mkdir -p "$out"
if [ $# -eq 0 ]; then
    rm -f "$out"/*.png
fi
"$root/build/$preset/showcase/psychic-ui-showcase" --out "$out" "$@"
