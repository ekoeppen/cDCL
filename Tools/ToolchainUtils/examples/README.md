# Sample code using DCL toolchain utils

## NTK native code module

Source files:
* ntkmodule.cpp
* ntkmodule.exp
* ntkmodule.map
* ntkmodule.ld
* PDemoInterface.h
* PDemoInterfaceGlue.s
* NewtonRuntime/NewtonPublicAPIs.s
* NewtonRuntime/NewtonExceptions.cpp

Features six functions:

* GetConstString()
* GetArrayElement(ix)
* BreakInMonitor()
* ProtocolGetConstString()
* ProtocolGetArrayElement(ix)
* ProtocolBreakInMonitor()

The functions are declared in ntkmodule.exp and ntkmodule.map.

GetArrayElement(ix) demoes code relocation. The code is compiled in position
independent mode, with no writable global variable, however relocation is
required for the typical case illustrated by GetArrayElement(ix):

static const char* const constArray[] = { "Area 51", "Groom Lake" };

The assembler will generate an array of pointers pointing to two const strings
and requires that the dynamic linker rewrites the pointers to actually point
to the strings.

ELFtoNTK generates the proper NSOF frame with relocation information and NTK
will generate a package with the proper relocation header.

Please note that there is a bug in NTK (?) and that calling the function
from NTK inspector will not work. However, if you call it from your code or a
tool such as ViewFrame, it will work as expected.

BreakInMonitor is useful only in the context of Einstein, the actual code will
crash the Newton (it may catch this as an exception, though).

NewtonPublicAPIs.s contains vector to NewtonOS public APIs. Most functions are
called by their MPW-mangled name, thus incompatible with gcc. You will need to
modify the file to set the proper function name using arm-elf-none-nm (or just
nm) in your object file to find out how the function is mangled. Automating this
process is not so easy.

NewtonExceptions.cpp contains definitions of Newton exceptions global constants
which should be included in the final binary.

The Protocol* functions work by instantiating a specific protocol named PDemo
and invoking the associated functions.

PDemoInterfaceGlue.s is an hand-made interface glue code based on what
ProtocolGenTool generates on MPW. ProtocolGenTool generates assembly code for
ARM assembler which has a different syntax, plus the C++ symbols are mangled
differently, it was easier to hand-write it.

## Protocol implementation

Source files:
* PDemoInterface.h
* PDemoInterfaceGlue.s
* PDemo.cpp
* PDemo.impl.h
* PDemo.impl.s
* protocol.map
* protocol.ld
* NewtonRuntime/NewtonPublicAPIs.s
* NewtonRuntime/NewtonExceptions.cpp

The protocol implementation glue (PDemo.impl.s) has been hand-written for the
same reasons as above.

This simplistic protocol defines and implements three methods:
* PDemoInterface::GetConstString()
* PDemoInterface::GetArrayElement(ix)
* PDemoInterface::BreakInMonitor()

They are called from the NTK module above using the interface glue.

ELFtoPKG generates a protocol package from PDemo.cpp and PDemo.impl.s source
files with a single part containing the protocol implementation. The protocol
is automatically registered as -autoLoad and -autoRemove are passed to the tool.
This generated package (protocol.pkg) can be installed separately or be included
in an NTK project in which case NTK will rewrite relocations as appropriate.

Please note that relocation of protocols is not something that was supported by
Newton C++ tools, yet it works as expected.
