// hello-gui.hc — hica Dear ImGui demo
//
// Demonstrates the full Phase 1 widget set:
//   - gui_window     open window + event loop
//   - gui_text       static label
//   - gui_button     click returns true on the click frame
//   - gui_checkbox   toggle with persisted state
//   - gui_slider_int integer slider
//   - gui_input_text single-line text input
//   - gui_separator  horizontal rule
//   - gui_same_line  widgets on the same line
//   - gui_spacing    extra vertical gap
//   - gui_begin_panel / gui_end_panel  collapsible section
//
// Build and run (from repo root):
//   ./build.sh                         # build libimgui_hica.a (once)
//   hica build examples/hello-gui/hello-gui.hc
//   examples/hello-gui/hello-gui
//
// Or one-shot:
//   hica run examples/hello-gui/hello-gui.hc

extern import "../../src/imgui"

fun main() {
  var count = 0
  var enabled = false
  var volume = 50
  var name = ""

  gui_window("hica -- Dear ImGui demo", 520, 480, () => {
    // --- Header ---
    gui_text("Welcome to hica GUI!")
    gui_text("Built with Dear ImGui + SDL2 + OpenGL3.")
    gui_separator()
    gui_spacing()

    // --- Counter ---
    gui_text("Counter: " + show(count))
    if gui_button("+ Increment") {
      count = count + 1
    }
    gui_same_line()
    if gui_button("Reset") {
      count = 0
    }
    gui_spacing()

    // --- Toggle ---
    enabled = gui_checkbox("Enable feature", enabled)
    if enabled {
      gui_text("  Feature is ON")
    }
    gui_spacing()

    // --- Slider ---
    volume = gui_slider_int("Volume", 0, 100, volume)
    gui_text("  Volume: " + show(volume))
    gui_spacing()

    // --- Text input ---
    name = gui_input_text("Your name", 128)
    if name != "" {
      gui_text("Hello, " + name + "!")
    }
    gui_spacing()

    // --- Collapsible section ---
    if gui_begin_panel("About") {
      gui_text("hica -- a compiled language that targets Koka.")
      gui_text("This GUI layer uses Dear ImGui via the C FFI.")
      gui_end_panel()
    }
  })
}
