#!/usr/bin/env sh
# Publish the website to the local gh-pages branch, which GitHub Pages serves from its root.
#
#     tools/showcase/publish.sh [preset]
#
# Captures the showcase (tools/showcase/capture.sh, with the CMake preset, debug by default; configured first if
# its build directory has none) into a temporary directory, lays the site out there (site/ at the root, the
# captures in showcase/, the API reference in html/, generated fresh by Doxygen from the doc target's Doxyfile
# with its output there instead of the checkout's docs/), and makes that tree the gh-pages branch as one commit without a parent,
# replacing whatever the branch held: recaptures never pile up in any history. Neither the working tree nor the
# index is touched. Nothing is pushed: the script prints the command that does it. It needs Doxygen, which the
# flake's shell has; outside direnv, run it there: nix develop -c tools/showcase/publish.sh
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
preset=${1:-debug}
branch=gh-pages

cd "$root"
if ! command -v doxygen >/dev/null 2>&1; then
    echo "Doxygen not found: publishing generates the API reference with it (the flake's shell has it)" >&2
    exit 1
fi
if git worktree list --porcelain | grep -qx "branch refs/heads/$branch"; then
    echo "$branch is checked out in a worktree; remove that worktree first" >&2
    exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM
site="$tmp/site"

# CMake writes the doc target's Doxyfile only once it finds Doxygen, so a build directory without one is configured again
if [ ! -f "build/$preset/CMakeCache.txt" ] || [ ! -f "build/$preset/Doxyfile" ]; then
    cmake --preset "$preset"
fi
if [ ! -f "build/$preset/Doxyfile" ]; then
    echo "CMake found no Doxygen, so build/$preset has no Doxyfile (see DOXYGEN_EXECUTABLE in its cache)" >&2
    exit 1
fi
"$root/tools/showcase/capture.sh" "$preset" --out "$site/showcase"

cp -R site/. "$site/"

# The doc target's Doxyfile with its output in the site (its html/ is the site's html/), read from the standard input
{
    cat "build/$preset/Doxyfile"
    echo "OUTPUT_DIRECTORY = $site"
    echo "QUIET = YES"
    echo "WARN_LOGFILE = $tmp/doxygen.log"
} | doxygen -
warnings=$(grep -c ': warning: ' "$tmp/doxygen.log" || true)

# The tree goes through an index of its own, so the repository's index and working tree stay as they are
GIT_INDEX_FILE="$tmp/index" git --work-tree="$site" add --all --force .
tree=$(GIT_INDEX_FILE="$tmp/index" git write-tree)

source=$(git rev-parse --short HEAD)
if [ -n "$(git status --porcelain --untracked-files=no)" ]; then
    source="$source with uncommitted changes"
fi
commit=$(git commit-tree "$tree" -m "The website, from $source")
git update-ref "refs/heads/$branch" "$commit"

size=$(git ls-tree -r -l "$commit" | awk '{ total += $4 } END { printf "%.1f MB", total / 1048576 }')
count=$(git ls-tree -r "$commit" -- showcase | wc -l | tr -d ' ')
echo
echo "$branch is now $(git rev-parse --short "$commit"): $count captures, $size in all."
echo "The API reference: $warnings Doxygen warnings (make doc prints them)."
echo "Preview it:  dir=\$(mktemp -d) && git archive $branch | tar -x -C \"\$dir\" && python3 -m http.server -d \"\$dir\""
echo "Publish it:  git push --force origin $branch"
