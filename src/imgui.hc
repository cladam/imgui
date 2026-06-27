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

// ---------------------------------------------------------------------------
// Theme — runtime color and geometry overrides
//
// Call any of these at the top of your gui_window callback each frame to
// apply a live theme.  Hover/active/dim variants of accent are derived
// automatically so only the base hue is needed.
// ---------------------------------------------------------------------------

// Set text and disabled-text colors.
pub fun gui_set_color_text(r: float, g: float, b: float) {
  gui_raw_set_color_text(r, g, b)
}

// Set window, child, panel, tab, and button backgrounds + GL clear color.
pub fun gui_set_color_bg(r: float, g: float, b: float) {
  gui_raw_set_color_bg(r, g, b)
}

// Set input fields, frames, and popups.
pub fun gui_set_color_surface(r: float, g: float, b: float) {
  gui_raw_set_color_surface(r, g, b)
}

// Set borders, separators, and scrollbar track.
pub fun gui_set_color_border(r: float, g: float, b: float) {
  gui_raw_set_color_border(r, g, b)
}

// Set the primary accent hue; hover/active/dim variants are derived automatically.
pub fun gui_set_color_accent(r: float, g: float, b: float) {
  gui_raw_set_color_accent(r, g, b)
}

// Set window, frame, and tab corner rounding in pixels.
pub fun gui_set_style_rounding(window: float, frame: float, tab: float) {
  gui_raw_set_style_rounding(window, frame, tab)
}

// Set frame padding in pixels.
pub fun gui_set_style_padding(frame_x: float, frame_y: float) {
  gui_raw_set_style_padding(frame_x, frame_y)
}

// Solid coloured square; returns true when clicked.
pub fun gui_color_button(id: string, r: float, g: float, b: float, a: float, w: float, h: float) {
  gui_raw_color_button(id, r, g, b, a, w, h)
}

// Copy text to the OS clipboard.
pub fun gui_set_clipboard(text: string) {
  gui_raw_set_clipboard(text)
}

// Plot line color; hover variant is derived automatically.
pub fun gui_set_color_plot(r: float, g: float, b: float) {
  gui_raw_set_color_plot(r, g, b)
}

// Plot bar/histogram color; hover variant is derived automatically.
pub fun gui_set_color_plot_bar(r: float, g: float, b: float) {
  gui_raw_set_color_plot_bar(r, g, b)
}

// Modal and nav-windowing overlay dim opacity (0=transparent, 1=opaque).
pub fun gui_set_color_modal_dim(alpha: float) {
  gui_raw_set_color_modal_dim(alpha)
}

// Window content area padding in pixels.
pub fun gui_set_style_window_padding(x: float, y: float) {
  gui_raw_set_style_window_padding(x, y)
}

// Item spacing and tree-indent width in pixels.
pub fun gui_set_style_spacing(item_x: float, item_y: float, indent: float) {
  gui_raw_set_style_spacing(item_x, item_y, indent)
}

// Window and frame border thickness in pixels (0=none, 1=thin).
pub fun gui_set_style_borders(window: float, frame: float) {
  gui_raw_set_style_borders(window, frame)
}

// ---------------------------------------------------------------------------
// ImPlot — 2-D plotting
//
// Use plot_push / plot_push_xy to feed data into named series, then call
// gui_plot (or the lower-level plot_begin / plot_end) to render them.
//
// Example:
//
//   // update data (e.g. inside gui_window callback, each frame):
//   plot_push("CPU", cpu_percent)
//
//   // render:
//   gui_plot("##cpu_chart", 0.0, 200.0, () => {
//     plot_setup_axes("frame", "%")
//     plot_setup_axis_limits(1, 0.0, 100.0, 1)   // Y1, 0–100, set once
//     plot_shaded("CPU", 0.0)
//     plot_line("CPU")
//   })
//
// Axis constants for plot_setup_axis_limits:
//   0 = X1 (bottom)   1 = Y1 (left)   2 = X2 (top)   3 = Y2 (right)
//
// Condition constants:  0 = always lock range   1 = set once (user can pan/zoom)
// ---------------------------------------------------------------------------

// Plot region with closure API.  content() is called only when the plot is visible.
// w=0.0 fills available width; h=0.0 fills available height.
pub fun gui_plot(title: string, w: float, h: float, content: () -> ()) {
  if plot_begin(title, w, h) {
    content()
    plot_end()
  }
}
// Axis and condition constants are exported directly from imgui_ffi:
//   plot_axis_x1 = 0, plot_axis_y1 = 3  (primary axes, always enabled)
//   plot_axis_x2 = 1, plot_axis_y2 = 4  (secondary axes, disabled by default)
//   plot_cond_always = 0, plot_cond_once = 1
