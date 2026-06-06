// hica Light Theme
//
// The default theme applied automatically by gui_window.
// Import this file and call apply_hica_theme() at the top of your
// gui_window callback to re-apply it explicitly — useful when you want
// to switch back from another theme at runtime.
//
// Palette derived from the hica website:
//   text    #1e293b  (--primary-text)
//   bg      #E6E9EE  (window background)
//   surface #f8f9fa  (--sidebar-bg, input fields)
//   border  #e2e8f0  (--border-color)
//   accent  #4f46e5  (--accent-indigo)

import "../src/imgui"

pub fun apply_hica_theme() {
  gui_set_color_text(0.118, 0.161, 0.231)
  gui_set_color_bg(0.902, 0.914, 0.933)
  gui_set_color_surface(0.973, 0.976, 0.980)
  gui_set_color_border(0.886, 0.910, 0.941)
  gui_set_color_accent(0.310, 0.275, 0.898)
  gui_set_style_rounding(8.0, 5.0, 5.0)
  gui_set_style_padding(10.0, 5.0)
}
