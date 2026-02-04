# Logging System Documentation

## 1. Overview

The **Chozo Engine Logging System** is a modular, high-performance logging framework built upon **spdlog**. It adopts a design pattern similar to Unreal Engine, utilizing **Log Categories** and **Verbosity Levels** to provide structured, controllable output across multiple DLL modules.

---

## 2. Verbosity Levels

Verbosity levels define the severity of the log message and control whether it should be displayed or acted upon.

| Level       | Macro Constant           | Behavior Description                                                                                     |
| :---------- | :----------------------- | :------------------------------------------------------------------------------------------------------- |
| **Fatal**   | `ELogVerbosity::Fatal`   | Critical failure. Logs the message and **triggers an immediate abort()**. Used for unrecoverable states. |
| **Error**   | `ELogVerbosity::Error`   | Runtime error. Indicates a functional failure, but the engine attempts to continue.                      |
| **Warning** | `ELogVerbosity::Warning` | Potential risk. Indicates unusual states that may lead to issues.                                        |
| **Info**    | `ELogVerbosity::Info`    | Standard output. Used for major lifecycle events (e.g., system startup).                                 |
| **Trace**   | `ELogVerbosity::Trace`   | Detailed tracing. Used for fine-grained debugging information.                                           |

---

## 3. How to Use

### 3.1 Defining a Log Category

Log categories allow you to filter logs by engine module (e.g., Renderer, Physics, Core).

#### Scenario A: Shared Category (Across Multiple Files)

If you want a category to be accessible throughout a module or the entire engine:

1. **Declare** it in a header file (`.h`):

    ```cpp
    // MyModule.h
    #include "Logging/LogMacros.h"

    DECLARE_LOG_CATEGORY_EXTERN(LogMyModule, Info);
    ```

2. **Define** it in a source file (.cpp):

    ```cpp
    // MyModule.cpp
    #include "MyModule.h"

    DEFINE_LOG_CATEGORY(LogMyModule);
    ```

#### Scenario B: Private Category (Single File Only)

If you only need a category within a specific .cpp file and don't want to expose it:

```cpp
// MyModule.cpp
#include "Logging/LogMacros.h"

// Defines type and instance in one step
DEFINE_LOG_CATEGORY_STATIC(LogPrivateInternal, Trace);
```

### 3.2 Printing Logs

Use the CZ_LOG macro to output messages.

```cpp
void FMyClass::Initialize() {
    // Syntax: CZ_LOG(Category, Verbosity, Message)
    CZ_LOG(LogMyModule, Info, "Subsystem initialized successfully.");

    if (LoadFailure) {
        CZ_LOG(LogMyModule, Error, "Failed to load resource data.");
    }
}
```
