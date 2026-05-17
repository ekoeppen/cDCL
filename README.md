![C/C++ CI](https://github.com/pguyot/DCL/workflows/C/C++%20CI/badge.svg)

The Desktop Connection Library (DCL) is a multi-platform library to
connect to Apple Newton Devices using the Dock protocol and to manipulate
NewtonOS data.

The library and sample code programs run on macOS X, Linux and Cygwin.

## Compilation and portability ##

This release of the DCL should be compiled with CMake:

```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

## Testing ##

Tests are executed using `ctest` from the build directory:

```sh
ctest . -C Debug
```

Note that the `doxygen` test is marked as optional since it fails due to
some documentation gaps, it can be excluded from test execution:

```sh
ctest . -C Debug -LE optional
```

## License and authors ##

The DCL is released under the Mozilla Public License 1.1. The original
authors are:

  - Paul Guyot https://github.com/pguyot
  - Nicolas Zinovieff https://github.com/krugazor
  - Michael Vacìk https://www.linkedin.com/in/mvacik

This version also includes changes by:

  - Eckhart Köppen https://github.com/ekoeppen

This distribution also includes a subset of K Libs by the same authors.

