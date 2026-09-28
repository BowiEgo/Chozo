#!/usr/bin/env bash
# Checks clang-format compliance for the files touched by a change (or the whole tree locally).
#
# Usage:
#   check-format.sh              # all tracked C/C++ sources
#   check-format.sh <base-ref>   # only files that differ from <base-ref>
#
# Requires the clang-format version pinned by CI (see .github/workflows/ci.yml) to avoid
# spurious diffs caused by formatter version skew.
set -euo pipefail

base="${1:-}"

if [ -n "$base" ] && git rev-parse --verify --quiet "$base" >/dev/null; then
    files=$(git diff --name-only --diff-filter=ACMR "$base"...HEAD -- '*.cpp' '*.hpp' '*.h' || true)
    echo "checking files changed against $base"
else
    files=$(git ls-files '*.cpp' '*.hpp' '*.h')
    echo "checking every tracked C/C++ file"
fi

files=$(printf '%s\n' "$files" | grep -v '^$' | grep -v '^External/' | grep -v '^build/' || true)

if [ -z "$files" ]; then
    echo "nothing to check"
    exit 0
fi

failures=0
while IFS= read -r file; do
    [ -f "$file" ] || continue
    if ! clang-format --dry-run --Werror "$file" >/dev/null 2>&1; then
        echo "::error file=$file::$file is not clang-format clean"
        failures=$((failures + 1))
    fi
done <<< "$files"

echo "checked $(printf '%s\n' "$files" | wc -l | tr -d ' ') file(s), $failures not formatted"
[ "$failures" -eq 0 ]
