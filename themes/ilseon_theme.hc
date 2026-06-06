// Ilseon Dark Theme
//
// An OLED-focused dark palette with a teal accent (#00BFA5).
// Call apply_ilseon_theme() at the top of your gui_window callback.
//
// Palette:
//   text    #E8E8EC  (near-white)
//   bg      #121217  (OLED near-black)
//   surface #1C1C24  (panel / frame background)
//   border  #5E6D7E  (slate-blue, subtle)
//   accent  #00BFA5  (TealAccent)

import "../src/imgui"

pub fun apply_ilseon_theme() {
  gui_set_color_text(0.910, 0.910, 0.925)
  gui_set_color_bg(0.071, 0.071, 0.090)
  gui_set_color_surface(0.110, 0.110, 0.141)
  gui_set_color_border(0.369, 0.427, 0.494)
  gui_set_color_accent(0.000, 0.749, 0.647)
  gui_set_style_rounding(6.0, 4.0, 4.0)
  gui_set_style_padding(8.0, 4.0)
}
