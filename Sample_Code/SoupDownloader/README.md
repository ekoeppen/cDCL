# SoupDownloader

Read-only export of a named soup from **every Newton store**, using the Newton
OS 2.x Dock/Dante protocol over serial/MNP. Internal storage and inserted cards
are visited in the order returned by the Newton. Stores without the named soup
are skipped. The name is an exact, case-sensitive match; command-line strings
are UTF-8 and soup names are sent as null-terminated UTF-16BE.

The sequence follows the related RDCL implementation in
`rdcl/apps/nwt/export_module.rb` and `rdcl/link/dock_modules/storage_dock_module.rb`:
select each store, select the soup, send `snds`, receive `entr` until `bsdn`.
The output directory is created if absent, or reused if it already exists.
The directory itself and unrelated files are never deleted.

## Build and run

On macOS or Linux:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target SoupDownloader

build/Sample_Code/SoupDownloader/SoupDownloader \
  --serial /dev/cu.usbserial-XXXX \
  --baud 38400 \
  --soup Names \
  --output ./Names-export
```

Start a serial connection from the Newton's Dock application. The program waits
for the Newton to connect. A PTY path can also be used, for example with an
emulator or serial bridge. Baud defaults to 38400. The output directory's parent
must exist. An existing output path must be a directory.

If a Dock password is configured, supply it in the `NEWTON_PASSWORD` environment
variable. Otherwise the empty password is used. Do not select an installation
or Inspector connection: this tool negotiates a full Dock session.

### MNP logging

MNP frame logging is compile-time configurable via `KDebug` (disabled by
default). Configure a Debug build with `-DDCL_MNP_FRAME_LOGGING=ON` (or
`-Ddebug_TDCLMNPPipe=1`) to log timestamped incoming/outgoing decoded MNP frames
and framing errors to stderr, with hex/ASCII dumps (the same format as
Inspector). Wire escaping and CRC bytes are not included.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DDCL_MNP_FRAME_LOGGING=ON
cmake --build build --target SoupDownloader
build/Sample_Code/SoupDownloader/SoupDownloader \
  --serial /dev/cu.usbserial-XXXX --soup Names --output ./Names-export \
  2>mnp.log
```

When enabled, the log also contains status/error messages and may expose private
soup data and authentication traffic. Treat it as sensitive. DES key/round
diagnostics are disabled by default; stdout is normally empty.

The Newton's operation buttons are deliberately hidden: the desktop drives this
export, so an otherwise empty Dock dialog is expected while it runs.

## Output

```text
Names-export/
  soup-name.nsof             selected soup name (NewtonScript string)
  stores.nsof                array of store frames returned by the Newton
  store-000001/              first store in that array, if it contains the soup
    store.nsof               selected store frame (including its signature)
    soup-names.nsof          array of soup names on this store
    soup-signatures.nsof     corresponding array of soup signatures
    soup-info.nsof           selected soup's metadata
    indexes.nsof             selected soup's index definitions
    entry-000001.nsof        first received entry
    entry-000002.nsof        second received entry
    ...
    COMPLETE                entry count; published only after bsdn
  store-000003/              numbering can have gaps for skipped stores
    ...
  COMPLETE                  exported-store count; all stores have been visited
```

Each NSOF file is a standalone stream, including its NSOF version byte. Entry,
metadata, and index payloads are saved verbatim, without the Dock header or
four-byte alignment padding. Entry filenames are receive-order counters, **not
Newton unique IDs**; `_uniqueID` remains in the entry itself. Entries from
different stores are never merged, even if they have the same ID.

Reusing a directory starts a **fresh export**, not a resumed download. Generated
metadata files are overwritten. Old completion markers are cleared at startup;
previous numbered entry files are removed from each matching store before it is
exported, so shorter or empty soups cannot leave stale entries. Unrelated files
are preserved. Data from stores skipped or absent in the new run is retained,
but without a `COMPLETE` marker; use only completed store directories for the
current export.

Only one entry is buffered at a time. Entries do not need to be decoded to be
saved, so unfamiliar NSOF objects are preserved. VBO compression is explicitly
disabled before requesting data because cDCL cannot decode compressed VBOs.
The `cvbo` command has no reply; `gsto` follows immediately, without waiting for
a `dres`.
The saved metadata does not constitute a complete device backup, and the tool
does not implement restoration. Treat exports as private Newton data: store
frames can contain password-related information.

The exit status is 0 only after a complete export and normal shutdown; 1 means
an error (including no matching soup on any store), and 130 means interruption.
Partial files are retained for inspection. Absence of the top-level `COMPLETE`
means the overall export did not finish; individual stores may still be complete.
An empty matching soup is a successful export with zero entries. Errors selecting
a store, errors accessing a listed soup, disk errors, or Newton cancellation stop
the operation rather than silently skipping potentially unread data.

## Tests

```sh
cmake --build build --target SoupDownloader SoupDownloaderTests
ctest --test-dir build -R '^soup-downloader' --output-on-failure
```

The scripted tests pass commands through the real Dock wire encoder/decoder and
cover multiple stores, missing and empty soups, UTF-16BE selection, byte-exact
NSOF export without padding, metadata, completion markers, keepalives,
cancellation, error results, disk errors, directory reuse and stale-entry cleanup,
and the received-single-long endian regression.

When Python 3 is available at CMake configure time, `soup-downloader-pty` also
launches the actual CLI against a private PTY. It verifies the MNP LR/LA
handshake, full Dock authentication, `cvbo` without a reply, store/soup selection,
and a complete export of an entry spanning several MNP frames. It checks the
saved bytes, buffered end-of-soup processing, normal disconnect, MNP logging
disabled by default, and the absence of DES dumps on stdout. A separate scenario
checks clean failure on premature peer disconnect. The peer uses a private mode
of `SoupDownloaderTests` only to compute DES challenge responses; its framing
and NSOF fixtures are independent. No live Newton/emulator PTY is opened by this
test. These tests do not replace a real Newton transfer test.
