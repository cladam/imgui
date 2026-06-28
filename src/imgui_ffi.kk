// imgui_ffi.kk — raw C FFI declarations for the hica Dear ImGui binding
//
// This file contains only `pub extern` declarations bridging to the C backend.
// Do not import this directly — import the hica wrapper instead:
//
//   import "../../src/imgui"
//
// This source file is part of the hica open source project
// Copyright (C) 2026 Claes Adamsson <claes.adamsson@gmail.com>
// See https://github.com/cladam/hica/blob/main/LICENSE for license information

module imgui_ffi

import std/num/float64

// ---------------------------------------------------------------------------
// C backend — compiled by Koka alongside this module
// ---------------------------------------------------------------------------

extern import
  c file "../backend/kk_imgui.c"

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

pub extern gui_init( title : string, w : int, h : int ) : io ()
  c "kk_hk_gui_init"

pub extern gui_begin_frame() : io bool
  c "kk_hk_gui_begin_frame"

pub extern gui_end_frame() : io ()
  c "kk_hk_gui_end_frame"

pub extern gui_shutdown() : io ()
  c "kk_hk_gui_shutdown"

// ---------------------------------------------------------------------------
// Widgets
// ---------------------------------------------------------------------------

pub extern gui_text( s : string ) : io ()
  c "kk_hk_gui_text"

pub extern gui_text_colored( s : string, r : float64, g : float64, b : float64, a : float64 ) : io ()
  c "kk_hk_gui_text_colored"

pub extern gui_text_wrapped( s : string ) : io ()
  c "kk_hk_gui_text_wrapped"

pub extern gui_bullet_text( s : string ) : io ()
  c "kk_hk_gui_bullet_text"

pub extern gui_button( label : string ) : io bool
  c "kk_hk_gui_button"

pub extern gui_checkbox( label : string, checked : bool ) : io bool
  c "kk_hk_gui_checkbox"

pub extern gui_slider_int( label : string, min_val : int, max_val : int, def_val : int ) : io int
  c "kk_hk_gui_slider_int"

pub extern gui_slider_float( label : string, min_val : float64, max_val : float64, def_val : float64 ) : io float64
  c "kk_hk_gui_slider_float"

pub extern gui_input_text( label : string, capacity : int ) : io string
  c "kk_hk_gui_input_text"

pub extern gui_input_int( label : string, def_val : int ) : io int
  c "kk_hk_gui_input_int"

pub extern gui_input_float( label : string, def_val : float64 ) : io float64
  c "kk_hk_gui_input_float"

pub extern gui_drag_int( label : string, min_val : int, max_val : int, def_val : int, speed : float64 ) : io int
  c "kk_hk_gui_drag_int"

pub extern gui_drag_float( label : string, min_val : float64, max_val : float64, def_val : float64, speed : float64 ) : io float64
  c "kk_hk_gui_drag_float"

pub extern gui_separator() : io ()
  c "kk_hk_gui_separator"

pub extern gui_same_line() : io ()
  c "kk_hk_gui_same_line"

pub extern gui_spacing() : io ()
  c "kk_hk_gui_spacing"

pub extern gui_new_line() : io ()
  c "kk_hk_gui_new_line"

pub extern gui_dummy( w : float64, h : float64 ) : io ()
  c "kk_hk_gui_dummy"

pub extern gui_indent() : io ()
  c "kk_hk_gui_indent"

pub extern gui_unindent() : io ()
  c "kk_hk_gui_unindent"

pub extern gui_begin_panel( label : string ) : io bool
  c "kk_hk_gui_begin_panel"

pub extern gui_end_panel() : io ()
  c "kk_hk_gui_end_panel"

// ---------------------------------------------------------------------------
// Interactive — selection
// ---------------------------------------------------------------------------

pub extern gui_radio_button( label : string, active : bool ) : io bool
  c "kk_hk_gui_radio_button"

pub extern gui_selectable( label : string, selected : bool ) : io bool
  c "kk_hk_gui_selectable"

// w=0.0 fills available width; h=0.0 fills available height.
// Always call gui_end_child() after this, even when it returns false.
pub extern gui_begin_child( id : string, w : float64, h : float64 ) : io bool
  c "kk_hk_gui_begin_child"

pub extern gui_end_child() : io ()
  c "kk_hk_gui_end_child"

// items: newline-separated list of options, e.g. "Apple\nBanana\nCherry"
pub extern gui_combo( label : string, items : string, def_index : int ) : io int
  c "kk_hk_gui_combo"

// ---------------------------------------------------------------------------
// Display
// ---------------------------------------------------------------------------

// fraction: 0.0–1.0 fill level; overlay: text drawn over the bar ("" = default %)
pub extern gui_progress_bar( fraction : float64, overlay : string ) : io ()
  c "kk_hk_gui_progress_bar"

// ---------------------------------------------------------------------------
// Layout — groups and item width
// ---------------------------------------------------------------------------

// Start a group: SameLine treats the whole group as one item.
pub extern gui_begin_group() : io ()
  c "kk_hk_gui_begin_group"

pub extern gui_end_group() : io ()
  c "kk_hk_gui_end_group"

// Push a fixed pixel width for the next input/slider/combo widget only.
pub extern gui_push_item_width( w : float64 ) : io ()
  c "kk_hk_gui_push_item_width"

pub extern gui_pop_item_width() : io ()
  c "kk_hk_gui_pop_item_width"

// ---------------------------------------------------------------------------
// Tree nodes
// ---------------------------------------------------------------------------

// Collapsible tree node.  Returns true when expanded.
// Caller must call gui_tree_pop() only when this returns true.
pub extern gui_tree_node( label : string ) : io bool
  c "kk_hk_gui_tree_node"

pub extern gui_tree_pop() : io ()
  c "kk_hk_gui_tree_pop"

// ---------------------------------------------------------------------------
// Tab bars
// ---------------------------------------------------------------------------

// Begin a tab bar.  id must be unique within the window.
// Call gui_end_tab_bar() only when this returns true.
pub extern gui_begin_tab_bar( id : string ) : io bool
  c "kk_hk_gui_begin_tab_bar"

pub extern gui_end_tab_bar() : io ()
  c "kk_hk_gui_end_tab_bar"

// Begin a tab item inside a tab bar.  Returns true when this tab is active.
// Call gui_end_tab_item() only when this returns true.
pub extern gui_begin_tab_item( label : string ) : io bool
  c "kk_hk_gui_begin_tab_item"

pub extern gui_end_tab_item() : io ()
  c "kk_hk_gui_end_tab_item"

// ---------------------------------------------------------------------------
// Tooltips
// ---------------------------------------------------------------------------

// Show a plain-text tooltip when the previous widget is hovered.
pub extern gui_set_tooltip( text : string ) : io ()
  c "kk_hk_gui_set_tooltip"

// Begin a custom-content tooltip region (always returns true).
pub extern gui_begin_tooltip() : io bool
  c "kk_hk_gui_begin_tooltip"

pub extern gui_end_tooltip() : io ()
  c "kk_hk_gui_end_tooltip"

// ---------------------------------------------------------------------------
// Popups
// ---------------------------------------------------------------------------

// Schedule a popup to open next frame.  id must match begin_popup's id.
pub extern gui_open_popup( id : string ) : io ()
  c "kk_hk_gui_open_popup"

// Returns true when the popup is open.  Call gui_end_popup() only then.
pub extern gui_begin_popup( id : string ) : io bool
  c "kk_hk_gui_begin_popup"

// Blocking modal popup.  Returns true when open.  Call gui_end_popup() only then.
pub extern gui_begin_popup_modal( id : string ) : io bool
  c "kk_hk_gui_begin_popup_modal"

// Close the current popup from within its begin/end block.
pub extern gui_close_popup() : io ()
  c "kk_hk_gui_close_popup"

pub extern gui_end_popup() : io ()
  c "kk_hk_gui_end_popup"

// ---------------------------------------------------------------------------
// Menu bar
// ---------------------------------------------------------------------------

// Begin the main menu bar.  Call gui_end_main_menu_bar() only when true.
pub extern gui_begin_main_menu_bar() : io bool
  c "kk_hk_gui_begin_main_menu_bar"

pub extern gui_end_main_menu_bar() : io ()
  c "kk_hk_gui_end_main_menu_bar"

// Begin a drop-down menu.  Call gui_end_menu() only when true.
pub extern gui_begin_menu( label : string ) : io bool
  c "kk_hk_gui_begin_menu"

pub extern gui_end_menu() : io ()
  c "kk_hk_gui_end_menu"

// Returns true on the frame the item is clicked.
pub extern gui_menu_item( label : string ) : io bool
  c "kk_hk_gui_menu_item"

// ---------------------------------------------------------------------------
// Tables
// ---------------------------------------------------------------------------

// Common flag values (can be combined with +):
//   0                        — plain table, no decorations
//   1  (borders inner)       — inner cell borders
//   2  (borders outer)       — outer border
//   3  (borders)             — both
//   64 (row bg)              — alternating row background
//   HK_TABLE_RESIZABLE etc. are defined in imgui_glue.h for C use;
//   in hica just pass the integer literal that matches your needs.

// Begin a table.  columns: number of columns.  flags: 0 = defaults.
// Call gui_end_table() only when this returns true.
pub extern gui_begin_table( id : string, columns : int, flags : int ) : io bool
  c "kk_hk_gui_begin_table"

pub extern gui_end_table() : io ()
  c "kk_hk_gui_end_table"

// Set a column header label.  Call once per column right after begin_table.
pub extern gui_table_setup_column( label : string ) : io ()
  c "kk_hk_gui_table_setup_column"

// Emit the header row built from gui_table_setup_column calls.
pub extern gui_table_headers_row() : io ()
  c "kk_hk_gui_table_headers_row"

// Advance to the next row.
pub extern gui_table_next_row() : io ()
  c "kk_hk_gui_table_next_row"

// Advance to the next column cell within the current row.
pub extern gui_table_next_column() : io ()
  c "kk_hk_gui_table_next_column"

// ---------------------------------------------------------------------------
// Theme — runtime color and geometry overrides
// ---------------------------------------------------------------------------

// Set text and disabled-text colors (disabled = base at 55% opacity).
pub extern gui_raw_set_color_text( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_text"

// Set window, child, panel, tab, and button-rest backgrounds.
pub extern gui_raw_set_color_bg( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_bg"

// Set input fields, frames, and popups.
pub extern gui_raw_set_color_surface( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_surface"

// Set borders, separators, and scrollbar track.
pub extern gui_raw_set_color_border( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_border"

// Set the primary accent hue; hover/active/dim variants are derived automatically.
pub extern gui_raw_set_color_accent( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_accent"

// Set window, frame, and tab corner rounding (pixels).
pub extern gui_raw_set_style_rounding( window : float64, frame : float64, tab : float64 ) : io ()
  c "kk_hk_gui_set_style_rounding"

// Set frame padding (pixels).
pub extern gui_raw_set_style_padding( frame_x : float64, frame_y : float64 ) : io ()
  c "kk_hk_gui_set_style_padding"

// Solid coloured square; returns true when clicked.
pub extern gui_raw_color_button( id : string, r : float64, g : float64, b : float64, a : float64, w : float64, h : float64 ) : io bool
  c "kk_hk_gui_color_button"

// Copy text to the OS clipboard.
pub extern gui_raw_set_clipboard( text : string ) : io ()
  c "kk_hk_gui_set_clipboard"

// Plot line color; hover variant is derived (15% brighter).
pub extern gui_raw_set_color_plot( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_plot"

// Plot bar/histogram color; hover variant is derived (15% brighter).
pub extern gui_raw_set_color_plot_bar( r : float64, g : float64, b : float64 ) : io ()
  c "kk_hk_gui_set_color_plot_bar"

// Modal and nav-windowing overlay dim opacity (0=transparent, 1=opaque).
pub extern gui_raw_set_color_modal_dim( alpha : float64 ) : io ()
  c "kk_hk_gui_set_color_modal_dim"

// Window content area padding (pixels).
pub extern gui_raw_set_style_window_padding( x : float64, y : float64 ) : io ()
  c "kk_hk_gui_set_style_window_padding"

// Item spacing and tree-indent width (pixels).
pub extern gui_raw_set_style_spacing( item_x : float64, item_y : float64, indent : float64 ) : io ()
  c "kk_hk_gui_set_style_spacing"

// Window and frame border thickness (0=none, 1=thin).
pub extern gui_raw_set_style_borders( window : float64, frame : float64 ) : io ()
  c "kk_hk_gui_set_style_borders"

// ---------------------------------------------------------------------------
// ImPlot — 2-D plotting
//
// Axis constants (pass to plot_setup_axis_limits).
// These match implot's ImAxis enum exactly:
//   X1=0 (bottom, always enabled)   X2=1 (top, disabled)    X3=2 (disabled)
//   Y1=3 (left,   always enabled)   Y2=4 (right, disabled)  Y3=5 (disabled)
//
// Condition constants (pass to plot_setup_axis_limits):
//   0 = always (lock the axis range every frame)
//   1 = once   (set range only on first frame; user can then pan/zoom)
// ---------------------------------------------------------------------------

// Axis identifiers — use these instead of raw integers.
pub val plot-axis-x1 : int = 0
pub val plot-axis-x2 : int = 1
pub val plot-axis-x3 : int = 2
pub val plot-axis-y1 : int = 3
pub val plot-axis-y2 : int = 4
pub val plot-axis-y3 : int = 5

// Condition identifiers for plot_setup_axis_limits.
pub val plot-cond-always : int = 0
pub val plot-cond-once   : int = 1

// Append a y-value to the named series (x auto-increments from 0).
pub extern plot_push( label : string, y : float64 ) : io ()
  c "kk_hk_plot_push"

// Append an explicit (x, y) pair to the named series.
pub extern plot_push_xy( label : string, x : float64, y : float64 ) : io ()
  c "kk_hk_plot_push_xy"

// Remove all data points from the named series.
pub extern plot_clear( label : string ) : io ()
  c "kk_hk_plot_clear"

// Remove all data points from every series.
pub extern plot_clear_all() : io ()
  c "kk_hk_plot_clear_all"

// Begin a plot region.  w=0 fills available width; h=0 fills available height.
// Returns true when visible; call plot_end() ONLY when this returns true.
pub extern plot_begin( title : string, w : float64, h : float64 ) : io bool
  c "kk_hk_plot_begin"

// End a plot region opened by plot_begin.  Call only when plot_begin returned true.
pub extern plot_end() : io ()
  c "kk_hk_plot_end"

// Set axis labels.  Call inside a plot_begin/plot_end block.
pub extern plot_setup_axes( x_label : string, y_label : string ) : io ()
  c "kk_hk_plot_setup_axes"

// Set an explicit axis range.
// axis: 0=X1, 1=Y1, 2=X2, 3=Y2.  cond: 0=always, 1=once (first frame only).
pub extern plot_setup_axis_limits( axis : int, v_min : float64, v_max : float64, cond : int ) : io ()
  c "kk_hk_plot_setup_axis_limits"

// Draw a line plot from the named series.  Call inside a plot_begin/end block.
pub extern plot_line( label : string ) : io ()
  c "kk_hk_plot_line"

// Draw a shaded area from the named series down to y_ref (typically 0.0).
pub extern plot_shaded( label : string, y_ref : float64 ) : io ()
  c "kk_hk_plot_shaded"

// Draw a bar chart from the named series.  bar_width in axis units (0.67 typical).
pub extern plot_bars( label : string, bar_width : float64 ) : io ()
  c "kk_hk_plot_bars"

// Draw a scatter plot from the named series.
pub extern plot_scatter( label : string ) : io ()
  c "kk_hk_plot_scatter"

// Draw a pie chart.
// labels_nl  : newline-separated segment labels  e.g. "Apple\nBanana\nCherry"
// values_csv : comma-separated segment sizes     e.g. "30,50,20"
// cx, cy     : centre in normalised [0,1] plot coordinates
// radius     : radius in normalised [0,1] units
pub extern plot_pie_chart( labels_nl : string, values_csv : string, cx : float64, cy : float64, radius : float64 ) : io ()
  c "kk_hk_plot_pie_chart"

// Draw a stairstep (step-line) plot from the named series.
pub extern plot_stairs( label : string ) : io ()
  c "kk_hk_plot_stairs"

// Draw a stem plot — vertical lines from ref to each y-value in the named series.
pub extern plot_stems( label : string, ref : float64 ) : io ()
  c "kk_hk_plot_stems"

// Draw a histogram of the y-values in the named series.
// bins: number of bins; pass 0 for auto (Sturges' rule).
pub extern plot_histogram( label : string, bins : int ) : io ()
  c "kk_hk_plot_histogram"

// Draw a 2-D heatmap.
// values_csv : row-major comma-separated grid values, e.g. "1,2,3,4,5,6"
// rows, cols : grid dimensions
// scale_min/max : colour-scale range; pass equal values for auto-scale
pub extern plot_heatmap( label : string, values_csv : string, rows : int, cols : int, scale_min : float64, scale_max : float64 ) : io ()
  c "kk_hk_plot_heatmap"

// Draw infinite vertical lines at the x-positions stored in the named series.
pub extern plot_inf_lines( label : string ) : io ()
  c "kk_hk_plot_inf_lines"

// Open a URL in the system default browser.
// Uses SDL_OpenURL (SDL 2.0.14+): open on macOS, xdg-open on Linux, ShellExecute on Windows.
pub extern gui_open_url( url : string ) : io ()
  c "kk_hk_gui_open_url"
