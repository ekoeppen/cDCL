# Inspector console

Build with `cmake -S . -B build && cmake --build build --target Inspector`.

Run `build/Sample_Code/Inspector/Inspector --serial /dev/cu.usbserial-10 --baud 38400`
(or supply a PTY slave path in place of the serial device). The Newton must initiate
an **Inspector** connection, not a Dock/package-install connection. The program
accepts the Inspector handshake over MNP and prints incoming `text` commands to
stdout. It does not send arbitrary expressions or other commands to the Newton.
Press Ctrl-C to stop; `--help` lists the options. Windows is not supported.

A live Newton/Inspector connection is required to validate the handshake and
text display; the build and CLI smoke tests do not exercise physical serial I/O.
