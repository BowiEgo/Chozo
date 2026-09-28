# Chozo

一个自研的 C++20 游戏引擎与编辑器：自建内存系统、句柄式 RHI（Vulkan 后端）、Slang 着色器编译、基于 EnTT 的场景、ImGui 编辑器，以及 Tracy 性能分析集成。

**平台状态**：macOS 已实机验证（MoltenVK）；Windows 为目标平台之一，构建已按平台分支、CI 有 `windows-core` job，但完整构建尚未验证；Linux 为占位（仅核心模块可构建，Vulkan surface 未实现）。详见 `docs/P2-PLAN.md` 的 G7。

> 状态：**开发中（WIP）**。分支 `dev-0.1.x` 已清掉评审列出的全部 P0 缺陷与部分 P1 项（见 `docs/TODO.md`），编辑器可运行、可渲染、退出时无残留分配；渲染功能仍在演进（当前只有单个测试管线 + 程序化网格）。

## 环境要求

| 用途 | 要求 |
|---|---|
| 通用 | CMake ≥ 3.28、支持 C++20 的编译器（Clang）、Git |
| 完整构建（含渲染器与编辑器） | Vulkan SDK（macOS 可用 Homebrew 的 `molten-vk` + `vulkan-loader` + `vulkan-headers`；Windows 用 LunarG SDK）、macOS 需 Xcode Command Line Tools |
| 仅核心构建（CI/快速迭代） | 无需 Vulkan/SDL/Slang —— 只编译 13 个 Core 模块与测试 |

首次配置会通过 `FetchContent` 拉取依赖（fmt、spdlog、glm、doctest、Tracy；完整构建还会拉取 SDL3、EnTT、VMA、ImGui、Slang 预编译包）。网络受限时可加 `-DCHOZO_GITHUB_MIRROR=https://ghfast.top/https://github.com` 走镜像。

## 构建与运行

```bash
# 仅核心（秒级，无需 Vulkan）
cmake --preset core-debug
cmake --build --preset core-debug
ctest --preset core-debug

# 完整引擎（需要 Vulkan SDK）
cmake --preset full-release
cmake --build --preset full-release

# 运行编辑器（产物在 .app bundle 中）
./build/dist/Release/Chozo.app/Contents/MacOS/Launch
```

可用的 preset：`core-{debug,release,asan}`、`full-{debug,release}`（`cmake --list-presets`）。本地与 CI 使用同一组命令。

若使用 LunarG SDK，确保 `VULKAN_SDK` 指向 SDK 根目录；使用 Homebrew 时 `VULKAN_SDK=/opt/homebrew`。CMake 从 `find_package(Vulkan)` 找到的 loader 推导其余路径，不假设特定的 SDK 目录布局。

## 测试与质量门禁

```bash
ctest --preset core-asan                    # ASan + UBSan 跑全部核心测试
.github/scripts/check-format.sh             # clang-format 22.1.5（版本需与 CI 一致）
.github/scripts/check-warnings.sh build.log 4   # 项目源码告警预算（第三方 _deps 不计）
```

CI（`.github/workflows/ci.yml`）包含四个 job：`format`、`core`（Debug/Release 矩阵）、`full-macos`（完整构建 + 编辑器）、`sanitizers`。

## 目录结构

```
Include/           公共头文件（Core/ 与 Runtime/ 两层，供引擎与插件使用）
Source/
  Core/            13 个基础模块：Memory、Log、Event、JobSystem、TypeRegistry、Math、Platform…
  Runtime/         RHI（抽象层）、RenderCore（渲染/场景/资产）、Window、App（引擎与主循环）
  Backend/Vulkan/  Vulkan 后端实现（以 dylib 形式加载）
  Editor/          编辑器（ImGui 面板、节点树、同步桥；编译为 dylib 插件）
  Launch/          可执行入口与 .app bundle 装配
  Test/            doctest 测试可执行文件（收集各模块的 Tests/*Test.cpp）
CMake/             模块化构建辅助（add_chozo_module、依赖获取、Vulkan 定位）
External/          vendored 第三方代码（当前 stb 未接入构建）
Resources/Shaders/ Slang 着色器
docs/              设计文档与计划（见下）
```

## 文档索引

| 文档 | 内容 |
|---|---|
| `docs/ownership.md` | **必读**：句柄/所有权约定（`Handle` 视图 vs `Scope` 所有者）与新资源检查清单 |
| `docs/TODO.md` | P0/P1 缺陷清单：已完成项与待处理项（含文件:行号与验收标准） |
| `docs/P2-PLAN.md` | P2 工程化方案：CI、测试、死代码、文档、构建、分层、跨平台 |
| `docs/ROADMAP.md` | 分阶段路线图（引擎重构与后续功能规划） |

## 已知限制

- 仅 macOS 被实机验证；Windows 的构建/平台层已就位但未验证，Linux 为占位（详见 `docs/P2-PLAN.md` 的 G7）。
- 渲染器当前只有一条测试管线与被当作样例的程序化网格，材质/光照尚未接入。
- 编辑器面板为骨架（场景层级、属性、控制台可用；内容浏览器/材质面板尚未实现）。
- 校验层仍有 4 条已知告警（`Basic.slang` 顶点属性声明与消费不一致），见 `docs/TODO.md` P1-10 的遗留项。
