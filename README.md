# Boon Engine

`Boon Engine` is a cross-platform C++ game engine designed for rapid iteration and multiplayer development. It provides:

- An entity-component-system (ECS) foundation (using `entt`).
- Reflection and code-generation support (via the `BClassGenerator` tool).
- 2D physics integration (`box2d`) and a renderer with tilemap support.
- A networking layer with replication and RPC support (GameNetworkingSockets integration).
- An editor executable (`Editor`) and a sample game module (`Sandbox`).
- Asset importers, scene serialization and runtime modules.

See the `Boon` folder for the engine core and `Editor`, `Sandbox`, `Runtime` modules for example apps.



---

## Documentation

- API reference (Doxygen): `docs/doxygen/html/index.html`

## Quick start

Prerequisites

- CMake >= 3.20
- Ninja (or another generator; Ninja is used by the CMake configs)
- A C++20-capable compiler (MSVC, Clang, GCC)
- On Windows: Visual Studio or the MSVC toolchain (the project was developed with MSVC)

Clone the repository:

```
git clone https://github.com/MichelleTobback/Boon-Engine.git
cd Boon-Engine
```

Configure and build (recommended with Ninja):

```
mkdir build
cmake -S . -B build -G "Ninja" -DCMAKE_BUILD_TYPE=Release
cmake --build build --target Editor --config Release
```

Notes

- The top-level `CMakeLists.txt` registers `Boon/tools/BClassGenerator` and generates reflection sources during the build. The `Editor` target depends on the `GenerateReflection` custom target and the generated sources are placed in the module `generated/` folders.
- Build output is placed under `bin/<Config>/<Module>` (for example `bin/Release/Editor`).
- The `Editor` post-build steps copy `Boon/Assets`, `Sandbox/Assets` and `Editor/Resources` into the runtime `Assets` directory.

Running the Editor

After a successful build launch the editor executable from the build output:

```
# On Windows
build\\bin\\Release\\Editor\\Editor.exe
```


<img width="1919" height="1028" alt="Screenshot_2025-11-20_120101" src="https://github.com/user-attachments/assets/a28bdf32-fc32-4ee7-b71c-7eafd4395980" />


<img width="1618" height="779" alt="Screenshot_2025-11-20_120335" src="https://github.com/user-attachments/assets/0292dbd2-dcb3-4edd-9a3d-ef6e21cb40e5" />

---

