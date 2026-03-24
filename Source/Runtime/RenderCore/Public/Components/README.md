# TMeshComponent - Typed Mesh Component Documentation

## Overview

TMeshComponent is a template-based mesh component that provides type-safe storage and management of mesh parameters. It supports compile-time type checking, automatic mesh data generation, caching, and integration with the plugin system.

## Key Features

- Type Safety: Compile-time type checking for mesh parameters
- Automatic Caching: Mesh data is generated only when needed and cached
- Property Access: Generic property getters/setters with compile-time validation
- Plugin Support: Works with both built-in and plugin mesh types
- Hash Support: Built-in hash calculation for caching and comparison

## Basic Usage

### 1. Create Mesh Components

```cpp
// Cube component
auto cubeComp = TMeshComponent<FCubeParams>();
cubeComp.Set<MeshProp::Width>(2.0f);
cubeComp.Set<MeshProp::Height>(3.0f);
cubeComp.Set<MeshProp::WidthSegments>(4);

// Sphere component
auto sphereComp = TMeshComponent<FSphereParams>(1.0f, 32, 16);
sphereComp.Set<MeshProp::Radius>(2.0f);
sphereComp.Set<MeshProp::Segments>(64);
```

### 2. Get Mesh Data

```cpp
// Access mesh data (generated on demand)
const auto& meshData = sphereComp.GetData();
for (const auto& vertex : meshData.Vertices) {
    // Process vertices
}

// Create procedural mesh for rendering
auto procMesh = sphereComp.CreateProceduralMesh();
procMesh->Upload(context);
procMesh->Draw(cmdList);
```

## Constructor Options

```cpp
// Default constructor (uses default parameter values)
TMeshComponent<FCubeParams> cube;

// Constructor with parameters
TMeshComponent<FCubeParams> cube(2.0f, 3.0f, 1.0f, 4, 4, 4);

// Constructor with params struct
FCubeParams params;
params.Width = 2.0f;
params.Height = 3.0f;
TMeshComponent<FCubeParams> cube(params);
```

## Property Access

### Using Generic Setters/Getters

```cpp
// Set property
cube.Set<MeshProp::Width>(2.0f);
cube.Set<MeshProp::HeightSegments>(8);

// Get property
auto width = cube.Get<MeshProp::Width>();
if (width) {
    std::cout << "Width: " << *width << std::endl;
}
```

### Using Convenience Methods (Built-in Types)

For built-in mesh types, you can also use convenience methods (if defined):

```cpp
// Cube
cube.SetWidth(2.0f);
cube.SetHeight(3.0f);
cube.SetWidthSegments(4);

// Sphere
sphere.SetRadius(1.5f);
sphere.SetSegments(64);
```

## Type Helpers

```cpp
// Get mesh type at compile time
constexpr EMeshType type = TMeshComponent<FCubeParams>::GetStaticType();

// Get mesh type at runtime
EMeshType runtimeType = cube.GetType();

// Check if component is valid
if (cube.IsValid()) {
    // Use component
}
```

### State Management

```cpp
// Check if mesh data needs regeneration
if (cube.IsDirty()) {
// Data is dirty, will regenerate on next GetData()
}

// Manually mark as dirty (forces regeneration)
cube.MarkDirty();

// Clear dirty flag
cube.ClearDirty();
```

### Comparison and Hashing

```cpp
// Compare two components
auto cube1 = TMeshComponent<FCubeParams>(2.0f, 2.0f, 2.0f);
auto cube2 = TMeshComponent<FCubeParams>(2.0f, 2.0f, 2.0f);
if (cube1 == cube2) {
// Same parameters
}

// Get hash for caching
size_t hash = cube.GetHash();
Converting to Variant
cpp
// Convert to variant for uniform storage
FMeshParams variant = cube;
std::vector<FMeshParams> meshList;
meshList.push_back(cube);
meshList.push_back(sphere);
```

## Performance Considerations

### Caching Behavior

- Mesh data is generated lazily on first GetData() call
- Generated data is cached until MarkDirty() is called
- Parameter changes automatically trigger dirty flag

### Memory Usage

- Component stores only parameters (lightweight)
- Mesh data is stored separately in FMeshBuffer and cached
- Multiple components with identical parameters share cached data via hash comparison

### Example: Complete Workflow

```cpp
#include "Mesh/TMeshComponent.h"

void ExampleUsage() {
// Create components
auto cube = TMeshComponent<FCubeParams>(2.0f, 2.0f, 2.0f);
auto sphere = TMeshComponent<FSphereParams>(1.0f, 32, 16);

    // Modify parameters
    cube.Set<MeshProp::Width>(3.0f);
    sphere.Set<MeshProp::Radius>(1.5f);
    sphere.Set<MeshProp::Segments>(64);

    // Get mesh data (automatically regenerated)
    const auto& cubeData = cube.GetData();
    const auto& sphereData = sphere.GetData();

    // Create rendering meshes
    auto cubeMesh = cube.CreateProceduralMesh();
    auto sphereMesh = sphere.CreateProceduralMesh();

    // Upload to GPU
    cubeMesh->Upload(context);
    sphereMesh->Upload(context);

    // Render
    cubeMesh->Draw(cmdList);
    sphereMesh->Draw(cmdList);

    // Store in uniform container
    std::vector<FMeshParams> meshes;
    meshes.push_back(cube);
    meshes.push_back(sphere);

}
```

## Supported Mesh Types

| Type     | Parameter Struct | Properties                                                                          |
| :------- | :--------------- | :---------------------------------------------------------------------------------- |
| Cube     | FCubeParams      | Width, Height, Depth, WidthSegments, HeightSegments, DepthSegments                  |
| Sphere   | FSphereParams    | Radius, WidthSegments, HeightSegments, PhiStart, PhiLength, ThetaStart, ThetaLength |
| Cylinder | FCylinderParams  | Radius, Height, RadialSegments, HeightSegments                                      |
| Plane    | FPlaneParams     | Width, Height, WidthSegments, HeightSegments                                        |
| Custom   | User-defined     | Registered via plugin system                                                        |

## Error Handling

- Compile-time errors: Using properties not supported by the mesh type
- Runtime checks: Invalid parameter values are handled by the mesh generator
- Null checks: GetData() returns empty data if mesh generation fails

## Thread Safety

- TMeshComponent is not thread-safe by default
- Use external synchronization when accessing from multiple threads
- Consider using const methods for read-only access

## Integration with Editor

```cpp
// In editor UI
void DrawMeshComponentUI(TMeshComponent<FSphereParams>& sphere) {
    float radius = sphere.Get<MeshProp::Radius>().value_or(0.5f);
    if (ImGui::DragFloat("Radius", &radius, 0.01f)) {
        sphere.Set<MeshProp::Radius>(radius);
    }

    uint32_t segments = sphere.Get<MeshProp::Segments>().value_or(32);
    if (ImGui::DragInt("Segments", (int*)&segments, 1, 3, 256)) {
        sphere.Set<MeshProp::Segments>(segments);
    }

    // Show preview
    ImGui::Image(sphere.GetData());
}
```

## Best Practices

1.  Use Type Aliases for readability:

```cpp
using FCubeComponent = TMeshComponent<FCubeParams>;
using FSphereComponent = TMeshComponent<FSphereParams>;
```

2.  Cache Frequently Used Components to avoid regeneration:

```cpp
static auto defaultCube = FCubeComponent(1.0f, 1.0f, 1.0f);
```

3.  Batch Parameter Updates before accessing data:

```cpp
auto sphere = FSphereComponent();
sphere.Set<MeshProp::Radius>(2.0f);
sphere.Set<MeshProp::Segments>(128);
const auto& data = sphere.GetData(); // Only generates once
```

4. Use Hash for Caching to share mesh data:

```cpp
std::unordered_map<size_t, FMeshBuffer> meshCache;
meshCache[sphere.GetHash()] = sphere.GetData();
```
