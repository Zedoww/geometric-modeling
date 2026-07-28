<div align="center">

# Geometric Modeling

**A cross-platform C++ mesh viewer built around a half-edge data structure.**

[![CI](https://github.com/Zedoww/geometric-modeling/actions/workflows/ci.yml/badge.svg)](https://github.com/Zedoww/geometric-modeling/actions/workflows/ci.yml)
[![CodeQL](https://github.com/Zedoww/geometric-modeling/actions/workflows/codeql.yml/badge.svg)](https://github.com/Zedoww/geometric-modeling/actions/workflows/codeql.yml)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-00599C.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/license-MIT-22c55e.svg)](LICENSE)

![Catmull-Clark subdivision wireframe](docs/images/catmullclark-3-wireframe.png)

</div>

This project loads Wavefront OBJ meshes, represents their topology with
half-edges, and exposes common geometry-processing operations through an
OpenGL viewer. The implementation focuses on topology invariants, portable
builds, and visible before/after results.

## Highlights

- OBJ loading with automatic twin reconstruction and mesh normalization
- Per-face and per-vertex normal computation
- View-dependent silhouette detection
- Convex and concave polygon triangulation using ear clipping
- Surface-of-revolution mesh generation
- Shortest-edge-collapse simplification
- Catmull-Clark subdivision with boundary handling
- Automated half-edge invariant tests

## Results

| Operation | Before | After |
| --- | --- | --- |
| Concave triangulation | ![Concave polygon](docs/images/triangulation-concave-before.png) | ![Triangulated polygon](docs/images/triangulation-concave-after.png) |
| Edge-collapse simplification | ![Original gear](docs/images/simplify-before.png) | ![Simplified gear](docs/images/simplify-after.png) |
| Catmull-Clark subdivision | ![Original cube](docs/images/catmullclark-0.png) | ![Subdivided cube](docs/images/catmullclark-3.png) |

Additional captures are available in [`docs/images`](docs/images).

## Architecture

```text
TP1/MeshViewerCMake/
|-- myMesh.*           Half-edge ownership and geometry algorithms
|-- myHalfedge.*       Directed-edge topology
|-- myFace.*           Face adjacency and normal computation
|-- myVertex.*         Vertex geometry and incident-edge access
|-- main.cpp           OpenGL/GLUT viewer and interaction
`-- test_mesh.cpp      Topology and algorithm regression tests
```

`mesh_core` contains the geometry implementation without a window-system
dependency. The viewer adds OpenGL, GLEW, GLM, and GLUT on top of that core.

## Build

Requirements:

- CMake 3.20 or newer
- A C++17 compiler
- OpenGL, GLEW, GLM, and GLUT/FreeGLUT

Install dependencies:

```bash
# Ubuntu / Debian
sudo apt-get install cmake g++ libgl1-mesa-dev libglew-dev libglm-dev freeglut3-dev

# macOS
brew install cmake glew glm

# Windows with vcpkg
vcpkg install glew:x64-windows glm:x64-windows freeglut:x64-windows
```

Configure, build, and test:

```bash
cmake -S TP1/MeshViewerCMake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

On Windows, also pass the vcpkg toolchain file during configuration:

```powershell
cmake -S TP1/MeshViewerCMake -B build `
  -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows
```

Launch `MeshViewer` from the build directory. Right-click inside the window to
open the operation menu.

To work only on the geometry core and tests without installing graphics
dependencies, disable the viewer:

```bash
cmake -S TP1/MeshViewerCMake -B build -DGEOMETRIC_MODELING_BUILD_VIEWER=OFF
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

## Test coverage

The regression executable validates loading, twin reciprocity, face-loop
consistency, triangulation, simplification, subdivision, and open-boundary
behavior. CI builds the project and runs the suite on Linux, macOS, and Windows.

## Project context

This project was developed as an academic geometric-modeling assignment. AI
tools were used for targeted portability troubleshooting and as a learning aid;
the implementation choices, integration, and validation remain the author's.

## License

[MIT](LICENSE) - copyright Allan Seddi.
