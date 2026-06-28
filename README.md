# imgui

A Dear ImGui binding for [hica](https://github.com/cladam/hica). Immediate-mode GUI backed by [Dear ImGui](https://github.com/ocornut/imgui) + SDL2 + OpenGL3, with the Inter font and the Ilseon dark theme bundled out of the box. Includes [ImPlot](https://github.com/epezent/implot) for 2-D plotting.

## Installation

### 1. Install SDL2

```sh
brew install sdl2                   # macOS
# sudo apt install libsdl2-dev      # Linux
```

### 2. Add the package

```sh
hica add imgui
hica fetch
```

This records the dependency in `hica.hml` and downloads the package.

### 3. Download the pre-built static library

`hica fetch` downloads the hica source files; the compiled C++ backend must be obtained separately (it contains native code that cannot be distributed as plain hica):

```sh
mkdir -p vendor/imgui/lib

# macOS Apple Silicon:
curl -L https://github.com/cladam/imgui/releases/latest/download/libimgui_hica-macos-arm64.a \
     -o vendor/imgui/lib/libimgui_hica.a

# Linux x86-64:
# curl -L https://github.com/cladam/imgui/releases/latest/download/libimgui_hica-linux-x86_64.a \
#      -o vendor/imgui/lib/libimgui_hica.a
```

Alternatively, build from source (requires clang++ and SDL2 dev headers):

```sh
cd vendor/imgui && ./build.sh
```

### 4. Configure `hica.hml`

Add the linker flags to your project's `hica.hml`. Run `sdl2-config --libs` to find the SDL2 lib path on your machine.

**macOS:**
```hml
@koka {
    flags: "--cclinkopts=-L./vendor/imgui/lib --cclinkopts=-L/opt/homebrew/opt/sdl2/lib --cclib=imgui_hica --cclib=SDL2 --cclinkopts=-lc++ --cclinkopts=-framework --cclinkopts=OpenGL"
}
```

**Linux:**
```hml
@koka {
    flags: "--cclinkopts=-L./vendor/imgui/lib --cclib=imgui_hica --cclib=SDL2 --cclib=GL"
}
```

### 5. Import

```hica
import "imgui"
```

## Quick start

```hica
import "imgui"

fun main() {
  var count = 0

  gui_window("My App", 520, 360, () => {
    gui_text("Counter: " + show(count))
    if gui_button("Increment") {
      count = count + 1
    }
  })
}
```

`gui_window` opens the OS window, runs the event loop, and calls your lambda every frame. Close the window to exit.

## API

### Window

| Function | Signature | Purpose |
|----------|-----------|---------|
| `gui_window` | `(title: string, w: int, h: int, frame: () -> ()) -> ()` | Open window and run the frame loop |

`gui_window` handles `gui_init`, `gui_begin_frame` / `gui_end_frame`, and `gui_shutdown` for you. Call the low-level lifecycle functions directly only if you need custom control.

### Lifecycle (low-level)

| Function | Signature | Purpose |
|----------|-----------|---------|
| `gui_init` | `(title: string, w: int, h: int) -> ()` | Open window, init SDL2 + OpenGL + ImGui |
| `gui_begin_frame` | `() -> bool` | Start frame; returns `false` when window is closed |
| `gui_end_frame` | `() -> ()` | Render and swap buffers |
| `gui_shutdown` | `() -> ()` | Tear down ImGui, SDL2, OpenGL |

### Widgets

All widgets must be called between `gui_begin_frame()` and `gui_end_frame()` — or inside the `gui_window` lambda.

| Function | Signature | Purpose |
|----------|-----------|------|
| `gui_text` | `(s: string) -> ()` | Static text label |
| `gui_text_colored` | `(s: string, r: float, g: float, b: float, a: float) -> ()` | Text in an RGBA colour (channels 0.0–1.0) |
| `gui_text_wrapped` | `(s: string) -> ()` | Text that wraps at the window edge |
| `gui_bullet_text` | `(s: string) -> ()` | Bullet point + text |
| `gui_button` | `(label: string) -> bool` | Button; returns `true` on the click frame |
| `gui_checkbox` | `(label: string, checked: bool) -> bool` | Toggle; returns current state |
| `gui_slider_int` | `(label: string, min: int, max: int, default: int) -> int` | Integer slider; returns current value |
| `gui_slider_float` | `(label: string, min: float, max: float, default: float) -> float` | Float slider; returns current value |
| `gui_drag_int` | `(label: string, min: int, max: int, default: int, speed: float) -> int` | Click-drag int control; `speed` ≈ change per pixel |
| `gui_drag_float` | `(label: string, min: float, max: float, default: float, speed: float) -> float` | Click-drag float control |
| `gui_input_text` | `(label: string, capacity: int) -> string` | Single-line text input; returns current content |
| `gui_input_int` | `(label: string, default: int) -> int` | Integer input field with +/- buttons |
| `gui_input_float` | `(label: string, default: float) -> float` | Float input field |
| `gui_radio_button` | `(label: string, active: bool) -> bool` | Radio button; returns `true` when clicked (caller manages which is selected) |
| `gui_selectable` | `(label: string, selected: bool) -> bool` | Clickable row; returns current selected state (widget owns state) |
| `gui_combo` | `(label: string, items: string, default: int) -> int` | Dropdown; `items` is newline-separated (`"A\nB\nC"`); returns selected index |
| `gui_progress_bar` | `(fraction: float, overlay: string) -> ()` | Horizontal bar filled 0.0–1.0; `overlay=""` shows default `XX%` label |

### Layout helpers

| Function | Signature | Purpose |
|----------|-----------|------|
| `gui_separator` | `() -> ()` | Horizontal rule |
| `gui_same_line` | `() -> ()` | Place next widget on the same line |
| `gui_new_line` | `() -> ()` | Undo a `same_line`; move to the next line |
| `gui_spacing` | `() -> ()` | Extra vertical gap |
| `gui_dummy` | `(w: float, h: float) -> ()` | Invisible spacer of exact pixel size |
| `gui_indent` | `() -> ()` | Push indent (shift subsequent widgets right) |
| `gui_unindent` | `() -> ()` | Pop indent |

### Panels

| Function | Signature | Purpose |
|----------|-----------|---------|
| `gui_begin_panel` | `(label: string) -> bool` | Start a collapsible section; returns `true` when expanded |
| `gui_end_panel` | `() -> ()` | End the collapsible section |

Call `gui_end_panel()` only when `gui_begin_panel()` returns `true`:

```rust
if gui_begin_panel("Settings") {
  gui_slider_int("Volume", 0, 100, 80)
  gui_end_panel()
}
```

### Containers & structure

| Helper | Signature | Purpose |
|--------|-----------|---------|
| `gui_child` | `(id: string, w: float, h: float, content: () -> ()) -> ()` | Scrollable child region |
| `gui_tree` | `(label: string, content: () -> ()) -> ()` | Collapsible tree node |
| `gui_with_item_width` | `(w: float, content: () -> ()) -> ()` | Constrain next widget width in pixels |
| `gui_tab_bar` | `(id: string, content: () -> ()) -> ()` | Tab bar container |
| `gui_tab` | `(label: string, content: () -> ()) -> ()` | One tab inside a `gui_tab_bar` |

### Tooltips

| Function | Signature | Purpose |
|----------|-----------|---------|
| `gui_tooltip` | `(text: string) -> ()` | Show a text tooltip when the previous widget is hovered |
| `gui_set_tooltip` | `(text: string) -> ()` | Raw version (same behaviour) |
| `gui_begin_tooltip` | `() -> bool` | Start a custom-content tooltip region |
| `gui_end_tooltip` | `() -> ()` | End the tooltip region |

### Popups

| Helper | Signature | Purpose |
|--------|-----------|---------|
| `gui_open_popup` | `(id: string) -> ()` | Schedule a popup to open next frame |
| `gui_popup` | `(id: string, content: () -> ()) -> ()` | Non-blocking popup window |
| `gui_modal` | `(id: string, content: () -> ()) -> ()` | Blocking modal dialog |
| `gui_close_popup` | `() -> ()` | Dismiss the current popup from within its block |

```rust
if gui_button("Options") { gui_open_popup("opts") }
gui_popup("opts", () => {
  if gui_menu_item("Reset") { ... }
  if gui_button("Close")   { gui_close_popup() }
})
```

### Menu bar

| Helper | Signature | Purpose |
|--------|-----------|---------|
| `gui_main_menu` | `(content: () -> ()) -> ()` | Top-level OS menu bar |
| `gui_menu` | `(label: string, content: () -> ()) -> ()` | Drop-down menu inside a bar or another menu |
| `gui_menu_item` | `(label: string) -> bool` | Clickable item; returns `true` when clicked |

```rust
gui_main_menu(() => {
  gui_menu("File", () => {
    if gui_menu_item("Save") { save() }
    if gui_menu_item("Quit") { }
  })
})
```

### Tables

| Function | Signature | Purpose |
|----------|-----------|---------|
| `gui_begin_table` | `(id: string, columns: int, flags: int) -> bool` | Start a table; returns `true` when visible |
| `gui_end_table` | `() -> ()` | End the table |
| `gui_table_setup_column` | `(label: string) -> ()` | Define a column header label |
| `gui_table_headers_row` | `() -> ()` | Emit the header row from setup columns |
| `gui_table_next_row` | `() -> ()` | Advance to the next row |
| `gui_table_next_column` | `() -> ()` | Advance to the next cell |

**Common flag values** (combine with `+`):

| Value | Meaning |
|-------|---------|
| `0` | Plain table, no decorations |
| `1` | Inner cell borders |
| `2` | Outer border |
| `3` | Both borders |
| `64` | Alternating row background |
| `67` | Borders + alternating rows (most common) |

```rust
if gui_begin_table("##scores", 3, 67) {
  gui_table_setup_column("Player")
  gui_table_setup_column("Score")
  gui_table_setup_column("Level")
  gui_table_headers_row()
  gui_table_next_row()
  gui_table_next_column()  // cell [0,0]
  gui_text("Alice")
  gui_table_next_column()  // cell [0,1]
  gui_text("1024")
  gui_table_next_column()  // cell [0,2]
  gui_text("12")
  gui_end_table()
}
```

### Widget state

Sliders, checkboxes, and text inputs manage their own persistent state keyed by label string. Read the return value each frame to get the current value, the hica `var` bindings work naturally:

```rust
var volume = 80
var muted  = false

gui_window("Player", 400, 200, () => {
  volume = gui_slider_int("Volume", 0, 100, volume)
  muted  = gui_checkbox("Mute", muted)
  if muted {
    gui_text("  (audio muted)")
  }
})
```

## Examples

See [examples/hello-gui](examples/hello-gui/hello-gui.hc) in this repo for a full widget showcase, and [examples/chart](examples/chart/chart.hc) for ImPlot.

To run the examples from a clone of this repo:

```sh
./build.sh                                # build libimgui_hica.a (once)
hica run examples/hello-gui/hello-gui.hc
hica run examples/chart/chart.hc
```

## Building the C backend

### Requirements

**macOS:**
```sh
brew install sdl2
```

**Linux (Debian/Ubuntu):**
```sh
sudo apt install libsdl2-dev libgl-dev
```

### Build

```sh
cd lib/imgui
./build.sh
```

This compiles Dear ImGui, the SDL2 + OpenGL3 backends, the C glue layer, and archives everything into `lib/libimgui_hica.a`. The Inter font and Ilseon theme are baked in – no runtime assets needed.

## Theme and font

The bundled defaults are:

- **Font**: [Inter](https://rsms.me/inter/) Regular 16px, HiDPI-aware (scales with display pixel density)
- **Theme**: Ilseon, an OLED-focused dark palette with organic muted accents, sourced from the [hica standard library](https://github.com/cladam/hica/blob/main/stdlib/std/term.hc)

## Project structure

```
backend/
  imgui_glue.h        # Plain-C API: hk_gui_* functions
  imgui_glue.cpp      # C++ implementation — Dear ImGui + SDL2 + OpenGL3
  kk_imgui.c          # Koka FFI trampolines (compiled by Koka)
  inter_font.h        # Inter-Regular.ttf embedded as a C array
src/
  imgui.kk            # Koka module: pub extern gui_* functions + gui_window
vendor/
  imgui/              # Dear ImGui source (cloned by build.sh)
  fonts/
    Inter-Regular.ttf
examples/
tests/
build.sh              # Builds libimgui_hica.a
lib/
  libimgui_hica.a     # Pre-built static library (after ./build.sh)
```

## License

MIT
