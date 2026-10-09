# About

YVNEngine - is a cross-patform VN engine created using modern C:23 language.

## STACK

YVNE uses SDL3 for handling window and input events. Powered by OpenGL renderer.
Vulkan and Metal native support in future.

## Requirements

- CMake 3.25 or newer;
- Compiler which supports C23;
- Video driver which supports OpenGL 3.3 Core;
- Ninja, Make .

## Compile and run

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --parallel
./build/bin/vn.app/Contents/MacOs/vn
```

Release:

```sh
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release --parallel
```

## Tests

Configure once with `BUILD_TESTING=ON` (the default):

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
```

Choose what to build and run:

| Action | Command |
|---|---|
| Build the program only | `cmake --build build --target vn --parallel` |
| Build the program and run all tests | `cmake --build build --target check --parallel` |
| Build the program and run one test | `cmake --build build --target check_timer_unit --parallel` |

Every test has a `check_<test_name>` target. List the test names with
`ctest --test-dir build -N`; for example, use `check_input_unit`,
`check_assets_unit`, or `check_engine_smoke`. Each target builds the program
and its required test executable before running CTest. Build or test failures
return a nonzero exit code. Assertions stay enabled in Debug
and Release tests. Test fixtures live in `tests/fixtures` and are copied into
the build directory; no local `test_project` is required.

| Stage | Current checks |
|---|---|
| M0 | Dependency linking, logging, SDL/OpenGL smoke test |
| M1 | Timer pause/resume/finish, frame delta clamp, keyboard/mouse input, base path, manifest validation, VFS/streams, failed initialization cleanup, engine startup/quit/shutdown |
| M2 (implemented part) | PNG/JPEG decoding, RGBA pixels, premultiplied alpha, asset limits/placeholders, GL texture uploads, sprite rendering, viewport and DPI/input mapping |

Run already built tests, or select an individual stage:

```sh
ctest --test-dir build --output-on-failure
ctest --test-dir build -L M1 --output-on-failure
ctest --test-dir build -R '^timer_unit$' --output-on-failure
```

`engine_smoke` and `renderer_smoke` need a graphical session and OpenGL 3.3 Core.
They briefly create windows and check rendered pixels, rendering after resize,
and engine shutdown through an injected quit event.
Without a display/GPU, build normally and run the tests without the `graphics`
label:

```sh
cmake --build build --parallel
ctest --test-dir build -LE graphics --output-on-failure
```

## Compiler parameters

| Parameter | Default | Meaning |
|---|---:|---|
| `VN_VENDORED_DEPENDENCIES` | `ON` | Compile SDL3 from sources |
| `VN_BUILD_MACOS_BUNDLE` | `OFF` | Create `.app`-bundle on macOS |
| `VN_WARNINGS_AS_ERRORS` | `OFF` | Set warns as errors |
| `BUILD_TESTING` | `ON` | Enabe testing |
