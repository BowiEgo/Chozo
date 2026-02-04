| Prefix  | Full Name             | Target Objects                                                | Example                             |
| :------ | :-------------------- | :------------------------------------------------------------ | :---------------------------------- |
| **`C`** | **Class / Component** | Objects with a lifecycle, inherited from engine base classes. | `CRenderer`, `CScene`, `CCamera`    |
| **`F`** | **Foundation / Fact** | Plain C++ structs or helper classes (usually "value" types).  | `FVector3`, `FColor`, `FTransform`  |
| **`U`** | **Utility / UI**      | UI widgets, textures, or auxiliary resource types.            | `UButton`, `UTexture2D`, `UWidget`  |
| **`I`** | **Interface**         | Pure virtual classes defining a contract.                     | `IRenderPass`, `IShaderResource`    |
| **`E`** | **Enum**              | Enumeration types.                                            | `ELogVerbosity`, `EBufferUsage`     |
| **`T`** | **Template**          | Template-based utility classes or containers.                 | `TArray`, `TSingleton`, `TCallback` |
