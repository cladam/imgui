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

#ifdef __cplusplus
}
#endif
