// hello-gui.hc — hica Dear ImGui demo
//
// Demonstrates the widget set:
//   Text:    gui_text, gui_text_colored, gui_text_wrapped, gui_bullet_text
//   Input:   gui_button, gui_checkbox, gui_input_text, gui_input_int, gui_input_float
//   Sliders: gui_slider_int, gui_slider_float, gui_drag_int, gui_drag_float
//   Layout:  gui_separator, gui_same_line, gui_spacing, gui_new_line,
//            gui_dummy, gui_indent, gui_unindent
//   Panel:   gui_begin_panel / gui_end_panel
//
// Build and run (from repo root):
//   ./build.sh                         # build libimgui_hica.a (once)
//   hica run examples/hello-gui/hello-gui.hc

import "../../src/imgui"

fun main() {
  // --- counter section ---
  var count   = 0
  var enabled = false

  // --- sliders / drag section ---
  var volume  = 50
  var zoom    = 1.0
  var score   = 0
  var speed   = 0.5

  // --- input fields section ---
  var name    = ""
  var age     = 25
  var rate    = 1.0

  gui_window("hica -- Dear ImGui demo", 560, 620, () => {
    // ── Header ────────────────────────────────────────────────────────────
    gui_text_colored("hica GUI demo", 0.0, 0.749, 0.647, 1.0)
    gui_text_wrapped("All widgets are demonstrated below. Resize the window to see text wrapping in action.")
    gui_separator()
    gui_spacing()

    // ── Counter ───────────────────────────────────────────────────────────
    gui_text("Counter: " + show(count))
    if gui_button("+ Increment") {
      count = count + 1
    }
    gui_same_line()
    if gui_button("Reset") {
      count = 0
    }
    gui_spacing()

    // ── Toggle ────────────────────────────────────────────────────────────
    enabled = gui_checkbox("Enable feature", enabled)
    if enabled {
      gui_indent()
      gui_text("Feature is ON")
      gui_unindent()
    }
    gui_spacing()

    // ── Sliders & drag ────────────────────────────────────────────────────
    if gui_begin_panel("Sliders & Drag") {
      volume = gui_slider_int("Volume",   0,    100,  volume)
      zoom   = gui_slider_float("Zoom",   0.1,  4.0,  zoom)
      score  = gui_drag_int("Score",      0,    9999, score, 2.0)
      speed  = gui_drag_float("Speed",    0.0,  2.0,  speed, 0.005)
      gui_spacing()
      gui_bullet_text("Volume: " + show(volume))
      gui_bullet_text("Zoom:   " + show(zoom))
      gui_bullet_text("Score:  " + show(score))
      gui_bullet_text("Speed:  " + show(speed))
      gui_end_panel()
    }

    // ── Input fields ──────────────────────────────────────────────────────
    if gui_begin_panel("Input Fields") {
      name = gui_input_text("Name",  64)
      age  = gui_input_int("Age",    age)
      rate = gui_input_float("Rate", rate)
      if name != "" {
        gui_spacing()
        gui_text("Hello, " + name + "! Age: " + show(age))
      }
      gui_end_panel()
    }

    // ── Layout demo ───────────────────────────────────────────────────────
    if gui_begin_panel("Layout helpers") {
      gui_text("Three buttons on one line:")
      gui_spacing()
      if gui_button("Alpha")   { }
      gui_same_line()
      if gui_button("Beta")    { }
      gui_same_line()
      if gui_button("Gamma")   { }
      gui_new_line()
      gui_dummy(0.0, 6.0)
      gui_indent()
      gui_text("Indented row")
      gui_indent()
      gui_text("Double-indented row")
      gui_unindent()
      gui_unindent()
      gui_end_panel()
    }

    // ── About ─────────────────────────────────────────────────────────────
    if gui_begin_panel("About") {
      gui_text("hica — a compiled language that targets Koka.")
      gui_text("This GUI layer uses Dear ImGui via the C FFI.")
      gui_end_panel()
    }
  })
}
