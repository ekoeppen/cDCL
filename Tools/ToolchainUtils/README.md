# Toolchain utilities

Replacement tools based on DCL code to generate packages/NSOF streams.

* `ELFtoNTK` replaces `AIFtoNTK` to generate native code modules for NTK from an ELF shared library
* `ELFtoPKG` replaces `Packer` to generate packages with a protocol part from an ELF shared library

## ELFtoNTK

Replacement to `AIFtoNTK` to build native code modules for NTK from ELF shared libraries.

Usage:

`ELFtoNTK -via expfile -o output.ntkc input.elf`
`SetFile -c NTP1 -t ntkc output.ntkc`

(Invoking `SetFile` is required for NTK running in a Classic environment)

The code is quite simplistic and does not perform all necessary checks to make sure everything will work as expected.

To build a compatible ELF dynamic library, the requirements are as follows:
- code must be for ARMv3 or ARMv4 (MP2x00 only)
- code must be in big-endian
- code must not include any read/write global variable
- code should be position independent
- code must have origin base address of 0 (actually a limitation of the tool)
- functions for use from NewtonScript should be exported.

Please see sample code in examples subdirectory for invocation from the
Makefile.

The suggested approach is to provide both a linker script that packs the
required sections and a version script that is used to strip down the ELF
library to the exported function code.

AIFtoNTK required a .exp file with the name of the module and the functions. It
used debugging information in the AIF binary file to infer the number of
arguments. ELFtoNTK requires the number of arguments to be specified in the
.exp file with the following format :

```
ModuleName
FunctionName:Arity
```

with Arity being an integer (from 0). Comments are line starting with `;`. Empty
lines are allowed.

## ELFtoPKG

This tool takes a dynamic ELF code and generates a package from it.

Usage:

`ELFtoPKG -o output.pkg package_name -protocol input.elf`
`SetFile -t 'pkg ' -c 'pkgX' output.pkg`

(Invoking `SetFile` is required for NTK running in a Classic environment)

To build a compatible ELF executable, the requirements are as follows:
- code must be for ARMv3 or ARMv4 (MP2x00 only)
- code must be in big-endian
- code must not include any read/write global variable
- code should be position independent
- code must have origin base address of 0 (actually a limitation of the tool)
- code should start with the protocol glue in an .interp section.

Unlike DDK and Newton C++ Tools, ELFtoPKG can generate relocation-aware code.

Please see sample code in examples subdirectory for invocation from the
Makefile.

The suggested approach is to put the protocol glue in the .interp section,
provide a linker script that packs the required sections and a version script
that is used to strip down the ELF library to code reachable from the protocol
glue.

## Global variables

Writable global variables are still not supported. A possible fix would be to
use the new fdpic runtime (-mfdpic) with gcc, which seems to work in 32 bits,
with a small linker glue with a r/w area. A prototype was built at some point,
however please note that fdpic support was added to gcc from version 10 while
ARMv3 support was dropped and generated code would only run on MP2x00 which is
of ARMv3 architecture. Alternatively, fdpic code could be backported to gcc8.

## C++ ABI

NewtonOS C++ ABI does not match modern ARM C++ ABI.
Symbol names are not mangled identically and conversion of many functions have
been applied to NewtonPublicAPIs.s. There might be some errors and functions
with compressed mangled names were not translated automatically.

There are several differences that might bite you eventually.

### Destructors

Destructors in NewtonOS ROM take an additional parameter in r1 determining if
they should behave like a deleting destructor or like a complete object
destructor, while modern C++ ABI defines destructors with D0, D1 and D2 names.

To help with this, NewtonPublicAPIs.s exports complete object destructors
(which have D1 names) with a small glue, setting r1 to 0 before calling to the
ROM destructor, as well as deallocating destructors (named D2) with another
glue.

For base objects with non-virtual destructors, g++ will refer to a D2 destructor
instead of the D1 destructor. For simplicity, all three destructors are provided
in NewtonPublicAPIs.s, even if some are meaningless.

### operator new

Norcroft's ARMCpp also transformed call to new operator to calls to the
constructor, even if it is private, which g++ doesn't.

To illustrate the case, consider the following code.

```
class T {
public:
    static T* New();
    void Delete();
private:
    T();
    ~T();
};

inline T* T::New() { return new T; }
inline void T::Delete() { delete this; }
```

Invoking `T::New()` will
- call `__ct__1TFv` with r0 = 0 with ARMCpp (constructor will allocation).
- call `_Znwj` with r0 = 1 and `_ZN1TC1Ev` with result with g++ (new of one byte followed by constructor).

Invoking `t->Delete()` will
- call `__dt__1TFv` with `r0 = t` and `r1 = 1` with ARMCpp (destructor with free).
- call `_ZN1TD1Ev` with `r0 = t` and `_ZdlPvj` with `r0 = t` and `r1 = 1` with g++.

This is ok if T is properly defined, with the destructor glue mentioned above.
The problem occurs with classes CBufferList and CBufferSegment that are not
properly defined in system headers. They are larger than what the definition
says. To prevent you from being bitten, symbols to their constructors are not
converted in NewtonPublicAPI.s and the linker will fail if you invoke
`CBufferList::New()` or `CBufferSegment::New()`. Instead, two fixed New
functions are provided, with the actual Norcroft mangled name. You need to add
the declarations to invoke them.

Instead of:
```
CBufferList* myList = CBufferList::New();
```

do:

```
extern "C" CBufferList* New__11CBufferListFv(void);
extern "C" CBufferSegment* New__14CBufferSegmentFv(void);

...

CBufferList* myList = New__11CBufferListFv();
```

### Virtual functions

Virtual functions are laid out differently and therefore you should not
invoke virtual functions of system classes or subclass system class. Virtual
functions in your own classes, however, might work with relocation.

Please note that virtual function layout changed between NewtonOS 2.0 and 2.1.
For more details, see note
[Events and NewtWorld](https://www.kallisys.com/files/newton/Bowels/Events_and_NewtWorld.txt).

Provided are updated TAEventHandlerProxy source code that work with g++.

### Other C++ features.

C++ (native) exceptions and rtti are not supported. NewtonOS exceptions do work
using macros from NewtonExceptions.h. Please compile with `-fno-exceptions`
and `-fno-rtti` to make sure the generated code doesn't refer to missing runtime
glue.
