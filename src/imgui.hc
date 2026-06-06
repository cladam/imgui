// imgui.hc — hica API for Dear ImGui
//
// Import this file from your program:
//   import "../../src/imgui"
//
// All pub extern functions from the C backend are re-exported transparently
// via the extern import below. The only addition here is gui_window, which
// wraps the lifecycle loop so callers don't need to manage gui_init /
// gui_begin_frame / gui_end_frame / gui_shutdown manually.

extern import "imgui_ffi"

// Opens a window and runs the frame callback each vsync tick (~60 fps).
// Closes when the user dismisses the window.
//
//   gui_window("My App", 800, 600, () => {
//     gui_text("Hello!")
//   })
pub fun gui_window(title: string, w: int, h: int, frame: () -> ()) {
  gui_init(title, w, h)
  while gui_begin_frame() {
    frame()
    gui_end_frame()
  }
  gui_shutdown()
}

// Scrollable child region.  Always calls gui_end_child() regardless of
// whether the region is visible, so content is always safe to render.
//
//   gui_child("##list", 0.0, 200.0, () => {
//     gui_text("inside the scrollable region")
//   })
//
// w=0.0 fills available width; h=0.0 fills available height.
pub fun gui_child(id: string, w: float, h: float, content: () -> ()) {
  if gui_begin_child(id, w, h) { }
  content()
  gui_end_child()
}
