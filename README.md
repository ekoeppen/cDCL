# Apple Newton Desktop Connectivity Library (cDCL)

![C/C++ CI](https://github.com/ekoeppen/cDCL/workflows/C/C++%20CI/badge.svg)

The Desktop Connectivity Library (cDCL) is a modern C++ multi-platform library
designed to connect to Apple Newton devices via the Newton Dock protocol,
manipulate NewtonOS data structures (`NSRef`, frames, symbols, packages), and
provide core streaming and serialization utilities.

The repository consists of two main libraries:

- **`DCL`**: Domain-specific implementation of the [Newton Dock
  protocol](Documentation/Dante-Connection-Protocol.md), [MNP over serial
  encoding](Documentation/MNP.md), package encoding/decoding, and object stores.
- **`K`**: Foundational systems library providing binary streams, ring buffers,
  memory-mapped files, and endianness utilities.

## Architecture & Design

### Modern C++20 Standard

The codebase targets **C++20** (`-std=c++20`) using standard primitives
(`std::thread`, `std::mutex`, `std::condition_variable`, `<atomic>`, and
`<chrono>`), implementing a [multithreaded](Documentation/Threading.md)
connectivity framework.

### Streamlined Platform Strategy

Builds rely on standard C++20 (`<bit>` `std::endian`) and built-in compiler
macros (`_WIN32`, `__APPLE__`, `__linux__`, etc.) rather than custom
`TARGET_OS_*` or `TARGET_RT_*` preprocessor definitions:

- **POSIX targets** (macOS, iOS, Linux, Android, BSD via Clang / GCC)
- **Windows** (native Win32 via MinGW-w64; `K` also supports MSVC)

Compared to the original [DCL](https://sourceforge.net/projects/newton-dcl/),
legacy platforms (Cygwin, Classic Mac OS 8/9, BeOS) and custom platform/boolean
macros (`TARGET_OS_*`, `TARGET_RT_*`, `Boolean`) have been retired in favor of
standard C++20 types (`bool`, `<cstdint>`) and compiler-provided macros.

### Compatibility Shims

To permit downstream consumers (such as Einstein) and legacy tools to migrate
smoothly without high churn:

- Legacy fixed-width aliases (`KUInt32`, `KSInt16`, etc.) are provided in
  `K/Defines/KDefinitions.h` as lightweight `using` aliases to `<cstdint>` types
  (`uint32_t`, `int16_t`).
- Concurrency facades (`TThread`, `TMutex`, `TCondVar`) remain available as thin
  wrappers encapsulating standard C++20 concurrency.

## CMake Integration (Downstream Consumers)

cDCL exports namespaced modern CMake targets: `cDCL::K` and `cDCL::DCL`
(`cDCL::DCL` automatically links and pulls `cDCL::K`).

### Via CMake `FetchContent` (Recommended)

In your project's `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    cDCL
    GIT_REPOSITORY https://github.com/ekoeppen/cDCL.git
    GIT_TAG        master  # or a specific tag/hash
)
FetchContent_MakeAvailable(cDCL)

# Link against K alone (for systems primitives) or DCL (for Newton dock/objects):
target_link_libraries(my_app PRIVATE cDCL::DCL)
```

### Via `add_subdirectory` / Git Submodule

```cmake
add_subdirectory(externals/cDCL)
target_link_libraries(my_app PRIVATE cDCL::K cDCL::DCL)
```

### Subproject Behavior (`FetchContent` / `add_subdirectory`)

When `cDCL` is embedded as a subproject (`PROJECT_IS_TOP_LEVEL` is `FALSE`),
tools, tests, install rules, RPATH settings, and the standalone
`BUILD_SHARED_LIBS=ON` default are disabled automatically so `cDCL` follows the
parent project's build configuration. Explicit override options are available:

- `CDCL_BUILD_TOOLS` (defaults to `ON` standalone, `OFF` when embedded)
- `CDCL_BUILD_TESTS` (defaults to `ON` standalone, `OFF` when embedded)
- `CDCL_INSTALL` (defaults to `ON` standalone, `OFF` when embedded)

### Header Include Conventions

Downstream consumers use namespaced includes relative to the repository root:

```cpp
#include <K/Misc/TCircleBuffer.h>
#include <K/Defines/UByteSex.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>
```

## Compilation

Build with CMake (requires CMake 3.21+ and a C++20 compiler):

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

MinGW requires usage of a toolchain file when cross-compiling from macOS or
Linux:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=cmake/mingw-toolchain.cmake
cmake --build build
```

Binaries can be statically linked by setting the `BUILD_SHARED_LIBS` CMake
option to `OFF`:

```sh
cmake -B build -DBUILD_SHARED_LIBS=OFF
cmake --build build
```

## Testing

Tests use [doctest](https://github.com/doctest/doctest) (`KTests` for `K`,
`DCLTests` for `DCL`, and `NSReferencesDeathTest` in `Debug` POSIX builds) and
are executed using `ctest` from the build directory (works in both `Debug` and
`Release`):

```sh
ctest --test-dir build -C Debug --output-on-failure
```

The `doxygen` test is marked as optional and can be excluded if Doxygen is not
installed:

```sh
ctest --test-dir build -C Debug -LE optional
```

Individual `doctest` runners can also be invoked directly with suite or case
filters:

```sh
./build/Tests/KTests
./build/Tests/DCLTests --test-suite=MNPPipe
```

Interactive hardware/serial tests are built separately in `DCLManualTests` and
not run by `ctest`:

```sh
./build/Tests/DCLManualTests cl-cad-bsd
./build/Tests/DCLManualTests cl-cad-fdserial /dev/ttyUSB0
```

Pass `-DCDCL_BUILD_TESTS=OFF` (or `-DBUILD_TESTING=OFF`) to CMake to skip
building the test suite.

## Releases

Prebuilt standalone CLI tool archives (`linux-x86_64`, `macos-universal`, and
`windows-x86_64`) along with `SHA256SUMS.txt` are published on [GitHub
Releases](https://github.com/ekoeppen/cDCL/releases).

To stage a standalone CLI tools tree locally:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_SHARED_LIBS=OFF
cmake --build build
cmake --install build --component tools --prefix dist --strip
```

### Cutting a Release

[`.github/workflows/release.yml`](.github/workflows/release.yml) builds and
publishes releases automatically when a version tag starting with a digit (e.g.,
`3.2` or `3.2.0-rc1`) is pushed:

```sh
git tag -a 3.2 -m "Release 3.2"
git push origin 3.2
```

Tags containing `-` are published as pre-releases. The workflow can also be
triggered manually via `workflow_dispatch` to produce dry-run archive artifacts
without creating a GitHub Release.

## License and authors

The DCL is released under the Mozilla Public License 1.1. The original authors
are:

- Paul Guyot https://github.com/pguyot
- Nicolas Zinovieff https://github.com/krugazor
- Michael Vacìk https://www.linkedin.com/in/mvacik

This version also includes changes by:

- Eckhart Köppen https://github.com/ekoeppen

This distribution also includes a subset of K Libs by the same authors.
