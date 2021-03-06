![C/C++ CI](https://github.com/pguyot/DCL/workflows/C/C++%20CI/badge.svg)

The Desktop Connection Library (DCL) is a multi-platform library to
connect to Newtons using the Dock protocol and to manipulate NewtonOS
data.

## Compilation and portability ##

This release of the DCL should be compiled with cmake.
The library and sample code programs run on macOS X and Linux.

    mkdir build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Debug
    cmake --build .
    ctest . -C Debug

## License and authors ##

The DCL is released under the Mozilla Public License 1.1. The original
authors are:

  - Paul Guyot https://github.com/pguyot
  - Nicolas Zinovieff https://github.com/krugazor
  - Michael Vacìk https://www.linkedin.com/in/mvacik

This version also includes changes by:

  - Eckhart Köppen https://github.com/ekoeppen

This distribution also includes a subset of K Libs with the same authors.

