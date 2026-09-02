# esp32-gui

ESP-IDF 5.4 + LVGL 9.5.0 + SquareLine Studio workspace, wired for Zed.
Includes a desktop SDL simulator that compiles the *same* UI sources as the firmware.

> **Handing this to someone else? Point them at [HANDOVER.md](HANDOVER.md).**
> It covers installing ESP-IDF, configuring `board_config.h` for their panel,
> bringing the display up, the SquareLine workflow, and the traps worth knowing.

## Status

| Piece | State |
|---|---|
| ESP-IDF v5.4 (`~/esp/esp-idf`) | installed; xtensa + riscv32 toolchains |
| Target | esp32s3 (`sdkconfig.defaults`) |
| `idf.py build` | **passing** — `build/esp32-gui.bin`, 80% of app partition free |
| LVGL / esp_lvgl_port | **lvgl 9.5.0** / **esp_lvgl_port 2.9.0** (see `dependencies.lock`) |
| clangd in Zed | **working** — `clangd --check` reports 0 errors |
| Desktop simulator (`sim/`) | written but **never compiled** — optional, see HANDOVER.md §8 |
| Board profiles | 5 profiles, **all compile clean** (ST7789/ILI9341/GC9A01/ST7796/RGB) |
| Flash to hardware | not done — no board available |
| SquareLine export | blocked — Studio not installed, `squareline_ui/` empty |

## Daily use

```bash
get_idf                 # alias for . $HOME/esp/esp-idf/export.sh
idf.py build
idf.py -p $ESPPORT flash monitor
```

Or `cmd-shift-p` -> `task: spawn` in Zed (10 tasks defined).

## Known blocker: Xcode Command Line Tools

The CLT install is destroyed (a macOS 26 upgrade left the receipt but removed
`/Library/Developer/CommandLineTools/usr/`). `xcode-select --install` fails with
"can't find the software" because this Mac is on macOS 26.0.1 (build 25A362) and
Apple's catalog no longer carries a CLT keyed to that build.

**This does not affect the firmware.** The ESP32 cross-compilers are self-contained;
`idf.py build` and clangd both work without CLT. Only the desktop simulator needs it.

Fix by downloading the DMG directly from https://developer.apple.com/download/all/
(search "Command Line Tools", free Apple ID), or by updating macOS to 26.6.2 first.

## Changing the chip target

`sdkconfig.defaults` holds the target on one line:

```
CONFIG_IDF_TARGET="esp32s3"
```

Edit that line (`esp32`, `esp32s2`, `esp32s3`, `esp32c3`, `esp32c6`, `esp32p4`, ...)
then run the Zed task **ESP-IDF: Set Target (from sdkconfig.defaults)**, or:

```bash
idf.py fullclean && idf.py set-target "$(sed -n 's/^CONFIG_IDF_TARGET="\(.*\)"$/\1/p' sdkconfig.defaults)"
```

## clangd / compile_commands.json

ESP-IDF emits `build/compile_commands.json`. Zed's clangd looks in the workspace
root, so symlink it **after the first successful build**:

```bash
ln -sf build/compile_commands.json compile_commands.json
```

This is a relative symlink, so it survives moving the project and re-points itself
every rebuild. Re-run it only if you `fullclean` and the link goes dangling.
Also available as the Zed task **ESP-IDF: Link compile_commands.json (clangd)**.

`.clangd` in the repo root strips the Xtensa/RISC-V GCC flags clangd cannot parse.

## squareline_ui/

Empty drop folder for the SquareLine Studio export (`ui.c`, `ui.h`,
`ui_helpers.*`, `screens/`, `components/`, `fonts/`, `images/`).

It is **not** in the build yet — an empty component dir is inert. To activate it
once the export is in place:

**a.** Create `squareline_ui/CMakeLists.txt`:

```cmake
file(GLOB_RECURSE UI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

set(UI_INCLUDE_DIRS ".")
file(GLOB UI_CHILDREN LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
foreach(child ${UI_CHILDREN})
    if(IS_DIRECTORY ${child})
        list(APPEND UI_INCLUDE_DIRS ${child})
    endif()
endforeach()

idf_component_register(SRCS ${UI_SOURCES}
                       INCLUDE_DIRS ${UI_INCLUDE_DIRS})
```

**b.** Create `squareline_ui/idf_component.yml` so it can `#include "lvgl.h"`:

```yaml
dependencies:
  lvgl/lvgl: "^9.0.0"
```

(Declaring the dep in the manifest is what adds `lvgl` to this component's
requirements — do not hand-write `REQUIRES lvgl`, managed components are
namespaced as `lvgl__lvgl`.)

**c.** Uncomment the `EXTRA_COMPONENT_DIRS` line in the root `CMakeLists.txt`.

**d.** Add `squareline_ui` to `main/CMakeLists.txt`:

```cmake
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS "."
                       REQUIRES squareline_ui)
```

In SquareLine Studio set **Export root** to this folder and enable
*Project Settings -> LVGL version 9*.

## Zed tasks

`cmd-shift-p` -> `task: spawn`. Build, Flash, Flash + Monitor, Monitor,
Menuconfig, Set Target, Fullclean, Link compile_commands.json.

Each task sources `export.sh` itself, since Zed's task shell is not your
`get_idf` shell. Override the IDF location by exporting `IDF_PATH` in your shell
profile, or edit the fallback `$HOME/esp/esp-idf` in `.zed/tasks.json`.

Exit `idf.py monitor` with `ctrl-]`.
