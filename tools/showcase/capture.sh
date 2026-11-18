#!/usr/bin/env sh
# Regenerate the website's screenshots (docs/showcase/) from the showcase program's scenes.
#
#     tools/showcase/capture.sh [preset] [--scene <name>]...
#
# Builds psychic-ui-showcase with the CMake preset (debug by default; it must already be configured) and runs it
# into docs/showcase/. Without --scene, the old captures go first, so that scenes renamed or removed leave nothing
# behind. Everything renders offscreen on the CPU: no window, no GL, no display. Outside direnv, run it in the
# flake's shell: nix develop -c tools/showcase/capture.sh
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
preset=debug
if [ $# -gt 0 ] && [ "${1#--}" = "$1" ]; then
    preset=$1
    shift
fi
out="$root/docs/showcase"

cd "$root"
cmake --build --preset "$preset" --target psychic-ui-showcase -j"$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 4)"

if [ $# -eq 0 ]; then
    rm -f "$out"/*.png
fi
mkdir -p "$out"
"$root/build/$preset/showcase/psychic-ui-showcase" --out "$out" "$@"
