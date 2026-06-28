/*
 * kk_imgui.c — Koka FFI trampoline for lib/imgui
 *
 * This file is compiled by Koka via:
 *   extern import
 *     c file "backend/kk_imgui.c"
 * in imgui.kk.
 *
 * It converts between Koka's runtime types (kk_string_t, kk_integer_t, …)
 * and the plain-C types expected by imgui_glue.h.
 *
 * No C++ is used here.  The actual Dear ImGui logic lives in imgui_glue.cpp,
 * which is pre-compiled into libimgui_hica.a by build.sh.
 *
 * Koka type mapping (v3.2.3 kklib):
 *   Koka bool    → C bool  (NOT kk_bool_t — that doesn't exist)
 *   Koka float64 → C double (NOT kk_double_t — that doesn't exist)
 *   Koka int     → kk_integer_t
 *   Koka string  → kk_string_t
 *   Koka ()      → kk_unit_t
 *
 * This source file is part of the hica open source project
 * Copyright (C) 2026 Claes Adamsson <claes.adamsson@gmail.com>
 * See https://github.com/cladam/hica/blob/main/LICENSE for license information
 */

#include "kklib.h"
#include <string.h>

/* Forward declarations from imgui_glue.h — inlined so Koka's build cache
 * does not need to resolve the header path at compile time. */
void        hk_gui_init(const char* title, int w, int h);
int         hk_gui_begin_frame(void);
void        hk_gui_end_frame(void);
void        hk_gui_shutdown(void);
void        hk_gui_text(const char* s);
void        hk_gui_text_colored(const char* s, double r, double g, double b, double a);
void        hk_gui_text_wrapped(const char* s);
void        hk_gui_bullet_text(const char* s);
int         hk_gui_button(const char* label);
int         hk_gui_checkbox(const char* label, int def);
int         hk_gui_slider_int(const char* label, int min, int max, int def);
double      hk_gui_slider_float(const char* label, double min, double max, double def);
const char* hk_gui_input_text(const char* label, int capacity);
int         hk_gui_input_int(const char* label, int def);
double      hk_gui_input_float(const char* label, double def);
int         hk_gui_drag_int(const char* label, int min, int max, int def, double speed);
double      hk_gui_drag_float(const char* label, double min, double max, double def, double speed);
void        hk_gui_separator(void);
void        hk_gui_same_line(void);
void        hk_gui_spacing(void);
void        hk_gui_new_line(void);
void        hk_gui_dummy(double w, double h);
void        hk_gui_indent(void);
void        hk_gui_unindent(void);
int         hk_gui_begin_panel(const char* label);
void        hk_gui_end_panel(void);
int         hk_gui_radio_button(const char* label, int active);
int         hk_gui_selectable(const char* label, int selected);
int         hk_gui_begin_child(const char* id, double w, double h);
void        hk_gui_end_child(void);
int         hk_gui_combo(const char* label, const char* items, int def_index);
void        hk_gui_progress_bar(double fraction, const char* overlay);
void        hk_gui_begin_group(void);
void        hk_gui_end_group(void);
void        hk_gui_push_item_width(double w);
void        hk_gui_pop_item_width(void);
int         hk_gui_tree_node(const char* label);
void        hk_gui_tree_pop(void);
int         hk_gui_begin_tab_bar(const char* id);
void        hk_gui_end_tab_bar(void);
int         hk_gui_begin_tab_item(const char* label);
void        hk_gui_end_tab_item(void);
void        hk_gui_set_tooltip(const char* text);
int         hk_gui_begin_tooltip(void);
void        hk_gui_end_tooltip(void);
void        hk_gui_open_popup(const char* id);
int         hk_gui_begin_popup(const char* id);
int         hk_gui_begin_popup_modal(const char* id);
void        hk_gui_close_popup(void);
void        hk_gui_end_popup(void);
int         hk_gui_begin_main_menu_bar(void);
void        hk_gui_end_main_menu_bar(void);
int         hk_gui_begin_menu(const char* label);
void        hk_gui_end_menu(void);
int         hk_gui_menu_item(const char* label);
int         hk_gui_begin_table(const char* id, int columns, int flags);
void        hk_gui_end_table(void);
void        hk_gui_table_setup_column(const char* label);
void        hk_gui_table_headers_row(void);
void        hk_gui_table_next_row(void);
void        hk_gui_table_next_column(void);
void        hk_gui_set_color_text(double r, double g, double b);
void        hk_gui_set_color_bg(double r, double g, double b);
void        hk_gui_set_color_surface(double r, double g, double b);
void        hk_gui_set_color_border(double r, double g, double b);
void        hk_gui_set_color_accent(double r, double g, double b);
void        hk_gui_set_style_rounding(double window, double frame, double tab);
void        hk_gui_set_style_padding(double frame_x, double frame_y);
int         hk_gui_color_button(const char* id, double r, double g, double b, double a, double w, double h);
void        hk_gui_set_clipboard(const char* text);
void        hk_gui_set_color_plot(double r, double g, double b);
void        hk_gui_set_color_plot_bar(double r, double g, double b);
void        hk_gui_set_color_modal_dim(double alpha);
void        hk_gui_set_style_window_padding(double x, double y);
void        hk_gui_set_style_spacing(double item_x, double item_y, double indent);
void        hk_gui_set_style_borders(double window, double frame);
/* ImPlot */
void        hk_plot_push(const char* label, double y);
void        hk_plot_push_xy(const char* label, double x, double y);
void        hk_plot_clear(const char* label);
void        hk_plot_clear_all(void);
int         hk_plot_begin(const char* title, double w, double h);
void        hk_plot_end(void);
void        hk_plot_setup_axes(const char* x_label, const char* y_label);
void        hk_plot_setup_axis_limits(int axis, double v_min, double v_max, int cond);
void        hk_plot_line(const char* label);
void        hk_plot_shaded(const char* label, double y_ref);
void        hk_plot_bars(const char* label, double bar_width);
void        hk_plot_scatter(const char* label);
void        hk_plot_pie_chart(const char* labels_nl, const char* values_csv, double cx, double cy, double radius);
void        hk_plot_stairs(const char* label);
void        hk_plot_stems(const char* label, double ref);
void        hk_plot_histogram(const char* label, int bins);
void        hk_plot_heatmap(const char* label, const char* values_csv, int rows, int cols, double scale_min, double scale_max);
void        hk_plot_inf_lines(const char* label);
void        hk_gui_open_url(const char* url);
void        hk_gui_hyperlink(const char* label, const char* url);

/* ---------------------------------------------------------------------------
 * Lifecycle
 * -------------------------------------------------------------------------*/

static kk_unit_t kk_hk_gui_init(kk_string_t title, kk_integer_t w, kk_integer_t h,
                                  kk_context_t* ctx) {
    const char* t = kk_string_cbuf_borrow(title, NULL, ctx);
    hk_gui_init(t, kk_integer_clamp32(w, ctx), kk_integer_clamp32(h, ctx));
    kk_string_drop(title, ctx);
    return kk_Unit;
}

static bool kk_hk_gui_begin_frame(kk_context_t* ctx) {
    return hk_gui_begin_frame() != 0;
}

static kk_unit_t kk_hk_gui_end_frame(kk_context_t* ctx) {
    hk_gui_end_frame();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_shutdown(kk_context_t* ctx) {
    hk_gui_shutdown();
    return kk_Unit;
}

/* ---------------------------------------------------------------------------
 * Widgets
 * -------------------------------------------------------------------------*/

static kk_unit_t kk_hk_gui_text(kk_string_t s, kk_context_t* ctx) {
    const char* cs = kk_string_cbuf_borrow(s, NULL, ctx);
    hk_gui_text(cs);
    kk_string_drop(s, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_text_colored(kk_string_t s,
                                         double r, double g, double b, double a,
                                         kk_context_t* ctx) {
    const char* cs = kk_string_cbuf_borrow(s, NULL, ctx);
    hk_gui_text_colored(cs, r, g, b, a);
    kk_string_drop(s, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_text_wrapped(kk_string_t s, kk_context_t* ctx) {
    const char* cs = kk_string_cbuf_borrow(s, NULL, ctx);
    hk_gui_text_wrapped(cs);
    kk_string_drop(s, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_bullet_text(kk_string_t s, kk_context_t* ctx) {
    const char* cs = kk_string_cbuf_borrow(s, NULL, ctx);
    hk_gui_bullet_text(cs);
    kk_string_drop(s, ctx);
    return kk_Unit;
}

static bool kk_hk_gui_button(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_button(lbl);
    kk_string_drop(label, ctx);
    return r != 0;
}

static bool kk_hk_gui_checkbox(kk_string_t label, bool def,
                                kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_checkbox(lbl, def ? 1 : 0);
    kk_string_drop(label, ctx);
    return r != 0;
}

static kk_integer_t kk_hk_gui_slider_int(kk_string_t label,
                                           kk_integer_t min_val, kk_integer_t max_val,
                                           kk_integer_t def_val,
                                           kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_slider_int(lbl,
                               kk_integer_clamp32(min_val, ctx),
                               kk_integer_clamp32(max_val, ctx),
                               kk_integer_clamp32(def_val, ctx));
    kk_string_drop(label, ctx);
    return kk_integer_from_int(r, ctx);
}

static double kk_hk_gui_slider_float(kk_string_t label,
                                      double min_val, double max_val, double def_val,
                                      kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    double r = hk_gui_slider_float(lbl, min_val, max_val, def_val);
    kk_string_drop(label, ctx);
    return r;
}

static kk_string_t kk_hk_gui_input_text(kk_string_t label, kk_integer_t capacity,
                                          kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    const char* result = hk_gui_input_text(lbl, kk_integer_clamp32(capacity, ctx));
    kk_string_drop(label, ctx);
    if (!result || result[0] == '\0') return kk_string_empty();
    /* Copy the backend's static buffer into a Koka-owned string. */
    size_t slen = strlen(result);
    char* sbuf = (char*)kk_malloc(slen + 1, ctx);
    memcpy(sbuf, result, slen + 1);
    return kk_string_alloc_raw_len(slen, sbuf, true, ctx);
}

static kk_integer_t kk_hk_gui_input_int(kk_string_t label, kk_integer_t def,
                                         kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_input_int(lbl, kk_integer_clamp32(def, ctx));
    kk_string_drop(label, ctx);
    return kk_integer_from_int(r, ctx);
}

static double kk_hk_gui_input_float(kk_string_t label, double def,
                                     kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    double r = hk_gui_input_float(lbl, def);
    kk_string_drop(label, ctx);
    return r;
}

static kk_integer_t kk_hk_gui_drag_int(kk_string_t label,
                                        kk_integer_t min_val, kk_integer_t max_val,
                                        kk_integer_t def_val, double speed,
                                        kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_drag_int(lbl,
                             kk_integer_clamp32(min_val, ctx),
                             kk_integer_clamp32(max_val, ctx),
                             kk_integer_clamp32(def_val, ctx),
                             speed);
    kk_string_drop(label, ctx);
    return kk_integer_from_int(r, ctx);
}

static double kk_hk_gui_drag_float(kk_string_t label,
                                    double min_val, double max_val,
                                    double def_val, double speed,
                                    kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    double r = hk_gui_drag_float(lbl, min_val, max_val, def_val, speed);
    kk_string_drop(label, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_separator(kk_context_t* ctx) {
    hk_gui_separator();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_same_line(kk_context_t* ctx) {
    hk_gui_same_line();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_spacing(kk_context_t* ctx) {
    hk_gui_spacing();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_new_line(kk_context_t* ctx) {
    hk_gui_new_line();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_dummy(double w, double h, kk_context_t* ctx) {
    hk_gui_dummy(w, h);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_indent(kk_context_t* ctx) {
    hk_gui_indent();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_unindent(kk_context_t* ctx) {
    hk_gui_unindent();
    return kk_Unit;
}

static bool kk_hk_gui_begin_panel(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_begin_panel(lbl);
    kk_string_drop(label, ctx);
    return r != 0;
}

static kk_unit_t kk_hk_gui_end_panel(kk_context_t* ctx) {
    hk_gui_end_panel();
    return kk_Unit;
}

static bool kk_hk_gui_radio_button(kk_string_t label, bool active,
                                    kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_radio_button(lbl, active ? 1 : 0);
    kk_string_drop(label, ctx);
    return r != 0;
}

static bool kk_hk_gui_selectable(kk_string_t label, bool selected,
                                  kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    int r = hk_gui_selectable(lbl, selected ? 1 : 0);
    kk_string_drop(label, ctx);
    return r != 0;
}

static bool kk_hk_gui_begin_child(kk_string_t id, double w, double h,
                                   kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    int r = hk_gui_begin_child(cid, w, h);
    kk_string_drop(id, ctx);
    return r != 0;
}

static kk_unit_t kk_hk_gui_end_child(kk_context_t* ctx) {
    hk_gui_end_child();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_begin_group(kk_context_t* ctx) {
    hk_gui_begin_group();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_end_group(kk_context_t* ctx) {
    hk_gui_end_group();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_push_item_width(double w, kk_context_t* ctx) {
    hk_gui_push_item_width(w);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_pop_item_width(kk_context_t* ctx) {
    hk_gui_pop_item_width();
    return kk_Unit;
}

static bool kk_hk_gui_tree_node(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    bool r = hk_gui_tree_node(lbl) != 0;
    kk_string_drop(label, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_tree_pop(kk_context_t* ctx) {
    hk_gui_tree_pop();
    return kk_Unit;
}

static bool kk_hk_gui_begin_tab_bar(kk_string_t id, kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    bool r = hk_gui_begin_tab_bar(cid) != 0;
    kk_string_drop(id, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_end_tab_bar(kk_context_t* ctx) {
    hk_gui_end_tab_bar();
    return kk_Unit;
}

static bool kk_hk_gui_begin_tab_item(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    bool r = hk_gui_begin_tab_item(lbl) != 0;
    kk_string_drop(label, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_end_tab_item(kk_context_t* ctx) {
    hk_gui_end_tab_item();
    return kk_Unit;
}

/* Tooltips */

static kk_unit_t kk_hk_gui_set_tooltip(kk_string_t text, kk_context_t* ctx) {
    const char* s = kk_string_cbuf_borrow(text, NULL, ctx);
    hk_gui_set_tooltip(s);
    kk_string_drop(text, ctx);
    return kk_Unit;
}

static bool kk_hk_gui_begin_tooltip(kk_context_t* ctx) {
    return hk_gui_begin_tooltip() != 0;
}

static kk_unit_t kk_hk_gui_end_tooltip(kk_context_t* ctx) {
    hk_gui_end_tooltip();
    return kk_Unit;
}

/* Popups */

static kk_unit_t kk_hk_gui_open_popup(kk_string_t id, kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    hk_gui_open_popup(cid);
    kk_string_drop(id, ctx);
    return kk_Unit;
}

static bool kk_hk_gui_begin_popup(kk_string_t id, kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    bool r = hk_gui_begin_popup(cid) != 0;
    kk_string_drop(id, ctx);
    return r;
}

static bool kk_hk_gui_begin_popup_modal(kk_string_t id, kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    bool r = hk_gui_begin_popup_modal(cid) != 0;
    kk_string_drop(id, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_close_popup(kk_context_t* ctx) {
    hk_gui_close_popup();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_end_popup(kk_context_t* ctx) {
    hk_gui_end_popup();
    return kk_Unit;
}

/* Menu bar */

static bool kk_hk_gui_begin_main_menu_bar(kk_context_t* ctx) {
    return hk_gui_begin_main_menu_bar() != 0;
}

static kk_unit_t kk_hk_gui_end_main_menu_bar(kk_context_t* ctx) {
    hk_gui_end_main_menu_bar();
    return kk_Unit;
}

static bool kk_hk_gui_begin_menu(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    bool r = hk_gui_begin_menu(lbl) != 0;
    kk_string_drop(label, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_end_menu(kk_context_t* ctx) {
    hk_gui_end_menu();
    return kk_Unit;
}

static bool kk_hk_gui_menu_item(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    bool r = hk_gui_menu_item(lbl) != 0;
    kk_string_drop(label, ctx);
    return r;
}

/* Tables */

static bool kk_hk_gui_begin_table(kk_string_t id, kk_integer_t columns,
                                   kk_integer_t flags, kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    bool r = hk_gui_begin_table(cid,
                                 kk_integer_clamp32(columns, ctx),
                                 kk_integer_clamp32(flags, ctx)) != 0;
    kk_string_drop(id, ctx);
    return r;
}

static kk_unit_t kk_hk_gui_end_table(kk_context_t* ctx) {
    hk_gui_end_table();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_table_setup_column(kk_string_t label, kk_context_t* ctx) {
    const char* lbl = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_gui_table_setup_column(lbl);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_table_headers_row(kk_context_t* ctx) {
    hk_gui_table_headers_row();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_table_next_row(kk_context_t* ctx) {
    hk_gui_table_next_row();
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_table_next_column(kk_context_t* ctx) {
    hk_gui_table_next_column();
    return kk_Unit;
}

/* Theme */

static kk_unit_t kk_hk_gui_set_color_text(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_text(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_bg(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_bg(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_surface(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_surface(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_border(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_border(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_accent(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_accent(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_style_rounding(double window, double frame, double tab,
                                               kk_context_t* ctx) {
    hk_gui_set_style_rounding(window, frame, tab); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_style_padding(double frame_x, double frame_y,
                                              kk_context_t* ctx) {
    hk_gui_set_style_padding(frame_x, frame_y); return kk_Unit;
}
static bool kk_hk_gui_color_button(kk_string_t id, double r, double g, double b,
                                    double a, double w, double h,
                                    kk_context_t* ctx) {
    const char* cid = kk_string_cbuf_borrow(id, NULL, ctx);
    bool result = hk_gui_color_button(cid, r, g, b, a, w, h) != 0;
    kk_string_drop(id, ctx);
    return result;
}
static kk_unit_t kk_hk_gui_set_clipboard(kk_string_t text, kk_context_t* ctx) {
    const char* t = kk_string_cbuf_borrow(text, NULL, ctx);
    hk_gui_set_clipboard(t);
    kk_string_drop(text, ctx);
    return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_plot(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_plot(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_plot_bar(double r, double g, double b, kk_context_t* ctx) {
    hk_gui_set_color_plot_bar(r, g, b); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_color_modal_dim(double alpha, kk_context_t* ctx) {
    hk_gui_set_color_modal_dim(alpha); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_style_window_padding(double x, double y, kk_context_t* ctx) {
    hk_gui_set_style_window_padding(x, y); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_style_spacing(double item_x, double item_y, double indent, kk_context_t* ctx) {
    hk_gui_set_style_spacing(item_x, item_y, indent); return kk_Unit;
}
static kk_unit_t kk_hk_gui_set_style_borders(double window, double frame, kk_context_t* ctx) {
    hk_gui_set_style_borders(window, frame); return kk_Unit;
}

static kk_integer_t kk_hk_gui_combo(kk_string_t label, kk_string_t items,
                                     kk_integer_t def_index,
                                     kk_context_t* ctx) {
    const char* lbl  = kk_string_cbuf_borrow(label, NULL, ctx);
    const char* itms = kk_string_cbuf_borrow(items, NULL, ctx);
    int r = hk_gui_combo(lbl, itms, kk_integer_clamp32(def_index, ctx));
    kk_string_drop(label, ctx);
    kk_string_drop(items, ctx);
    return kk_integer_from_int(r, ctx);
}

static kk_unit_t kk_hk_gui_progress_bar(double fraction, kk_string_t overlay,
                                         kk_context_t* ctx) {
    const char* ov = kk_string_cbuf_borrow(overlay, NULL, ctx);
    /* Empty string means no overlay label — pass NULL so ImGui shows the default % */
    hk_gui_progress_bar(fraction, ov[0] != '\0' ? ov : NULL);
    kk_string_drop(overlay, ctx);
    return kk_Unit;
}

/* ---------------------------------------------------------------------------
 * ImPlot — FFI trampolines
 * -------------------------------------------------------------------------*/

static kk_unit_t kk_hk_plot_push(kk_string_t label, double y, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_push(l, y);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_push_xy(kk_string_t label, double x, double y, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_push_xy(l, x, y);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_clear(kk_string_t label, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_clear(l);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_clear_all(kk_context_t* ctx) {
    hk_plot_clear_all();
    return kk_Unit;
}

static bool kk_hk_plot_begin(kk_string_t title, double w, double h, kk_context_t* ctx) {
    const char* t = kk_string_cbuf_borrow(title, NULL, ctx);
    bool r = hk_plot_begin(t, w, h) != 0;
    kk_string_drop(title, ctx);
    return r;
}

static kk_unit_t kk_hk_plot_end(kk_context_t* ctx) {
    hk_plot_end();
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_setup_axes(kk_string_t x_label, kk_string_t y_label,
                                        kk_context_t* ctx) {
    const char* xl = kk_string_cbuf_borrow(x_label, NULL, ctx);
    const char* yl = kk_string_cbuf_borrow(y_label, NULL, ctx);
    hk_plot_setup_axes(xl, yl);
    kk_string_drop(x_label, ctx);
    kk_string_drop(y_label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_setup_axis_limits(kk_integer_t axis, double v_min, double v_max,
                                               kk_integer_t cond, kk_context_t* ctx) {
    hk_plot_setup_axis_limits(kk_integer_clamp32(axis, ctx), v_min, v_max,
                               kk_integer_clamp32(cond, ctx));
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_line(kk_string_t label, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_line(l);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_shaded(kk_string_t label, double y_ref, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_shaded(l, y_ref);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_bars(kk_string_t label, double bar_width, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_bars(l, bar_width);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_scatter(kk_string_t label, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_scatter(l);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_pie_chart(kk_string_t labels_nl, kk_string_t values_csv,
                                       double cx, double cy, double radius,
                                       kk_context_t* ctx) {
    const char* ls = kk_string_cbuf_borrow(labels_nl,  NULL, ctx);
    const char* vs = kk_string_cbuf_borrow(values_csv, NULL, ctx);
    hk_plot_pie_chart(ls, vs, cx, cy, radius);
    kk_string_drop(labels_nl,  ctx);
    kk_string_drop(values_csv, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_stairs(kk_string_t label, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_stairs(l);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_stems(kk_string_t label, double ref, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_stems(l, ref);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_histogram(kk_string_t label, kk_integer_t bins,
                                       kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_histogram(l, kk_integer_clamp32(bins, ctx));
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_heatmap(kk_string_t label, kk_string_t values_csv,
                                     kk_integer_t rows, kk_integer_t cols,
                                     double scale_min, double scale_max,
                                     kk_context_t* ctx) {
    const char* l  = kk_string_cbuf_borrow(label,      NULL, ctx);
    const char* vs = kk_string_cbuf_borrow(values_csv, NULL, ctx);
    hk_plot_heatmap(l, vs,
                    kk_integer_clamp32(rows, ctx),
                    kk_integer_clamp32(cols, ctx),
                    scale_min, scale_max);
    kk_string_drop(label,      ctx);
    kk_string_drop(values_csv, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_plot_inf_lines(kk_string_t label, kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    hk_plot_inf_lines(l);
    kk_string_drop(label, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_open_url(kk_string_t url, kk_context_t* ctx) {
    const char* s = kk_string_cbuf_borrow(url, NULL, ctx);
    hk_gui_open_url(s);
    kk_string_drop(url, ctx);
    return kk_Unit;
}

static kk_unit_t kk_hk_gui_hyperlink(kk_string_t label, kk_string_t url,
                                       kk_context_t* ctx) {
    const char* l = kk_string_cbuf_borrow(label, NULL, ctx);
    const char* u = kk_string_cbuf_borrow(url,   NULL, ctx);
    hk_gui_hyperlink(l, u);
    kk_string_drop(label, ctx);
    kk_string_drop(url,   ctx);
    return kk_Unit;
}
