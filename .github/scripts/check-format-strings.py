#!/usr/bin/env python3
"""Rejects malformed fmt/native format strings in log and formatting calls.

A stray brace in a format string is not a style issue but a crash: fmt throws
format_error, which aborts the process. One such literal ("{}}") stayed hidden in
EditorLayer::OnKeyPressed for months because the handler was never called, and it
crashed the editor on the first keystroke once event dispatch was wired up.

Only literals passed to known formatting entry points are checked, so JSON, shader
source and other embedded text in ordinary string literals cannot cause false
positives.
"""
from __future__ import annotations

import pathlib
import re
import sys

CALLS = r"(?:CZ_\w*LOG|fmt::format|std::format|spdlog::\w+|printf|fprintf)"
PATTERN = re.compile(CALLS + r"\s*\(\s*(?:[A-Za-z_][\w:]*\s*,\s*)?\"((?:[^"\\]|\\.)*)\"")
SOURCE_SUFFIXES = {".cpp", ".hpp", ".h", ".cc"}


def brace_error(literal: str) -> str | None:
    """Returns a description when the literal has unbalanced {} placeholders."""
    depth = 0
    i = 0
    while i < len(literal):
        ch = literal[i]
        if ch in "{}" and i + 1 < len(literal) and literal[i + 1] == ch:
            i += 2  # escaped brace pair
            continue
        if ch == "{":
            depth += 1
        elif ch == "}":
            depth -= 1
            if depth < 0:
                return "unmatched '}'"
        i += 1
    return "unmatched '{'" if depth else None


def main() -> int:
    root = pathlib.Path("Source")
    if not root.is_dir():
        print("check-format-strings: run from the repository root", file=sys.stderr)
        return 2

    failures: list[str] = []
    checked = 0
    for path in sorted(root.rglob("*")):
        if path.suffix not in SOURCE_SUFFIXES:
            continue
        for number, line in enumerate(path.read_text(encoding="utf-8", errors="replace").splitlines(), 1):
            for match in PATTERN.finditer(line):
                literal = match.group(1)
                if "{" not in literal and "}" not in literal:
                    continue
                checked += 1
                problem = brace_error(literal)
                if problem:
                    failures.append(f"{path}:{number}: {problem} in \"{literal}\"")
                    print(f"::error file={path},line={number}::{problem} in \"{literal}\"")

    if failures:
        print(f"check-format-strings: {len(failures)} malformed format string(s)", file=sys.stderr)
        return 1

    print(f"check-format-strings: {checked} format string(s) checked, 0 malformed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
