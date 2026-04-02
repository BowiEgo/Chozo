# VFS Module Documentation

## 1. Overview

The `VFS` module serves as the **Virtual File System (VFS)** core for the Chozo Engine. Its primary responsibility is to translate abstract **Virtual Paths** (used internally by the engine) into **Physical Paths** (recognized by the Operating System).

By utilizing a protocol-based mechanism, developers can decouple asset loading logic from physical storage locations, ensuring better project portability.

---

## 2. Core Concepts

### 2.1 Virtual Path

A string representing a logical location, prefixed with a protocol.

- **Example**: `engine://Shaders/BasePBR.glsl`
- **Protocol**: `engine://`
- **Relative Path**: `Shaders/BasePBR.glsl`

### 2.2 Physical Path

A full, OS-compliant path on the actual storage device.

- **Example**: `D:/Codes/Chozo/Resources/Shaders/BasePBR.glsl`

---

## 3. Key Features

### 3.1 Protocol Registration

Protocols are mapped to physical directories during the engine's initialization phase via `Mount`.

| Protocol      | Intended Use                                          | Typical Mapping                |
| :------------ | :---------------------------------------------------- | :----------------------------- |
| `engine://`   | Built-in resources (Internal shaders, default meshes) | `/Resources` (relative to EXE) |
| `shaders://`  | Project-specific GLSL/HLSL files                      | `/Assets/Shaders`              |
| `textures://` | Project-specific texture assets                       | `/Assets/Textures`             |

### 3.2 Path Resolution

The `Resolve` function implements the following logic:

1.  **Protocol Matching**: Checks if the path starts with a registered key (e.g., `engine://`).
2.  **Sanitization**: Automatically handles redundant slashes (`/` or `\`) to prevent path corruption.
3.  **Concatenation**: Uses `std::filesystem` for cross-platform safe path joining.
4.  **Validation**: Verifies if the physical file exists and logs a warning if it is missing.

---

## 4. Code Examples

### 4.1 Initializing the System

```cpp
// Within Application.cpp or Application initialization
auto root = std::filesystem::current_path();

// Map engine-internal resources
VFS::Mount("engine", root / "Internal/Resources");
VFS::Mount("shaders", root / "MyProject/Assets/Shaders");
```

### 4.2 Resolve the Virtual Path

```cpp
std::filesystem::path sourcePath = VFS::Resolve("shaders://Player.glsl");
```
