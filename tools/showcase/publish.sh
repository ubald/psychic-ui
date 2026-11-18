#!/usr/bin/env sh
# Publish the website to the local gh-pages branch, which GitHub Pages serves from its root.
#
#     tools/showcase/publish.sh [preset]
#
# Captures the showcase (tools/showcase/capture.sh, with the CMake preset, debug by default; configured first if
# its build directory has none) into a temporary directory, lays the site out there (site/ at the root, the
# captures in showcase/, the bold variant site-bold/ in bold/, which reads the same captures from ../showcase/,
# Doxygen's docs/html/ in html/), and makes that tree the gh-pages branch as one commit
# without a parent, replacing whatever the branch held: recaptures never pile up in any history. Neither the
# working tree nor the index is touched. Nothing is pushed: the script prints the command that does it.
# Outside direnv, run it in the flake's shell: nix develop -c tools/showcase/publish.sh
set -eu

root=$(cd "$(dirname "$0")/../.." && pwd)
preset=${1:-debug}
branch=gh-pages

cd "$root"
if git worktree list --porcelain | grep -qx "branch refs/heads/$branch"; then
    echo "$branch is checked out in a worktree; remove that worktree first" >&2
    exit 1
fi

tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT INT TERM
site="$tmp/site"

if [ ! -f "build/$preset/CMakeCache.txt" ]; then
    cmake --preset "$preset"
fi
"$root/tools/showcase/capture.sh" "$preset" --out "$site/showcase"

cp -R site/. "$site/"
mkdir "$site/bold"
cp -R site-bold/. "$site/bold/"
cp -R docs/html "$site/html"

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
echo "Preview it:  dir=\$(mktemp -d) && git archive $branch | tar -x -C \"\$dir\" && python3 -m http.server -d \"\$dir\""
echo "Publish it:  git push --force origin $branch"
