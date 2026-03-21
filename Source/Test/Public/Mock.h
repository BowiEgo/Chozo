#pragma once

#include <functional>
#include <string>
#include <unordered_map>

template <typename Signature> class FMock;

template <typename R, typename... Args> class FMock<R(Args...)> {
public:
    using FuncType = std::function<R(Args...)>;

    void SetDefault(FuncType func) { m_Default = func; }

    void ExpectCall(const std::string& name, FuncType func) { m_Expectations[name] = func; }

    R Invoke(Args... args) {
        std::string callSig = GetCallSignature(args...);

        auto it = m_Expectations.find(callSig);
        if (it != m_Expectations.end()) {
            return it->second(args...);
        }

        if (m_Default) {
            return m_Default(args...);
        }

        throw std::runtime_error("Unexpected call: " + callSig);
    }

private:
    std::string GetCallSignature(Args... args) {
        std::stringstream ss;
        ((ss << typeid(Args).name() << ":" << args << " "), ...);
        return ss.str();
    }

    FuncType m_Default;
    std::unordered_map<std::string, FuncType> m_Expectations;
};

#define MOCK_METHOD(mock, return_type, name, args)                                                 \
    mock.ExpectCall(#name, [&] args -> return_type

#define MOCK_EXPECT(mock, name)                                                                    \
    mock.ExpectCall(#name, [&]