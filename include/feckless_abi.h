/* SPDX-License-Identifier: MIT
 *
 * The ABI between a Tab5 launcher and the apps it loads with esp_elf.
 *
 * An app is an ELF built by feckless_abi_elf() (project_include.cmake):
 * its own code and whatever it links statically, with every call into
 * the feckless libraries, FreeRTOS, the heap, the timer, the log, libc,
 * libm and libgcc left undefined. exports.txt is the list it may leave
 * undefined; the launcher hands esp_elf a table of the same list.
 *
 * The board is the app's. A launcher starts an app on a board nothing
 * has initialised -- tab5io_init(), lcd_init() and the rest are the
 * app's to call, as in a firmware of its own, and none of them may be
 * called twice.
 */
#pragma once

#include <stddef.h>
#include <stdint.h>

#define FECKLESS_ABI_VERSION 1

/* The symbol that names the version. The host defines it; an app's ELF
 * refers to it (feckless_abi_require()), so on a host of another version
 * esp_elf_relocate() fails and logs this name, and nothing of the app
 * runs. */
#define FECKLESS_ABI_SYMBOL feckless_abi_v1

extern const uint32_t FECKLESS_ABI_SYMBOL;

/* An app calls this first thing in app_main(). It does nothing at run
 * time; it is the reference that ties the ELF to the version. The ELF
 * build fails without it. */
static inline void feckless_abi_require(void)
{
    __asm__ volatile("" :: "r"(&FECKLESS_ABI_SYMBOL));
}

/* ---- the host's side --------------------------------------------------- */

/* Every symbol in exports.txt, and FECKLESS_ABI_SYMBOL. Typed as void so
 * that an app need not include esp_elf.h: it is an esp_elf_symbol_table_t. */
extern const void *const feckless_abi_table;

/* Relocate an app's ELF image and call its app_main(). The image may be
 * in flash (mmapped) or RAM, and must stay there while the app runs.
 * Apps do not return; if one does, so does this, with 0. If the image
 * will not load, returns esp_elf's negative errno. */
int feckless_abi_run(const uint8_t *image, size_t size);
