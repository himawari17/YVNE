# About

YVNEngine - is a cross-patform VN engine created using modern C:23 language.

## STACK

YVNE uses SDL3 for handling window and input events. Powered by OpenGL renderer.
Vulkan and Metal native support in future.

Images are decoded with vendored stb_image through the VFS. PNG and JPEG are
covered by tests; WebP is not supported. Images are limited to 4096 pixels per
dimension and stored as RGBA8 with alpha premultiplied in sRGB space.

## Requirements

- CMake 3.25 or newer;
- Compiler which supports C23;
- Video driver which supports OpenGL 3.3 Core;
- Ninja, Make .


## Compile and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/vn
```

Release:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

## Compiler parameters

| Parameter | Default | Meaning |
|---|---:|---|
| `VN_VENDORED_DEPENDENCIES` | `ON` | Compile SDL3 from sources |
| `VN_BUILD_MACOS_BUNDLE` | `OFF` | Create `.app`-bundle on macOS |
| `VN_WARNINGS_AS_ERRORS` | `OFF` | Set warns as errors |
| `BUILD_TESTING` | `ON` | Enabe testing |
