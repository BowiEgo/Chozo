# Chozo Engine Test Framework User Guide

## 1. Overview

Chozo Engine includes a built-in test framework that supports unit testing, integration testing, and assertion validation. Test code is placed alongside module code for easy maintenance and debugging.

### 1.1 Test Framework Features

- **Modular Testing**: Tests for each module are placed in the `Tests/` directory
- **Auto Discovery**: The build system automatically discovers and compiles all test files
- **Rich Assertions**: Comprehensive assertion macros supporting value comparison and exception verification
- **Colored Output**: Test results are displayed in color for quick identification
- **CI Friendly**: Returns correct exit codes suitable for continuous integration

---

## 2. Quick Start

### 2.1 Build and Run Tests

```bash
# 1. Clean previous configuration
xmake f -c

# 2. Configure Debug mode with tests enabled
xmake f -m debug --tests=y

# 3. Build the project
xmake

# 4. Run tests
./build/tests/Test
```

### 2.2 Expected Output

```text
========================================
        Chozo Engine Test Suite
========================================

Registered test suites: 3
  - Vector3Tests
  - QuaternionTests
  - EntityTests

Running Vector3Tests...
  ✅ Default constructor (0.12 ms)
  ✅ Parameterized constructor (0.08 ms)
  ✅ Addition (0.09 ms)
  ✅ Dot product (0.06 ms)
  ✅ Cross product (0.07 ms)
  ✅ Length (0.05 ms)
  ✅ Normalized (0.08 ms)
  7 passed, 0 failed

Running QuaternionTests...
  ✅ Identity (0.05 ms)
  ✅ FromEuler (0.08 ms)
  2 passed, 0 failed

Running EntityTests...
  ✅ Default constructor (0.08 ms)
  ✅ Constructor with handle (0.06 ms)
  ✅ Equality operator (0.07 ms)
  ✅ Ostream output (0.06 ms)
  4 passed, 0 failed

========================================
Total: 13 passed, 0 failed
Time: 0.85 ms
========================================
✨ All tests passed!
```

## 3. Test File Organization

### 3.1 Directory Structure

### 3.2 Creating New Tests

```text
Source/
├── Runtime/
│   ├── Core/
│   │   ├── Public/
│   │   ├── Private/
│   │   └── Tests/                    # Core module tests
│   │       ├── EntityTests.cpp
│   │       └── UUIDTests.cpp
│   │
│   ├── RenderCore/
│   │   ├── Public/
│   │   ├── Private/
│   │   └── Tests/                    # RenderCore module tests
│   │       ├── Math/
│   │       │   ├── Vector3Tests.cpp
│   │       │   ├── Vector4Tests.cpp
│   │       │   ├── Matrix3Tests.cpp
│   │       │   ├── Matrix4Tests.cpp
│   │       │   └── QuaternionTests.cpp
│   │       └── Scene/
│   │           ├── EntityTests.cpp
│   │           └── SceneTests.cpp
│   │
│   └── Scene/
│       └── Tests/                    # Scene module tests
│           └── SceneTests.cpp
│
└── Test/
    ├── Public/
    │   ├── TestAssert.h              # Assertion macros
    │   ├── TestMacros.h              # Test suite macros
    │   └── TestFixture.h             # Test fixture
    ├── Main.cpp                      # Test runner
    └── xmake.lua                     # Test build configuration
```

Create a `*Tests.cpp` file in the module's `Tests/` directory:

```cpp
// Source/Runtime/MyModule/Tests/MyTests.cpp
#include "TestAssert.h"
#include "TestMacros.h"
#include "MyModule/MyClass.h"

using namespace Chozo;

TEST_SUITE(MyTests) {

    TEST("My first test") {
        MyClass obj;
        TEST_ASSERT(obj.IsValid());
    } TEST_END;

    TEST("My second test") {
        int value = 42;
        TEST_ASSERT_EQ(42, value);
    } TEST_END;
}
```

## 4. Assertion Macros

### 4.1 Basic Assertions

| Macro                              | Description                 | Example                                   |
| ---------------------------------- | --------------------------- | ----------------------------------------- |
| `TEST_ASSERT(cond)`                | Assert condition is true    | `TEST_ASSERT(x > 0)`                      |
| `TEST_ASSERT_EQ(expected, actual)` | Assert equality             | `TEST_ASSERT_EQ(5, x)`                    |
| `TEST_ASSERT_NE(expected, actual)` | Assert inequality           | `TEST_ASSERT_NE(0, x)`                    |
| `TEST_ASSERT_GT(a, b)`             | Assert a > b                | `TEST_ASSERT_GT(10, 5)`                   |
| `TEST_ASSERT_LT(a, b)`             | Assert a < b                | `TEST_ASSERT_LT(5, 10)`                   |
| `TEST_ASSERT_GE(a, b)`             | Assert a >= b               | `TEST_ASSERT_GE(5, 5)`                    |
| `TEST_ASSERT_LE(a, b)`             | Assert a <= b               | `TEST_ASSERT_LE(5, 5)`                    |
| `TEST_ASSERT_NEAR(a, b, epsilon)`  | Assert approximate equality | `TEST_ASSERT_NEAR(0.1, 0.100001, 0.0001)` |

### 4.2 Exception Testing

| Macro                                  | Description                                | Example                                              |
| -------------------------------------- | ------------------------------------------ | ---------------------------------------------------- |
| `TEST_EXPECT_FAIL(expr)`               | Expect expression to trigger assertion     | `TEST_EXPECT_FAIL(TestFunction())`                   |
| `TEST_EXPECT_FAIL_WITH_MSG(expr, msg)` | Expect assertion with message verification | `TEST_EXPECT_FAIL_WITH_MSG(TestFunction(), "error")` |

### 4.3 Usage Examples

```cpp
TEST("Basic assertions") {
    int a = 5, b = 10;

    TEST_ASSERT(a < b);
    TEST_ASSERT_EQ(5, a);
    TEST_ASSERT_NE(0, a);
    TEST_ASSERT_GT(b, a);
    TEST_ASSERT_LT(a, b);
    TEST_ASSERT_GE(b, 10);
    TEST_ASSERT_LE(a, 5);
} TEST_END;

TEST("Floating point comparison") {
    float value = 0.1f + 0.2f;
    TEST_ASSERT_NEAR(0.3f, value, 0.0001f);
} TEST_END;

TEST("Exception test") {
    FScene scene;
    FEntity invalid;

    // Expect assertion when adding component to invalid entity
    TEST_EXPECT_FAIL_WITH_MSG(
        (scene.AddComponent<TestComponent>(invalid)),
        "invalid entity"
    );
} TEST_END;
```

## 5. Test Suites

### 5.1 Basic Test Suite

```cpp
TEST_SUITE(MyTests) {

    TEST("Test case 1") {
        // Test code
    } TEST_END;

    TEST("Test case 2") {
        // Test code
    } TEST_END;
}
```

### 5.2 Test Fixture

```cpp
#include "TestFixture.h"

struct MyFixture {
    FScene scene;
    FEntity entity;

    MyFixture() {
        // Setup
        entity = scene.CreateEntity("Test");
    }

    ~MyFixture() {
        // Teardown
        scene.DestroyEntity(entity);
    }
};

TEST_FIXTURE(MyTests, MyFixture) {
    FIXTURE_SETUP({
        // Per-test setup
    });

    FIXTURE_TEARDOWN({
        // Per-test cleanup
    });

    TEST_F("Test with fixture", {
        TEST_ASSERT(f.entity.IsValid());
        TEST_ASSERT(f.scene.IsValid(f.entity));
    });

} TEST_F_END;
```

## 6. Build Configuration

### 6.1 Enable/Disable Tests

```bash
# Enable tests (Debug mode)
xmake f -m debug --tests=y
xmake

# Enable tests (Release mode)
xmake f -m release --tests=y
xmake

# Disable tests
xmake f --tests=n
xmake

# Clean and reconfigure
xmake f -c
```

### 6.2 Running Specific Tests

```bash
# Run all tests
./build/tests/Test

# Run specific test suite (if supported by your test runner)
./build/tests/Test --gtest_filter=EntityTests.*

# Verbose output
./build/tests/Test -v
```

## 7. Test Runner Features

### 7.1 Output Format

The test runner provides:

- Color-coded output: Green for PASS, Red for FAIL, Cyan for INFO
- Test duration: Each test shows execution time in milliseconds
- Suite summary: Pass/fail count and total time per test suite
- Failed test details: Full error messages with file and line numbers
- Final summary: Total tests, pass rate, and execution time

### 7.2 Exit Codes

| Exit Code | Meaning                  |
| --------- | ------------------------ |
| 0         | All tests passed         |
| 1         | One or more tests failed |

This makes the test runner suitable for CI/CD pipelines.

## 8. Best Practices

### 8.1 Test Organization

- One test per concept: Each TEST block should test a single behavior
- Descriptive names: Test names should clearly describe what is being tested
- Arrange-Act-Assert: Structure tests with setup, action, and verification phases

### 8.2 Assertion Guidelines

- Use specific assertions (TEST_ASSERT_EQ) over generic ones (TEST_ASSERT) when possible
- Use TEST_ASSERT_NEAR for floating point comparisons
- Use TEST_EXPECT_FAIL_WITH_MSG to verify error messages

### 8.3 Test Isolation

- Each test should be independent
- Use fixtures for common setup/teardown
- Avoid sharing state between tests

## 9. Troubleshooting

### 9.1 Common Issues

| Issue                                     | Solution                                                                     |
| ----------------------------------------- | ---------------------------------------------------------------------------- |
| Undefined symbols for architecture arm64  | Ensure CoreAssert.cpp is compiled and linked, or use static inline variables |
| option("tests") not found                 | Ensure option("tests") is defined in root xmake.lua                          |
| No tests registered                       | Check that test files are in Tests/ directory and --tests=y is set           |
| TEST_EXPECT_FAIL doesn't catch assertions | Ensure assertions throw exceptions (enable CZ_ENABLE_ASSERT_THROW())         |

### 9.2 Debugging Tests

```bash
# Build with debug symbols
xmake f -m debug --tests=y
xmake

# Run with debugger
lldb ./build/tests/Test

# Set breakpoint
(lldb) b EntityTests.cpp:42
(lldb) run
```

## 10. Reference

### 10.1 Assertion Macros Reference

| Macro                       | Parameters         | Returns                                 |
| --------------------------- | ------------------ | --------------------------------------- |
| `TEST_ASSERT`               | `cond`             | Throws on failure                       |
| `TEST_ASSERT_EQ`            | `expected, actual` | Throws on failure                       |
| `TEST_ASSERT_NE`            | `expected, actual` | Throws on failure                       |
| `TEST_ASSERT_GT`            | `a, b`             | Throws on failure                       |
| `TEST_ASSERT_LT`            | `a, b`             | Throws on failure                       |
| `TEST_ASSERT_GE`            | `a, b`             | Throws on failure                       |
| `TEST_ASSERT_LE`            | `a, b`             | Throws on failure                       |
| `TEST_ASSERT_NEAR`          | `a, b, epsilon`    | Throws on failure                       |
| `TEST_EXPECT_FAIL`          | `expr`             | Throws if no assertion                  |
| `TEST_EXPECT_FAIL_WITH_MSG` | `expr, msg`        | Throws if no assertion or wrong message |

### 10.2 Test Suite Macros Reference

| Macro                             | Description                 |
| --------------------------------- | --------------------------- |
| `TEST_SUITE(name)`                | Start a test suite          |
| `TEST(name)`                      | Start a test case           |
| `TEST_END`                        | End a test case             |
| `TEST_FIXTURE(name, FixtureType)` | Start a test fixture suite  |
| `FIXTURE_SETUP(code)`             | Setup code for each test    |
| `FIXTURE_TEARDOWN(code)`          | Teardown code for each test |
| `TEST_F(name, code)`              | Test case using fixture     |
