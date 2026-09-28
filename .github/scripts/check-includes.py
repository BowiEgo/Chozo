#!/usr/bin/env python3
"""Include audit: reports project files that use a standard facility without including the
header that provides it.

libc++ hands those facilities out through unrelated headers, libstdc++ and MSVC do not, so a
file that builds on macOS can fail on the other platforms (this happened for ~230 files when
CI was introduced). Run it before pushing:

    python3 .github/scripts/check-includes.py          # list findings
    python3 .github/scripts/check-includes.py --check  # exit 1 when something is missing
"""
import re, pathlib, sys

# facility pattern -> required header
RULES = [
    (r"\bstd::(shared_ptr|unique_ptr|make_unique|make_shared|weak_ptr|default_delete|allocator|enable_shared_from_this)\b", "<memory>"),
    (r"\bstd::(find|find_if|find_if_not|sort|stable_sort|remove|remove_if|transform|any_of|all_of|none_of|count|count_if|min_element|max_element|clamp|min|max|fill|copy|copy_if|lower_bound|upper_bound|unique|reverse|accumulate|for_each|replace|generate|shuffle)\b", "<algorithm>"),
    (r"\bstd::(move|forward|swap|pair|make_pair|exchange|declval)\b", "<utility>"),
    (r"\bstd::(function|hash|invoke|bind|reference_wrapper|less|greater)\b", "<functional>"),
    (r"\bstd::optional\b", "<optional>"),
    (r"\bstd::variant\b", "<variant>"),
    (r"\bstd::any\b|\bstd::any_cast\b", "<any>"),
    (r"\bstd::tuple\b|\bstd::get<", "<tuple>"),
    (r"\bstd::array\b", "<array>"),
    (r"\bstd::atomic", "<atomic>"),
    (r"\bstd::(mutex|lock_guard|unique_lock|scoped_lock|recursive_mutex|timed_mutex)\b", "<mutex>"),
    (r"\bstd::shared_mutex\b|\bstd::shared_lock\b", "<shared_mutex>"),
    (r"\bstd::condition_variable\b", "<condition_variable>"),
    (r"\bstd::thread\b|\bstd::this_thread\b", "<thread>"),
    (r"\bstd::chrono\b", "<chrono>"),
    (r"\bstd::filesystem\b", "<filesystem>"),
    (r"\bstd::(promise|future|async|packaged_task)\b", "<future>"),
    (r"\bstd::(string|to_string|stoi|stof)\b", "<string>"),
    (r"\bstd::string_view\b", "<string_view>"),
    (r"\bstd::vector\b", "<vector>"),
    (r"\bstd::unordered_map\b|\bstd::unordered_multimap\b", "<unordered_map>"),
    (r"\bstd::unordered_set\b|\bstd::unordered_multiset\b", "<unordered_set>"),
    (r"\bstd::map\b|\bstd::multimap\b", "<map>"),
    (r"\bstd::set\b|\bstd::multiset\b", "<set>"),
    (r"\bstd::deque\b", "<deque>"),
    (r"\bstd::list\b", "<list>"),
    (r"\bstd::(stringstream|ostringstream|istringstream)\b", "<sstream>"),
    (r"\bstd::(ifstream|ofstream|fstream)\b", "<fstream>"),
    (r"\bstd::(cout|cerr|clog|endl)\b", "<iostream>"),
    (r"\bstd::ostream\b|\bstd::basic_ostream\b", "<ostream>"),
    (r"\bstd::istream\b|\bstd::basic_istream\b", "<istream>"),
    (r"\bstd::?(memcpy|memset|strlen|strcmp|strcpy|strstr|memcmp)\b", "<cstring>"),
    (r"\bstd::?(printf|snprintf|sprintf|fprintf|FILE)\b", "<cstdio>"),
    (r"\bstd::(sqrt|abs|floor|ceil|sin|cos|tan|pow|fabs|round|atan2|isnan|isfinite)\b", "<cmath>"),
    (r"\bstd::numeric_limits\b", "<limits>"),
    (r"\bstd::(mt19937|mt19937_64|random_device|uniform_int_distribution|uniform_real_distribution)\b", "<random>"),
    (r"\bstd::(runtime_error|logic_error|exception|invalid_argument|out_of_range)\b", "<stdexcept>"),
    (r"\btypeid\b|\bstd::type_info\b", "<typeinfo>"),
    (r"\bstd::(is_same_v|enable_if|enable_if_t|decay_t|remove_reference_t|remove_cv_t|invoke_result_t|conditional_t|is_enum_v|is_integral_v|is_floating_point_v|is_trivially_copyable_v|is_convertible|is_base_of|underlying_type_t|is_arithmetic_v|is_pointer_v|is_constructible_v|add_pointer_t|remove_pointer_t|void_t|conjunction|disjunction)\b", "<type_traits>"),
    (r"\bstd::size_t\b|\bstd::ptrdiff_t\b", "<cstddef>"),
    (r"\bstd::?(uint8_t|uint16_t|uint32_t|uint64_t|int8_t|int16_t|int32_t|int64_t)\b", "<cstdint>"),
    (r"\bstd::initializer_list\b", "<initializer_list>"),
    (r"\bstd::(countr_zero|countl_zero|countr_one|countl_one|popcount|rotl|rotr|bit_cast|bit_ceil|bit_floor|bit_width|has_single_bit|endian|byteswap)\b", "<bit>"),
    (r"\bstd::span\b", "<span>"),
    (r"\bstd::(midpoint|lerp)\b", "<numeric>"),
    (r"\bstd::format\b|\bstd::format_to\b|\bstd::formatter\b", "<format>"),
    (r"\bstd::numbers\b", "<numbers>"),
    (r"\bstd::source_location\b", "<source_location>"),
    (r"\bstd::(jthread|stop_token|stop_source)\b", "<stop_token>"),
    (r"\bstd::ranges\b", "<ranges>"),
    (r"\bstd::(to_chars|from_chars)\b", "<charconv>"),
    (r"\bstd::(greater|less)\b", "<functional>"),
]

def analyse(paths):
    out = []
    for f in sorted(paths):
        text = f.read_text(errors="ignore")
        body = re.sub(r"//[^\n]*", "", text)
        needs = []
        for pattern, header in RULES:
            if re.search(pattern, body) and not re.search(r"#include\s*" + re.escape(header), text):
                needs.append(header)
        if needs:
            out.append((f, sorted(set(needs))))
    return out

if __name__ == "__main__":
    import sys

    roots = [pathlib.Path("Include"), pathlib.Path("Source")]
    files = [p for r in roots for p in r.rglob("*") if p.suffix in (".hpp", ".h", ".cpp")]
    findings = analyse(files)

    for f, needs in findings:
        print(f"{f}: missing {', '.join(needs)}")

    print(f"files needing includes: {len(findings)}")

    if "--check" in sys.argv and findings:
        sys.exit(1)
