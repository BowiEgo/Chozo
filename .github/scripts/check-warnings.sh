#!/usr/bin/env bash
# Fails when a build log contains compiler warnings from the project's own sources.
#
# Only compiler diagnostics count (`<source>:<line>:<col>: warning: ...`); toolchain notices
# such as ranlib/ld messages are ignored. Third-party code (FetchContent under build/_deps,
# vendored External/) is ignored as well: the project itself is expected to build
# warning-free, and this budget keeps it that way.
#
# Usage: check-warnings.sh <build.log> [budget]
set -euo pipefail

log="${1:?usage: check-warnings.sh <build.log> [budget]}"
budget="${2:-0}"

[ -f "$log" ] || { echo "::error::build log '$log' not found"; exit 1; }

count=0
while IFS= read -r line; do
    case "$line" in
        *_deps*|*External/*) continue ;;
    esac

    if [[ "$line" =~ ^[^[:space:]]+\.(c|cc|cpp|cxx|h|hh|hpp|hxx|m|mm):([0-9]+:)?([0-9]+:)?[[:space:]]warning: ]]; then
        echo "::error::$line"
        count=$((count + 1))
    fi
done < "$log"

echo "project warnings: $count (budget: $budget)"
[ "$count" -le "$budget" ]
