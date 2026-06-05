# imgui

A Dear ImGui binding for [hica](https://github.com/cladam/hica). Immediate-mode GUI backed by [Dear ImGui](https://github.com/ocornut/imgui) + SDL2 + OpenGL3, with the Inter font and the Ilseon dark theme bundled out of the box.

## Installation

### Option A: pre-built binary (recommended)

No C++ compiler or ImGui source needed. Only SDL2 must be installed on the machine.

```sh
# 1. Install SDL2
brew install sdl2                          # macOS
# sudo apt install libsdl2-dev             # Linux

# 2. Add this repo as a submodule (brings imgui.kk + the C trampoline)
git submodule add https://github.com/cladam/imgui.git lib/imgui

# 3. Download the pre-built .a for your platform from:
#    https://github.com/cladam/imgui/releases/latest
mkdir -p lib/imgui/lib

# macOS Apple Silicon:
curl -L https://github.com/cladam/imgui/releases/latest/download/libimgui_hica-macos-arm64.a \
     -o lib/imgui/lib/libimgui_hica.a

# Linux x86-64:
# curl -L https://github.com/cladam/imgui/releases/latest/download/libimgui_hica-linux-x86_64.a \
#      -o lib/imgui/lib/libimgui_hica.a
```

### Option B — build from source

```sh
git submodule add https://github.com/cladam/imgui.git lib/imgui
cd lib/imgui && ./build.sh   # requires clang++ and SDL2 dev headers
```

### Common: import and link

Import the library from your `.hc` file:

```rust
extern import "./lib/imgui/src/imgui"
```

Configure the linker in your `hica.hml`. The SDL2 lib path depends on where SDL2 is installed, use `sdl2-config --libs` to find it:

```sh
sdl2-config --libs
# macOS Apple Silicon → -L/opt/homebrew/opt/sdl2/lib -lSDL2
# Linux               → -L/usr/lib/x86_64-linux-gnu -lSDL2
```

**macOS:**
```hml
@koka {
    include: "./lib/imgui/src"
    flags: "--cclinkopts=-L./lib/imgui/lib --cclinkopts=-L/opt/homebrew/opt/sdl2/lib --cclib=imgui_hica --cclib=SDL2 --cclinkopts=-lc++ --cclinkopts=-framework --cclinkopts=OpenGL"
}
```

**Linux:**
```hml
@koka {
    include: "./lib/imgui/src"
    flags: "--cclinkopts=-L./lib/imgui/lib --cclib=imgui_hica --cclib=SDL2 --cclib=GL"
}
```

## Quick start

```rust
extern import "./lib/imgui/src/imgui"

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
|----------|-----------|---------|
| `gui_text` | `(s: string) -> ()` | Static text label |
| `gui_button` | `(label: string) -> bool` | Button; returns `true` on the click frame |
| `gui_checkbox` | `(label: string, checked: bool) -> bool` | Toggle; returns current state |
| `gui_slider_int` | `(label: string, min: int, max: int, default: int) -> int` | Integer slider; returns current value |
| `gui_slider_float` | `(label: string, min: float, max: float, default: float) -> float` | Float slider; returns current value |
| `gui_input_text` | `(label: string, capacity: int) -> string` | Single-line text input; returns current content |
| `gui_separator` | `() -> ()` | Horizontal rule |
| `gui_same_line` | `() -> ()` | Place next widget on the same line |
| `gui_spacing` | `() -> ()` | Extra vertical gap |

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

See [examples/hello-gui](examples/hello-gui/hello-gui.hc) in this repo for a full demo. Build and run:

```sh
./build.sh                                # build libimgui_hica.a (once)
hica run examples/hello-gui/hello-gui.hc
```

```sh
cd examples/hello-gui
hica build hello-gui.hc
./hello-gui
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
- **Theme**: Ilseon — an OLED-focused dark palette with organic muted accents, sourced from the [hica standard library](https://github.com/cladam/hica/blob/main/stdlib/std/term.hc)

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
