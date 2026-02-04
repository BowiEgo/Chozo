#pragma once

template <typename T> using TScope = std::unique_ptr<T>;

template <typename T, typename... Args>
constexpr TScope<T> CreateScope(Args &&...args) {
    return std::make_unique<T>(std::forward<Args>(args)...);
}