# esp32-gui — Handover

An ESP-IDF 5.4 project scaffolded and pre-wired for **LVGL 9.5** + **SquareLine Studio**,
ready for you to point at your specific board.

The environment setup, dependency wiring, and display/touch bring-up code are done and
**verified to compile**. What's left is the part that needs the physical hardware:
telling it your pin numbers, and dropping in your UI.

---

## 1. What is already done

| | |
|---|---|
| ESP-IDF project skeleton | done, target `esp32s3` (easy to change) |
| LVGL 9.5.0 + esp_lvgl_port 2.9.0 | resolved via Component Manager, pinned in `dependencies.lock` |
| Panel drivers | ILI9341, GC9A01, ST7796 (+ ST7789 built into IDF) |
| Touch drivers | GT911, FT5x06, CST816S |
| Display/touch/LVGL bring-up | `main/display.c`, complete, SPI **and** RGB parallel |
| Board abstraction | `main/board_config.h`, 5 ready profiles |
| SquareLine integration | `squareline_ui/` component, auto-compiles any export |
| Zed editor integration | 10 tasks, clangd working (0 errors) |

**Compile-verified:** all five board profiles build clean, and the SquareLine path was
tested with a stand-in export (`ui_init` confirmed present in the ELF).

**NOT verified — be aware:**
- **Nothing has ever run on real hardware.** No board was available. Expect to spend
  your first session on display bring-up (§5 has a symptom table).
- **The GPIO numbers in `board_config.h` are placeholders.** They are not your board's
  pinout. They will not work until you replace them. Everything else in a profile
  (driver, resolution, colour order, timings) is already correct for that panel family.
- The desktop simulator in `sim/` was never compiled — see §8.

---

## 2. Install ESP-IDF

```bash
mkdir -p ~/esp && cd ~/esp
git clone -b v5.4 --recursive https://github.com/espressif/esp-idf.git
~/esp/esp-idf/install.sh esp32,esp32s3,esp32c3
echo "alias get_idf='. \$HOME/esp/esp-idf/export.sh'" >> ~/.zshrc   # or ~/.bashrc
```

`--recursive` is not optional; without it the build fails much later with confusing
errors. If you forget: `cd ~/esp/esp-idf && git submodule update --init --recursive`.

Open a new terminal, then `get_idf` in any shell where you want `idf.py`.

> **macOS note:** ESP-IDF does *not* need Xcode Command Line Tools — the ESP32
> cross-compilers are self-contained. This project was built start to finish on a Mac
> with a broken CLT install. Only the optional simulator (§8) needs a host compiler.

---

## 3. First build

```bash
cd <this directory>
get_idf
idf.py set-target esp32s3     # only if changing chip; already configured
idf.py build
```

First build downloads the 11 dependencies into `managed_components/`. Then:

```bash
ln -sf build/compile_commands.json compile_commands.json   # for clangd, once
```

---

## 4. Configure for YOUR board  ← the main job

Everything hardware-specific is in **`main/board_config.h`**. You should not need to
touch any other file.

**Step 1 — pick your panel.** Near the top:

```c
#define BOARD_PROFILE  BOARD_ST7789_SPI
```

| Profile | Panel | Res | Bus | Touch |
|---|---|---|---|---|
| `BOARD_ST7789_SPI` | ST7789 | 240×320 | SPI | none |
| `BOARD_ILI9341_SPI` | ILI9341 | 240×320 | SPI | none |
| `BOARD_GC9A01_ROUND` | GC9A01 | 240×240 | SPI | CST816S |
| `BOARD_ST7796_SPI` | ST7796 | 320×480 | SPI | FT5x06 |
| `BOARD_RGB_800X480` | RGB parallel | 800×480 | 16-bit RGB | GT911 |

**Step 2 — fill in the pins** in that profile's `PINS` block, from your board's
schematic or product page. For an SPI panel that's `SCLK, MOSI, DC, CS, RST, BL`;
for RGB it's the sync pins plus 16 data lines.

**Step 3 — set the touch pins** at the bottom of the file (`BOARD_PIN_TOUCH_SDA/SCL/RST/INT`)
if your profile uses touch. Set `BOARD_TOUCH` to `TOUCH_NONE` to skip touch entirely
while you get the display working — recommended for the first bring-up.

Then `idf.py build flash monitor`.

### If your hardware isn't covered

- **XPT2046** (resistive touch, common on ILI9341 "cheap yellow display" boards) is
  *not* included — it's a community component on a second SPI bus, not an Espressif
  one. Add with `idf.py add-dependency "atanisoft/esp_lcd_touch_xpt2046^1.0.5"` and
  extend `touch_init()` in `display.c`.
- **Other SPI panels** (ST7735, ILI9488, NV3041A…): add the driver component, then add
  a `#elif` branch in `display.c` next to the existing `esp_lcd_new_panel_*` calls.
  About 3 lines.
- **MIPI-DSI** is ESP32-P4 only; `esp_lvgl_port` has `lvgl_port_add_disp_dsi()` for it.

---

## 5. Bringing the display up

Flash the placeholder first — `main.c` draws an LVGL version label and a tappable
button, which proves panel + LVGL + touch end to end before any UI is involved.

```bash
idf.py -p /dev/cu.usbmodem101 flash monitor      # ctrl-] to exit
ls /dev/cu.*                                      # to find your port
```

If it won't enter download mode: hold **BOOT**, tap **RESET**, release **BOOT**, retry.

| Symptom | Fix (all in `board_config.h`) |
|---|---|
| Blank / white screen | wrong `BOARD_PIN_LCD_BL`, or `BOARD_LCD_BL_ON_LEVEL` inverted |
| Nothing at all, no log | wrong SPI pins — recheck `SCLK`/`MOSI`/`CS`/`DC` |
| Garbled or torn image | lower `BOARD_LCD_PCLK_HZ` to 20 MHz, work back up |
| Colours inverted (photo negative) | flip `BOARD_LCD_INVERT` |
| Red and blue swapped | flip `BOARD_LCD_BGR` |
| Snow / random pixels | `swap_bytes` — see the SPI branch in `display.c` |
| Image shifted a few pixels | set `BOARD_LCD_GAP_X` / `BOARD_LCD_GAP_Y` |
| Rotated / mirrored | `BOARD_LCD_SWAP_XY`, `MIRROR_X`, `MIRROR_Y` |
| Touch offset or inverted axes | same three flags — they feed the touch driver too |
| RGB panel: boot loop or no PSRAM | see §7, PSRAM is mandatory for 800×480 |

---

## 6. Adding your SquareLine UI

In **SquareLine Studio**: Project Settings → **LVGL version 9.x**, board *ESP32*, and
set the UI export path to this project's `squareline_ui/` folder. Then
**Export → Export UI Files**. Do *not* use "Create Template Project" — it would
overwrite this scaffold.

```bash
idf.py reconfigure     # <-- REQUIRED after adding or removing exported files
idf.py build flash monitor
```

> ### The one trap that will get you
> `squareline_ui/CMakeLists.txt` globs `*.c`, and CMake evaluates globs at **configure**
> time only. Drop in an export and run plain `idf.py build` and it will succeed while
> compiling **none of your UI** — no error, no warning, just the old binary.
>
> Always run `idf.py reconfigure` after adding or removing files in `squareline_ui/`.
>
> (The usual fix, `CONFIGURE_DEPENDS`, is illegal inside an ESP-IDF component — IDF
> re-evaluates component CMakeLists in script mode, where CMake rejects that flag.
> This was tried and reverted; don't re-add it.)

No code change is needed to activate your UI. `main.c` uses `__has_include("ui.h")`,
so it draws the placeholder until an export exists and calls your `ui_init()` after.

`ui_init()` is called while holding `lvgl_port_lock()`. **Any LVGL call you make from
your own tasks must hold that lock too** — LVGL is not thread-safe and `esp_lvgl_port`
runs it on its own task:

```c
if (lvgl_port_lock(0)) {
    lv_label_set_text(my_label, "hello");
    lvgl_port_unlock();
}
```

---

## 7. menuconfig settings you will need

`idf.py menuconfig`, then:

**Fonts** — *Component config → LVGL configuration → Font usage*.
Only Montserrat 14 is on by default. SquareLine will reference whatever sizes your
design uses, and a missing one is a **link error** (`undefined reference to
lv_font_montserrat_16`). Enable every size your design uses.

**LVGL memory** — *→ Memory settings → LV_MEM_SIZE*.
The default is small for a real UI. Raise to 48–64 KB. Symptom of running out:
widgets silently missing, `lv_malloc` warnings in the monitor.

**PSRAM** — required for `BOARD_RGB_800X480`, optional but helpful otherwise.
*Component config → ESP PSRAM* → enable, set mode (octal for most S3 boards) and speed.
Uncomment the matching lines in `sdkconfig.defaults` to make it stick across a
`set-target`.

**Flash size** — *Serial flasher config* → match your module (often 8 or 16 MB).

---

## 8. Optional: desktop simulator

`sim/` contains a complete SDL2 simulator that compiles the **same** `squareline_ui/*.c`
sources against the **same** LVGL 9.5.0, rendering to a desktop window. It lets you
iterate on UI in ~2 seconds instead of a flash cycle.

**It has never been compiled.** It was written on a Mac whose Xcode Command Line Tools
were broken, so it could not be verified. Treat it as a promising starting point, not
working code.

```bash
brew install sdl2 cmake ninja        # macOS
cd sim && cmake -B build -G Ninja . && ninja -C build && ./build/sim
```

Screen size: `cmake -B build -DSIM_HOR_RES=800 -DSIM_VER_RES=480 -DSIM_ZOOM=1 .`

If you don't want it: `rm -rf sim/` — nothing else depends on it. (`sim/lvgl/` alone is
276 MB, so deleting it makes this project far easier to move around.)

**Watch for config drift:** the simulator reads `sim/lv_conf.h` while the firmware reads
menuconfig's Kconfig. When they disagree you get "works in the sim, broken on device."
Fonts and `LV_MEM_SIZE` are the usual culprits. All 20 Montserrat sizes are enabled in
the sim; the firmware defaults to only size 14.

---

## 9. Project map

```
esp32-gui/
├── CMakeLists.txt            project root; pulls in squareline_ui
├── sdkconfig.defaults        chip target + LVGL-friendly defaults
├── main/
│   ├── board_config.h        ← ALL hardware specifics live here
│   ├── display.c/.h          panel + touch + LVGL bring-up (no edits needed)
│   ├── main.c                app_main; placeholder or your ui_init()
│   ├── CMakeLists.txt
│   └── idf_component.yml     LVGL, esp_lvgl_port, panel + touch drivers
├── squareline_ui/            drop the SquareLine export here
│   ├── CMakeLists.txt        auto-globs whatever you put in
│   └── idf_component.yml     declares the LVGL dependency
├── sim/                      optional desktop simulator (unverified)
├── .zed/
│   ├── tasks.json            10 build/flash/monitor tasks
│   └── settings.json         clangd --query-driver for the cross-compilers
├── .clangd                   strips GCC flags clangd can't parse
└── compile_commands.json     symlink → build/ (create after first build)
```

---

## 10. Zed setup

`cmd-shift-p` → `task: spawn` for Build, Flash, Monitor, Menuconfig, Set Target, and more.

Each task sources `export.sh` itself, because Zed's task shell is not the shell you ran
`get_idf` in. If your ESP-IDF is somewhere other than `~/esp/esp-idf`, either export
`IDF_PATH` in your shell profile or edit the fallback path in `.zed/tasks.json`.

`.zed/settings.json` points clangd at the cross-compilers with
`--query-driver=/Users/*/.espressif/tools/**/bin/*-elf-gcc`. **On Linux change
`/Users/*` to `/home/*`.** Without it, clangd falls back to the host compiler and
reports spurious "file not found" on `stdio.h` across the whole project.

Note the glob must be `*-elf-gcc`, not `*-esp-elf-gcc` — the binary is named
`xtensa-esp32s3-elf-gcc`, with the chip baked into the name.

---

## 11. Gotchas, collected

1. **`idf.py reconfigure` after touching `squareline_ui/`** — otherwise your UI silently
   isn't compiled. Most likely thing to waste your afternoon.
2. **Placeholder GPIOs** in `board_config.h` must be replaced before anything works.
3. **Fonts must be enabled in menuconfig** or SquareLine exports fail to link.
4. **`REQUIRES lvgl` does not work** — managed components are namespaced `lvgl__lvgl`.
   Declare dependencies in `idf_component.yml` instead; that's what adds them to
   `REQUIRES`. Already set up correctly here.
5. **All LVGL calls need `lvgl_port_lock()`** outside the LVGL task.
6. **`swap_bytes = true` for SPI panels, `false` for RGB parallel.** Already correct
   per-branch in `display.c`; only matters if you add a bus type.
7. **macOS `/usr/bin/{nm,cc,make,git,...}` are Xcode stubs.** If CLT isn't installed,
   running any of them pops an install dialog. Use the toolchain's
   `xtensa-esp-elf-nm` instead. Harmless, just noisy.
