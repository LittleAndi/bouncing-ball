# Bouncing Ball Engine

Just playing around with what AI can do with C++ and 3D.

A real-time 3D simulation of colourful balls bouncing inside a box, complete with physics, lighting, and sound effects — all written in C++ with OpenGL.

![Bouncing balls in a 3D box with Phong shading]

## Features

- **3D physics** — gravity, elastic ball-wall collisions with energy damping, and ball-ball collisions resolved with impulse-based elastic response
- **UV-sphere rendering** — smooth 32×32 stack/sector spheres with per-vertex normals
- **Phong lighting** — ambient, diffuse, and specular shading via GLSL shaders (OpenGL 4.6 core)
- **Procedural sound** — synthesised collision sounds using damped sine waves played through [miniaudio](https://miniaud.io/); a higher-pitched tone for ball-ball collisions and a lower tone for wall hits, with volume scaled by impact speed
- **Configurable ball count** — pass the number of balls (1–50) as a command-line argument (default: 3)
- **Distinct colours** — balls are automatically assigned evenly-spaced hues in HSV space

## Dependencies

| Library | Purpose |
|---|---|
| [GLFW 3](https://www.glfw.org/) | Window creation & OpenGL context |
| [GLAD](https://glad.dav1d.de/) | OpenGL function loader |
| [GLM](https://github.com/g-truc/glm) | Math (vectors, matrices, transforms) |
| [miniaudio](https://miniaud.io/) | Audio playback |

All dependencies are managed via [vcpkg](https://vcpkg.io/).

## Building

### Prerequisites

- CMake ≥ 3.15
- A C++17 compiler (MSVC, GCC, or Clang)
- [vcpkg](https://vcpkg.io/) with the `VCPKG_ROOT` environment variable set (or integrated via `vcpkg integrate install`)

### Steps

```bash
# Configure (vcpkg manifest mode picks up vcpkg.json automatically)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

# Build
cmake --build build
```

The shaders are automatically copied next to the executable after each build.

## Running

```bash
# Default — 3 balls
./build/BouncingBallEngine

# Custom ball count (1–50)
./build/BouncingBallEngine 10
```

Press **Escape** to exit.

## Project Structure

```
.
├── CMakeLists.txt          # Build configuration
├── vcpkg.json              # vcpkg dependency manifest
├── src/
│   ├── main.cpp            # Entry point, render loop, physics update, audio
│   ├── ball.h              # Ball struct, wall-collision update, ball-ball collision resolution
│   └── shader.cpp          # GLSL shader loader/compiler
├── include/
│   └── shader.h            # Shader class declaration
└── shaders/
    ├── vertex.glsl         # Transforms positions & normals (MVP matrices)
    └── fragment.glsl       # Phong lighting model
```

## License

See [LICENSE](LICENSE).
