#pragma once

#include <utility>

namespace CZ {

template <typename T, typename E> class Result {
public:
    static Result Success(T value) {
        Result r;
        r.m_success = true;
        new (&r.m_value) T(std::move(value));
        return r;
    }

    static Result Error(E error) {
        Result r;
        r.m_success = false;
        new (&r.m_error) E(std::move(error));
        return r;
    }

    ~Result() {
        if (m_success)
            m_value.~T();
        else
            m_error.~E();
    }

    bool has_value() const { return m_success; }
    explicit operator bool() const { return m_success; }

    T& value() { return m_value; }
    const T& value() const { return m_value; }
    E& error() { return m_error; }
    const E& error() const { return m_error; }

    Result(Result&& other) noexcept : m_success(other.m_success) {
        if (m_success)
            new (&m_value) T(std::move(other.m_value));
        else
            new (&m_error) E(std::move(other.m_error));
    }

private:
    Result() = default;
    union {
        T m_value;
        E m_error;
    };
    bool m_success;
};

} // namespace CZ