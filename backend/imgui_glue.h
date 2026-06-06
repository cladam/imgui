/*
 * imgui_glue.h — plain-C API for the hica Dear ImGui backend
 *
 * This header is included by kk_imgui.c (compiled by Koka) and by
 * imgui_glue.cpp (the C++ implementation).  The C++ side wraps Dear ImGui's
 * SDL2 + OpenGL3 backends and exports every symbol with extern "C" so the
 * C side can link against them.
 *
 * Design principle: hica never sees ImGui or SDL2 types.  Everything crossing
 * the boundary is a plain-C int/float/char*.
 *
 * This source file is part of the hica open source project
 * Copyright (C) 2026 Claes Adamsson <claes.adamsson@gmail.com>
 * See https://github.com/cladam/hica/blob/main/LICENSE for license information
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* ---------------------------------------------------------------------------
 * Lifecycle — call in this order:
 *   hk_gui_init(...)
 *   while (hk_gui_begin_frame()) { ... widgets ... hk_gui_end_frame(); }
 *   hk_gui_shutdown()
 * -------------------------------------------------------------------------*/

/* Open the window and initialise SDL2 / OpenGL3 / Dear ImGui.
 * Must be called once before any other function.
 * title : window title (UTF-8)
 * w, h  : initial window size in pixels */
void hk_gui_init(const char* title, int w, int h);

/* Poll events and start a new Dear ImGui frame.
 * Returns 1 if the frame should be rendered; 0 when the window has been
 * closed (the caller should exit the loop and call hk_gui_shutdown). */
int hk_gui_begin_frame(void);

/* Finish the frame: end the root window, render ImGui draw data, swap buffers. */
void hk_gui_end_frame(void);

/* Tear down ImGui, SDL2 and OpenGL3.  Call once after the loop exits. */
void hk_gui_shutdown(void);

/* ---------------------------------------------------------------------------
 * Widgets — call only between hk_gui_begin_frame() and hk_gui_end_frame()
 * -------------------------------------------------------------------------*/

/* Display a text label. */
void hk_gui_text(const char* s);

/* Display a text label in the given RGBA colour (each channel 0.0–1.0). */
void hk_gui_text_colored(const char* s, double r, double g, double b, double a);

/* Display text that wraps at the window/column edge. */
void hk_gui_text_wrapped(const char* s);

/* Display a bullet point followed by text. */
void hk_gui_bullet_text(const char* s);

/* Display a button.  Returns 1 on the frame it was clicked, 0 otherwise. */
int hk_gui_button(const char* label);

/* Display a checkbox.  Manages its own state keyed by label.
 * def    : initial value (0 = unchecked, 1 = checked)
 * Returns the current checked state (0 or 1). */
int hk_gui_checkbox(const char* label, int def);

/* Display an integer slider in [min, max].  Manages its own state.
 * def    : initial value
 * Returns the current value. */
int hk_gui_slider_int(const char* label, int min, int max, int def);

/* Display a float slider in [min, max].  Manages its own state.
 * def    : initial value (as a double for easier FFI; truncated to float internally)
 * Returns the current value as a double. */
double hk_gui_slider_float(const char* label, double min, double max, double def);

/* Display a single-line text input.  Manages its own buffer (capacity bytes).
 * Returns a pointer to the current buffer contents (valid until next call
 * with the same label). */
const char* hk_gui_input_text(const char* label, int capacity);

/* Display an integer input field with +/- step buttons.
 * def : initial value.  Returns the current value. */
int hk_gui_input_int(const char* label, int def);

/* Display a float input field.
 * def : initial value (as double for FFI convenience).  Returns current value. */
double hk_gui_input_float(const char* label, double def);

/* Display a click-drag integer control in [min, max].
 * speed : change per pixel of drag (e.g. 1.0).  Returns current value. */
int hk_gui_drag_int(const char* label, int min, int max, int def, double speed);

/* Display a click-drag float control in [min, max].
 * speed : change per pixel of drag (e.g. 0.01).  Returns current value. */
double hk_gui_drag_float(const char* label, double min, double max, double def, double speed);

/* Horizontal separator rule. */
void hk_gui_separator(void);

/* Place the next widget on the same line as the previous one. */
void hk_gui_same_line(void);

/* Extra vertical spacing. */
void hk_gui_spacing(void);

/* Undo a SameLine — move back to the start of the next line. */
void hk_gui_new_line(void);

/* Invisible widget of the given size in pixels; useful as a spacer. */
void hk_gui_dummy(double w, double h);

/* Push indent (moves subsequent widgets right by one level). */
void hk_gui_indent(void);

/* Pop indent. */
void hk_gui_unindent(void);

/* Collapsible section header.
 * Returns 1 if the section is expanded (caller should render children and
 * call hk_gui_end_panel); returns 0 if collapsed (skip children). */
int hk_gui_begin_panel(const char* label);

/* Close a collapsible section opened by hk_gui_begin_panel.
 * Must be called only when hk_gui_begin_panel returned 1. */
void hk_gui_end_panel(void);

/* Display a radio button.  active=1 means the button appears selected.
 * Returns 1 on the frame it is clicked (caller should update the selection). */
int hk_gui_radio_button(const char* label, int active);

/* Display a selectable row.  Stateless: renders as selected when selected=1;
 * returns 1 on the frame it is clicked.  Caller owns the selection state. */
int hk_gui_selectable(const char* label, int selected);

/* Begin a scrollable child region.
 * id : unique string ID (use "##id" to hide the label).
 * w  : width in pixels; 0 = fill available width.
 * h  : height in pixels; 0 = fill available height.
 * Returns 1 always; hk_gui_end_child() must always be called. */
int hk_gui_begin_child(const char* id, double w, double h);

/* End a child region opened by hk_gui_begin_child.  Always call this. */
void hk_gui_end_child(void);

/* Display a combo/dropdown.  Manages its own state keyed by label.
 * items      : newline-separated list of item strings.
 * def_index  : initial selected index.
 * Returns the currently selected index. */
int hk_gui_combo(const char* label, const char* items, int def_index);

/* Display a progress bar filled to fraction (0.0–1.0).
 * overlay : text drawn over the bar; pass NULL for the default "XX%" label. */
void hk_gui_progress_bar(double fraction, const char* overlay);

/* Start a group: subsequent widgets share the same cursor origin so that
 * SameLine() can treat the whole group as a single item. */
void hk_gui_begin_group(void);

/* End a group opened by hk_gui_begin_group. */
void hk_gui_end_group(void);

/* Push a fixed pixel width for the next input/slider/combo widget only. */
void hk_gui_push_item_width(double w);

/* Pop the item width pushed by hk_gui_push_item_width. */
void hk_gui_pop_item_width(void);

/* Display a collapsible tree node.  Returns 1 if expanded (caller should
 * render children and call hk_gui_tree_pop); 0 if collapsed. */
int hk_gui_tree_node(const char* label);

/* Close a tree node opened by hk_gui_tree_node. */
void hk_gui_tree_pop(void);

/* Begin a tab bar.  id must be a unique string per window.
 * Returns 1 if the tab bar is visible; caller must call hk_gui_end_tab_bar()
 * only when this returns 1. */
int hk_gui_begin_tab_bar(const char* id);

/* Close a tab bar opened by hk_gui_begin_tab_bar. */
void hk_gui_end_tab_bar(void);

/* Begin a tab item within the current tab bar.
 * Returns 1 when this tab is selected; caller should render contents and
 * call hk_gui_end_tab_item() only when this returns 1. */
int hk_gui_begin_tab_item(const char* label);

/* Close a tab item opened by hk_gui_begin_tab_item. */
void hk_gui_end_tab_item(void);

/* ---------------------------------------------------------------------------
 * Tooltips
 * -------------------------------------------------------------------------*/

/* Show a plain-text tooltip on the previous widget (call after it). */
void hk_gui_set_tooltip(const char* text);

/* Begin a custom tooltip region.  Returns 1 always.
 * Call hk_gui_end_tooltip() when done. */
int  hk_gui_begin_tooltip(void);

/* End a tooltip opened by hk_gui_begin_tooltip. */
void hk_gui_end_tooltip(void);

/* ---------------------------------------------------------------------------
 * Popups
 * -------------------------------------------------------------------------*/

/* Schedule a popup to open next frame.  id must match the id passed to
 * hk_gui_begin_popup / hk_gui_begin_popup_modal. */
void hk_gui_open_popup(const char* id);

/* Begin a popup window.  Returns 1 when the popup is open.
 * Call hk_gui_end_popup() only when this returns 1. */
int  hk_gui_begin_popup(const char* id);

/* Begin a blocking modal popup.  Returns 1 when the popup is open.
 * Call hk_gui_end_popup() only when this returns 1. */
int  hk_gui_begin_popup_modal(const char* id);

/* Close the current popup (call from within begin/end_popup). */
void hk_gui_close_popup(void);

/* End a popup opened by hk_gui_begin_popup or hk_gui_begin_popup_modal. */
void hk_gui_end_popup(void);

/* ---------------------------------------------------------------------------
 * Menu bar
 * -------------------------------------------------------------------------*/

/* Begin the OS-level main menu bar at the top of the window.
 * Returns 1 when visible; call hk_gui_end_main_menu_bar() only then. */
int  hk_gui_begin_main_menu_bar(void);

/* End the main menu bar. */
void hk_gui_end_main_menu_bar(void);

/* Begin a drop-down menu inside a menu bar or another menu.
 * Returns 1 when the menu is open; call hk_gui_end_menu() only then. */
int  hk_gui_begin_menu(const char* label);

/* End a menu opened by hk_gui_begin_menu. */
void hk_gui_end_menu(void);

/* Display a menu item.  Returns 1 on the frame it is clicked. */
int  hk_gui_menu_item(const char* label);

/* ---------------------------------------------------------------------------
 * Tables
 * -------------------------------------------------------------------------*/

/* Begin a table with the given number of columns.
 * flags: 0 for defaults, or combine ImGuiTableFlags_* values.
 * Returns 1 if the table is visible; call hk_gui_end_table() only then. */
int  hk_gui_begin_table(const char* id, int columns, int flags);

/* End a table opened by hk_gui_begin_table. */
void hk_gui_end_table(void);

/* Define a column header.  Call once per column after begin_table. */
void hk_gui_table_setup_column(const char* label);

/* Emit the header row from column labels set by hk_gui_table_setup_column. */
void hk_gui_table_headers_row(void);

/* Advance to the next row.  Call before filling each row of cells. */
void hk_gui_table_next_row(void);

/* Advance to the next cell within the current row. */
void hk_gui_table_next_column(void);

/* Convenience flag constants (mirrors ImGuiTableFlags) */
#define HK_TABLE_BORDERS_INNER  0x01
#define HK_TABLE_BORDERS_OUTER  0x02
#define HK_TABLE_BORDERS        0x03
#define HK_TABLE_ROW_BG         0x40
#define HK_TABLE_RESIZABLE      0x40000
#define HK_TABLE_SIZING_STRETCH 0x60000000

/* ---------------------------------------------------------------------------
 * Theme — runtime color and geometry overrides
 *
 * Call these at the top of your gui_window callback each frame to apply a
 * live theme.  Alpha is always 1.0; hover/active/dim variants are derived
 * automatically so you only need to set the base hue.
 * -------------------------------------------------------------------------*/

/* Text + disabled text (disabled = base at 55% opacity). */
void hk_gui_set_color_text(double r, double g, double b);

/* Window, child, panel, menu bar, scrollbar, tab backgrounds.
 * ChildBg is derived as 4% darker. */
void hk_gui_set_color_bg(double r, double g, double b);

/* Input fields, frames, popups, selected tab. */
void hk_gui_set_color_surface(double r, double g, double b);

/* Border, separator, scrollbar grab track. */
void hk_gui_set_color_border(double r, double g, double b);

/* Primary accent hue.  Derives hover (18% alpha) and active (full) variants
 * for buttons, checkmarks, sliders, tabs, nav highlight, and resize grips. */
void hk_gui_set_color_accent(double r, double g, double b);

/* Set window, frame and tab corner rounding in pixels. */
void hk_gui_set_style_rounding(double window, double frame, double tab);

/* Set frame padding in pixels. */
void hk_gui_set_style_padding(double frame_x, double frame_y);

/* Solid coloured square.  Returns 1 when clicked.  w=0/h=0 uses default size. */
int  hk_gui_color_button(const char* id, double r, double g, double b, double a, double w, double h);

/* Copy text to the OS clipboard. */
void hk_gui_set_clipboard(const char* text);

/* Plot line color; hover variant is derived (15% brighter). */
void hk_gui_set_color_plot(double r, double g, double b);

/* Plot bar/histogram color; hover variant is derived (15% brighter). */
void hk_gui_set_color_plot_bar(double r, double g, double b);

/* Modal and nav-windowing overlay dim.  alpha=0 transparent, 1=opaque.
 * NavWindowingDimBg is set to half this alpha. */
void hk_gui_set_color_modal_dim(double alpha);

/* Window content area padding (pixels). */
void hk_gui_set_style_window_padding(double x, double y);

/* Item spacing and tree-indent width (pixels). */
void hk_gui_set_style_spacing(double item_x, double item_y, double indent);

/* Window and frame border thickness in pixels (0 = none, 1 = thin). */
void hk_gui_set_style_borders(double window, double frame);

#ifdef __cplusplus
}
#endif
