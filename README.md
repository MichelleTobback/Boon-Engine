# Boon Engine

[![Build: GitHub Actions](https://img.shields.io/github/actions/workflow/status/MichelleTobback/Boon-Engine/ci.yml?branch=main&label=build&logo=github)](https://github.com/MichelleTobback/Boon-Engine/actions)
[![License](https://img.shields.io/github/license/MichelleTobback/Boon-Engine.svg)](LICENSE)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-20-brightgreen.svg)](https://isocpp.org/std/the-standard)
[![Docs (Doxygen)](https://img.shields.io/badge/docs-Doxygen-blue.svg)](https://michelletobback.github.io/Boon-Engine/)

Boon Engine is a C++ game engine focused on multiplayer. It provides a modular core, editor tooling, networking primitives and runtime modules aimed at 2D multiplayer games.

Key goals:
- Fast iteration cycle (editor + hot-reloadable modules)
- Pragmatic networking primitives for replication & RPCs
- Small, composable building blocks (ECS + reflection)

---

## Features
- Entity-Component-System foundation (entt)
- Reflection & code generation (`BClassGenerator`)
- Networking layer with replication and RPC support (GameNetworkingSockets integration)
- 2D physics via Box2D
- Renderer with tilemap and sprite systems
- Asset importers, scene serialization and runtime asset management
- Editor executable (`Editor`) and sample module (`Sandbox`)
- Modular CMake build with generated sources support

---

## Getting started

Prerequisites
- CMake >= 3.20
- Ninja (recommended) or another CMake generator
- C++20-capable compiler (MSVC, Clang, GCC)
- On Windows: Visual Studio / MSVC toolchain

Quick clone & build
```bash
git clone https://github.com/MichelleTobback/Boon-Engine.git
cd Boon-Engine
mkdir build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Editor --config Release
```

Notes
- The top-level `CMakeLists.txt` registers `Boon/tools/BClassGenerator`. Reflection sources are generated during the build; the `Editor` target depends on the `GenerateReflection` custom target. Generated code is placed in each module's `generated/` folder.
- Build output is placed under `bin/<Config>/<Module>` (for example `bin/Release/Editor`).
- The `Editor` post-build steps copy `Boon/Assets`, `Sandbox/Assets` and `Editor/Resources` into the runtime `Assets` directory.

Run built binaries
- Editor (Windows example):
```bash
build/bin/Release/Editor/Editor.exe
```
- Sandbox (sample runtime) — run the `Sandbox` binary from `bin/<Config>/Sandbox` after building that target.

---

## Documentation
- [![Doxygen docs](https://img.shields.io/badge/docs-Doxygen-blue.svg)](https://michelletobback.github.io/Boon-Engine/)
- Module design notes and contributing docs: `docs/`

---

## Examples
Small snippets that demonstrate the usage of the engine

Create and load a `Scene`
```cpp
SceneManager& sceneManager = ServiceLocator::Get<SceneManager>();
Scene& scene = sceneManager.CreateScene("Sample");

// load from file
SceneSerializer serializer(scene);
serializer.Deserialize("Assets/scenes/Sample.scene");
```

In Editor/Initialization - spawn a replicated `GameObject` with components
```cpp
// spawn GameObject
GameObject player = scene.Instantiate();
GameObject player2 = scene.Instantiate({30.f, 15.f});

// add replication components 
player.AddComponent<NetIdentity>();
player.AddComponent<NetTransform>();
player.AddComponent<NetRigidbody2D>();

//optional
player.GetOrAddComponent<TagComponent>().Tag = "Player";
```

During Runtime - spawn a replicated `GameObject`
```cpp
uint64_t connectionId = 0u;
NetIdentity& netId = player.GetComponent<NetIdentity>();
netId.pScene->InstantiateGameObject(connectionId);
```

Import and load `assets`
```cpp
// import the asset
AssetLibrary& assetLib = Assets::Get();
AssetRef<SpriteAtlasAsset> atlas = assetLib.Import<SpriteAtlasAsset>("game/Witch/Witch-combined.bsa");

// get the asset handle from the asset
SpriteRendererComponent& sprite = player.AddComponent<SpriteRendererComponent>();
sprite.SpriteAtlasHandle = atlas->GetHandle();
sprite.Sprite = 0;

// get the instance of the asset resource
SpriteAnimatorComponent& animator = player.AddComponent<SpriteAnimatorComponent>();
animator.Clip = 0;
animator.Atlas = atlas->GetInstance();
animator.pRenderer = player;
```

---

## Project structure
- `Boon/` — Engine core: headers (`include/`), sources (`src/`), subsystems and core modules
- `Editor/` — Editor executable, UI panels, editor-only tools and resources
- `Sandbox/` — Example game module demonstrating engine systems
- `Runtime/` — Runtime launcher glue and lightweight runtime targets
- `tools/` — Build-time utilities (e.g., `BClassGenerator`)
- `docs/` — Design notes, guides and contributing information
- `generated/` — Generated reflection/source files (created at build time)
- `external/` — Third-party libraries (Box2D, entt, etc.)
- `CMakeLists.txt` — Top-level build and configuration

Refer to module docs in `docs/modules/` for per-module details.

---

## Contributing
- Contributions welcome. See `docs/CONTRIBUTING.md` for guidelines on code style, PR workflow and testing.
- Keep changes small, document design decisions and run the project's linters/formatters where applicable.

If `docs/CONTRIBUTING.md` is missing, open an issue to request contribution guidelines before submitting non-trivial changes.

---

## License
See the `LICENSE` file in the repository root for licensing terms.

---

## Troubleshooting & tips
- If generated reflection sources are missing, run a full configure & build — the generator runs as part of the build.
- Use Ninja for faster incremental builds (`-G "Ninja"`).
- On Windows, prefer building from a Developer Command Prompt or the Visual Studio toolchain matching your VS install.

---

Screenshots

