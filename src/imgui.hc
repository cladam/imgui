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

// Collapsible tree node.  Renders children only when expanded.
//
//   gui_tree("Fruits", () => {
//     gui_bullet_text("Apple")
//     gui_bullet_text("Banana")
//   })
pub fun gui_tree(label: string, content: () -> ()) {
  if gui_tree_node(label) {
    content()
    gui_tree_pop()
  }
}

// Push item width for one widget, then restore.
//
//   gui_with_item_width(120.0, () => {
//     volume = gui_slider_int("##vol", 0, 100, volume)
//   })
pub fun gui_with_item_width(w: float, content: () -> ()) {
  gui_push_item_width(w)
  content()
  gui_pop_item_width()
}

// Tab bar with closure-style tabs.
//
//   gui_tab_bar("##tabs", () => {
//     gui_tab("Stats", () => { gui_text("stats here") })
//     gui_tab("Config", () => { gui_text("config here") })
//   })
pub fun gui_tab_bar(id: string, content: () -> ()) {
  if gui_begin_tab_bar(id) {
    content()
    gui_end_tab_bar()
  }
}

// Renders one tab inside a gui_tab_bar block.
// content() is only called when this tab is the active one.
pub fun gui_tab(label: string, content: () -> ()) {
  if gui_begin_tab_item(label) {
    content()
    gui_end_tab_item()
  }
}

// Simple text tooltip on the previous widget.
//
//   gui_button("Save")
//   gui_tooltip("Save the current file")
pub fun gui_tooltip(text: string) {
  gui_set_tooltip(text)
}

// Popup triggered by gui_open_popup(id).
//
//   if gui_button("Options") { gui_open_popup("opts") }
//   gui_popup("opts", () => {
//     if gui_menu_item("Reset") { ... }
//     if gui_menu_item("Close") { gui_close_popup() }
//   })
pub fun gui_popup(id: string, content: () -> ()) {
  if gui_begin_popup(id) {
    content()
    gui_end_popup()
  }
}

// Blocking modal dialog.
//
//   if gui_button("Confirm") { gui_open_popup("confirm") }
//   gui_modal("confirm", () => {
//     gui_text("Are you sure?")
//     if gui_button("Yes") { gui_close_popup() }
//   })
pub fun gui_modal(id: string, content: () -> ()) {
  if gui_begin_popup_modal(id) {
    content()
    gui_end_popup()
  }
}

// Main menu bar with closure-style menus.
//
//   gui_main_menu(() => {
//     gui_menu("File", () => {
//       if gui_menu_item("Quit") { ... }
//     })
//   })
pub fun gui_main_menu(content: () -> ()) {
  if gui_begin_main_menu_bar() {
    content()
    gui_end_main_menu_bar()
  }
}

// Drop-down menu inside a gui_main_menu or another gui_menu.
pub fun gui_menu(label: string, content: () -> ()) {
  if gui_begin_menu(label) {
    content()
    gui_end_menu()
  }
}

// Closure-based table.  columns defines how many columns.
// Use flags=0 for a plain table; add 3 for borders, 64 for alternating rows.
//
//   gui_table("##t", 3, 3, ["Name", "Type", "Value"], rows, (row) => {
//     -- not directly supported due to hica lambda limits;
//     -- use the raw API below instead
//   })
//
// Simple two-step pattern:
//   if gui_begin_table("##scores", 2, 3) {
//     gui_table_setup_column("Player")
//     gui_table_setup_column("Score")
//     gui_table_headers_row()
//     gui_table_next_row()
//     gui_table_next_column()  gui_text("Alice")
//     gui_table_next_column()  gui_text("42")
//     gui_end_table()
//   }
