# Apple Newton Desktop Connectivity Library (cDCL)

![C/C++ CI](https://github.com/ekoeppen/cDCL/workflows/C/C++%20CI/badge.svg)

The Desktop Connectivity Library (cDCL) is a modern C++ multi-platform library
designed to connect to Apple Newton devices via the Newton Dock protocol,
manipulate NewtonOS data structures (`NSRef`, frames, symbols, packages), and
provide core streaming and serialization utilities.

The repository consists of two main libraries:

- **`DCL`**: Domain-specific implementation of the Newton Dock protocol, package
  encoding/decoding, and object stores.
- **`K`**: Foundational systems library providing binary streams, ring buffers,
  memory-mapped files, and endianness utilities.

## Architecture & Design

### Modern C++17 Standard

The codebase targets **C++17** (`-std=c++17`) using standard primitives
(`std::thread`, `std::mutex`, `std::condition_variable`, `<atomic>`, and
`<chrono>`).

### Streamlined Platform Strategy

Builds target three platform tiers using built-in compiler macros
(`__APPLE__`, `__linux__`, `_WIN32`):

- **macOS** (Clang / libc++)
- **Linux** (GCC / Clang / libstdc++)
- **Windows** (native Win32 via MinGW-w64)

Compared to the original [DCL](https://sourceforge.net/projects/newton-dcl/),
legacy platforms (Cygwin, MSVC, Classic Mac OS 8/9, BeOS) are dropped. macOS and
Linux share a unified POSIX implementation (`TARGET_OS_POSIX`), while Windows
relies on native Win32 APIs (`TARGET_OS_WIN32`) via MinGW-w64.

### Compatibility Shims

To permit downstream consumers (such as Einstein) and legacy tools to migrate
smoothly without high churn:

- Legacy types (`KUInt32`, `KSInt16`, `Boolean`, etc.) are provided as
  lightweight `using` aliases to standard `<cstdint>` types (`uint32_t`,
  `int16_t`) and `bool`.
- Concurrency facades (`K::TThread`, `K::TMutex`, `K::TCondVar`) remain
  available as thin wrappers encapsulating standard C++17 concurrency.

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

### Header Include Conventions

The include directories are configured so downstream consumers can use either
namespaced includes or flat includes:

```cpp
// Namespaced (preferred):
#include <K/Misc/TCircleBuffer.h>
#include <K/Defines/UByteSex.h>
#include <DCL/NS_Objects/Objects/TDCLNSFrame.h>

// Flat includes are also supported:
#include "TCircleBuffer.h"
#include "TDCLNSFrame.h"
```

## Compilation

Build with CMake (requires CMake 3.31+ and a C++17 compiler):

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

Tests are executed using `ctest` from the build directory:

```sh
ctest --test-dir build -C Debug --output-on-failure
```

The `doxygen` test is marked as optional and can be excluded if Doxygen is not
installed:

```sh
ctest --test-dir build -C Debug -LE optional
```

## License and authors

The DCL is released under the Mozilla Public License 1.1. The original authors
are:

- Paul Guyot https://github.com/pguyot
- Nicolas Zinovieff https://github.com/krugazor
- Michael Vacìk https://www.linkedin.com/in/mvacik

This version also includes changes by:

- Eckhart Köppen https://github.com/ekoeppen

This distribution also includes a subset of K Libs by the same authors.
