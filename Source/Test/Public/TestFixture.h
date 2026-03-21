#pragma once

#include <functional>
#include <vector>

template <typename T> class FTestFixture {
public:
    using SetupFunc = std::function<void(T&)>;
    using TeardownFunc = std::function<void(T&)>;

    void SetUp(SetupFunc func) { m_Setup = func; }
    void TearDown(TeardownFunc func) { m_Teardown = func; }

    template <typename TestFunc> void Run(const std::string& name, TestFunc test) {
        T instance;

        if (m_Setup) m_Setup(instance);

        try {
            test(instance);
        } catch (...) {
            if (m_Teardown) m_Teardown(instance);
            throw;
        }

        if (m_Teardown) m_Teardown(instance);
    }

private:
    SetupFunc m_Setup;
    TeardownFunc m_Teardown;
};

#define TEST_FIXTURE(SuiteName, FixtureType)                                                       \
    TEST_SUITE(SuiteName) {                                                                        \
        FTestFixture<FixtureType> fixture;                                                         \
        auto& s = suite;                                                                           \
        (void)s;

#define FIXTURE_SETUP(code) fixture.SetUp([&](FixtureType& f) { code; });
#define FIXTURE_TEARDOWN(code) fixture.Teardown([&](FixtureType& f) { code; });

#define TEST_F(name, code) fixture.Run(#name, [&](FixtureType& f) { code; });

#define TEST_F_END }