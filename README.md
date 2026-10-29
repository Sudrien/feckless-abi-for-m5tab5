# Feckless ABI for M5Tab5

The ABI between a launcher on the M5Stack Tab5 and the apps it loads
with esp_elf (Espressif's
[elf_loader](https://github.com/espressif/esp-iot-solution/tree/master/components/elf_loader)),
as an ESP-IDF component.

The launcher links the libraries, pinned here:
[feckless-drivers](https://github.com/Sudrien/feckless-drivers-for-m5tab5)
and
[feckless-graphics-handler](https://github.com/Sudrien/feckless-graphics-handler-for-m5tab5).
An app is an ELF that contains only its own code and what it links
statically, and leaves every call into the libraries, FreeRTOS, the heap,
the log, libc, libm and libgcc for the launcher to supply.
`exports.txt` is the list of what it may leave.

## Version 1

- **feckless_drivers v0.5.9 and feckless_graphics_handler v0.2.7**,
  their whole public API: every global their public headers declare.
  That is 110 functions and the font tables.
- **A fixed set from ESP-IDF and the toolchain**: FreeRTOS tasks,
  queues, semaphores and event groups; the heap; esp_timer; the log;
  esp_err; libc's strings, formatting and sorting; libm, float and
  double; libgcc's double-precision and 64-bit helpers.

Nothing else. File I/O and feckless-storage-handler are not in v1.

**The board is the app's.** A launcher starts an app on a board nothing
has initialised, and the app calls `tab5io_init()`, `lcd_init()` and the
rest, as in a firmware of its own. None of them may be called twice.

## An app

```yaml
# main/idf_component.yml
dependencies:
  feckless_abi:
    git: https://github.com/Sudrien/feckless-abi-for-m5tab5.git
    version: "v0.1.0"
```

`feckless_abi` goes in main's `REQUIRES`, and `feckless_abi_elf()` goes
in the project's CMakeLists.txt in place of elf_loader's `project_elf()`:

```cmake
project(myapp)
set(ELF_COMPONENTS ...)     # anything to link into the ELF, e.g. esp-dsp
feckless_abi_elf(myapp)     # writes build/myapp.app.elf
```

The first thing in `app_main()`:

```c
#include "feckless_abi.h"

void app_main(void)
{
    feckless_abi_require();
    ...
```

The build checks the ELF. It fails on any undefined symbol that is not
in `exports.txt`: link that one into the ELF with `ELF_COMPONENTS`, or
it needs a new ABI version. It also fails without
`feckless_abi_require()`.

Lothesome Audio Analyzer is an app built this way, and also builds a
standalone firmware that embeds its own ELF and loads it through this
component:
[lothesome-audio-analyzer-for-m5tab5](https://github.com/Sudrien/lothesome-audio-analyzer-for-m5tab5).

## A launcher

`feckless_abi` goes in main's `REQUIRES`, and then:

```c
#include "feckless_abi.h"

int err = feckless_abi_run(image, size);   /* runs app_main(); returns only on failure */
```

`image` may be in RAM or memory-mapped flash, and must stay there while
the app runs. esp_elf loads the app into PSRAM, so the launcher's
sdkconfig needs:

```
CONFIG_SPIRAM=y
CONFIG_ELF_LOADER_LOAD_PSRAM=y
CONFIG_ESP_SYSTEM_MEMPROT_FEATURE=n
```

An app's sdkconfig should match its launcher's: struct layouts and
inline functions in the headers depend on it.

## Versions

`feckless_abi.h` defines `FECKLESS_ABI_VERSION` and a symbol named for
it, `feckless_abi_v1`. The launcher defines the symbol; every app refers
to it. An app built for another version fails in `esp_elf_relocate()`,
and esp_elf logs `feckless_abi_v1` (or whichever) as the symbol it could
not find. It fails before any of the app runs.

- **Adding** a name to `exports.txt` is compatible. Apps built against
  the shorter list still load; a new app that uses the name needs a
  launcher that has it. That is a minor release.
- **Removing** a name, or changing what one means, is a new version:
  a signature, a struct, or a feckless tag moved in `idf_component.yml`.
  Bump `FECKLESS_ABI_VERSION` and `FECKLESS_ABI_SYMBOL` together, in a
  major release.

After moving a tag, `tools/feckless_exports.py <build> <managed_components>`
prints the libraries' public API afresh, for the feckless section of
`exports.txt`.

## Licence

MIT. See `LICENSE`.
