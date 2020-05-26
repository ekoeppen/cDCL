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

with Arity being an integer (from 0). Comments are line starting with `;`. Empty lines are allowed.

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
