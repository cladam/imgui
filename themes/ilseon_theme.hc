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
  // Colors — OLED dark palette
  gui_set_color_text(0.910, 0.910, 0.925)       // #E8E8EC near-white
  gui_set_color_bg(0.071, 0.071, 0.090)          // #121217 OLED near-black
  gui_set_color_surface(0.110, 0.110, 0.141)     // #1C1C24 panel / frame
  gui_set_color_border(0.369, 0.427, 0.494)      // #5E6D7E slate-blue
  gui_set_color_accent(0.000, 0.749, 0.647)      // #00BFA5 TealAccent
  gui_set_color_plot(0.353, 0.608, 0.502)        // MutedTeal
  gui_set_color_plot_bar(0.753, 0.541, 0.243)    // QuietAmber
  gui_set_color_modal_dim(0.35)
  // Geometry
  gui_set_style_rounding(6.0, 4.0, 4.0)
  gui_set_style_padding(8.0, 4.0)
  gui_set_style_window_padding(12.0, 10.0)
  gui_set_style_spacing(8.0, 6.0, 18.0)
  gui_set_style_borders(1.0, 0.0)
}
