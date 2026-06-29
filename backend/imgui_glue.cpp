/*
 * imgui_glue.cpp — Dear ImGui + SDL2 + OpenGL3 backend for hica
 *
 * Implements the plain-C API declared in imgui_glue.h.
 * Linked as a pre-built static library (libimgui_hica.a) by build.sh.
 *
 * Vendor layout expected:
 *   lib/imgui/vendor/imgui/               ← Dear ImGui source
 *   lib/imgui/vendor/imgui/backends/      ← SDL2 + OpenGL3 backend sources
 *
 * This source file is part of the hica open source project
 * Copyright (C) 2026 Claes Adamsson <claes.adamsson@gmail.com>
 * See https://github.com/cladam/hica/blob/main/LICENSE for license information
 */

#include "imgui_glue.h"
#include "inter_font.h"   /* Inter-Regular — embedded via xxd -i */

#include <SDL.h>
#include <SDL_opengl.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Dear ImGui — C++ API used internally, C API exposed via imgui_glue.h */
#include "../vendor/imgui/imgui.h"
#include "../vendor/imgui/backends/imgui_impl_sdl2.h"
#include "../vendor/imgui/backends/imgui_impl_opengl3.h"

/* ImPlot — 2-D plotting extension */
#include "../vendor/implot/implot.h"
#include <vector>
#include <string>
#include <algorithm>

/* ---------------------------------------------------------------------------
 * Internal state
 * -------------------------------------------------------------------------*/

static SDL_Window*    g_window  = nullptr;
static SDL_GLContext  g_gl_ctx  = nullptr;
static bool           g_quit    = false;

/* GL clear colour — kept in sync with hk_gui_set_color_bg so the area
 * behind the root ImGui window matches the background theme. */
static float g_clear_r = 0.902f, g_clear_g = 0.914f, g_clear_b = 0.933f;

/* ---------------------------------------------------------------------------
 * Per-widget state tables
 *
 * Widgets with persistent state (sliders, checkboxes, text inputs) are keyed
 * by a FNV-1a hash of their label string.  Up to HK_MAX_SLOTS distinct widgets
 * of each kind are supported per window.
 * -------------------------------------------------------------------------*/

#define HK_MAX_SLOTS 256
#define HK_TEXT_BUF  512

static uint32_t fnv1a(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) { h ^= (uint8_t)*s++; h *= 16777619u; }
    return h;
}

/* --- int state (sliders) --- */
struct hk_int_slot { uint32_t hash; int value; bool init; };
static hk_int_slot g_int_slots[HK_MAX_SLOTS];
static int         g_int_n = 0;

static int* int_state(const char* label, int def) {
    uint32_t h = fnv1a(label);
    for (int i = 0; i < g_int_n; i++)
        if (g_int_slots[i].hash == h) return &g_int_slots[i].value;
    if (g_int_n < HK_MAX_SLOTS) {
        g_int_slots[g_int_n] = { h, def, true };
        return &g_int_slots[g_int_n++].value;
    }
    fprintf(stderr, "hica imgui: int state table full (max %d slots)\n", HK_MAX_SLOTS);
    return nullptr;
}

/* --- float state (sliders) --- */
struct hk_float_slot { uint32_t hash; float value; };
static hk_float_slot g_float_slots[HK_MAX_SLOTS];
static int           g_float_n = 0;

static float* float_state(const char* label, float def) {
    uint32_t h = fnv1a(label);
    for (int i = 0; i < g_float_n; i++)
        if (g_float_slots[i].hash == h) return &g_float_slots[i].value;
    if (g_float_n < HK_MAX_SLOTS) {
        g_float_slots[g_float_n] = { h, def };
        return &g_float_slots[g_float_n++].value;
    }
    fprintf(stderr, "hica imgui: float state table full (max %d slots)\n", HK_MAX_SLOTS);
    return nullptr;
}

/* --- bool state (checkboxes) --- */
struct hk_bool_slot { uint32_t hash; bool value; };
static hk_bool_slot g_bool_slots[HK_MAX_SLOTS];
static int          g_bool_n = 0;

static bool* bool_state(const char* label, bool def) {
    uint32_t h = fnv1a(label);
    for (int i = 0; i < g_bool_n; i++)
        if (g_bool_slots[i].hash == h) return &g_bool_slots[i].value;
    if (g_bool_n < HK_MAX_SLOTS) {
        g_bool_slots[g_bool_n] = { h, def };
        return &g_bool_slots[g_bool_n++].value;
    }
    fprintf(stderr, "hica imgui: bool state table full (max %d slots)\n", HK_MAX_SLOTS);
    return nullptr;
}

/* --- text state (input fields) --- */
struct hk_text_slot { uint32_t hash; char buf[HK_TEXT_BUF]; };
static hk_text_slot g_text_slots[HK_MAX_SLOTS];
static int          g_text_n = 0;

static char* text_state(const char* label, int capacity) {
    (void)capacity; /* clamped to HK_TEXT_BUF internally */
    uint32_t h = fnv1a(label);
    for (int i = 0; i < g_text_n; i++)
        if (g_text_slots[i].hash == h) return g_text_slots[i].buf;
    if (g_text_n < HK_MAX_SLOTS) {
        g_text_slots[g_text_n].hash = h;
        g_text_slots[g_text_n].buf[0] = '\0';
        return g_text_slots[g_text_n++].buf;
    }
    fprintf(stderr, "hica imgui: text state table full (max %d slots)\n", HK_MAX_SLOTS);
    return nullptr;
}

/* --- combo/dropdown state (selected index) --- */
/* Reuses the int state table — combo current-item is just an int. */

/* ---------------------------------------------------------------------------
 * Ilseon theme — OLED-focused dark palette, Inter font
 *
 * Palette values taken from stdlib/std/term.hc ilseon_* functions.
 * -------------------------------------------------------------------------*/

/* ---------------------------------------------------------------------------
 * hica theme — default
 *
 * Palette derived from the hica website and logo:
 *   Indigo  #5B4FCF  — primary accent (matches nav-active + link colour)
 *   Blue    #3A6BD5  — secondary (logo hexagon fill)
 *   Coral   #E04545  — active-press accent (logo arrow →)
 *   Backgrounds run from #0B0C14 (deepest) through #12131F → #1A1B2E.
 *
 * Layout geometry is Spectrum-inspired: slightly more rounding and padding
 * than vanilla ImGui.
 * -------------------------------------------------------------------------*/
static void hk_apply_hica_theme() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    // --- Palette — exact values from hica/assets/css/style.css + syntax.css ---
    //   body bg         : #FCFCFC
    //   --sidebar-bg    : #f8f9fa
    //   --border-color  : #e2e8f0
    //   --primary-text  : #1e293b
    //   --secondary-text: #64748b
    //   --accent-indigo : #4f46e5
    //   --accent-cyan   : #0891b2  (function color in syntax.css)
    //   keyword violet  : #7c3aed  (syntax.css .k)
    //   variable orange : #ea580c  (syntax.css .nv)
    const ImVec4 indigo      = ImVec4(0.310f, 0.275f, 0.898f, 1.00f); // #4f46e5 --accent-indigo
    const ImVec4 indigo_h    = ImVec4(0.427f, 0.400f, 0.945f, 1.00f); // #6D66F1 hover
    const ImVec4 indigo_med  = ImVec4(0.310f, 0.275f, 0.898f, 0.18f);
    const ImVec4 indigo_dim  = ImVec4(0.310f, 0.275f, 0.898f, 0.10f);
    const ImVec4 indigo_text = ImVec4(1.00f,  1.00f,  1.00f,  1.00f); // white text on indigo bg
    const ImVec4 cyan        = ImVec4(0.031f, 0.569f, 0.698f, 1.00f); // #0891b2 --accent-cyan
    const ImVec4 orange      = ImVec4(0.918f, 0.345f, 0.047f, 1.00f); // #ea580c variable colour
    const ImVec4 text_main   = ImVec4(0.118f, 0.161f, 0.231f, 1.00f); // #1e293b --primary-text
    const ImVec4 text_muted  = ImVec4(0.392f, 0.455f, 0.545f, 1.00f); // #64748b --secondary-text
    const ImVec4 border      = ImVec4(0.886f, 0.910f, 0.941f, 1.00f); // #e2e8f0 --border-color
    const ImVec4 bg_body     = ImVec4(0.902f, 0.914f, 0.933f, 1.00f); // #E6E9EE medium light (window bg)
    const ImVec4 bg_sidebar  = ImVec4(0.863f, 0.878f, 0.902f, 1.00f); // #DCE0E6 slightly deeper for panels
    const ImVec4 bg_frame    = ImVec4(0.973f, 0.976f, 0.980f, 1.00f); // #f8f9fa input fields (was white)
    const ImVec4 bg_hover    = ImVec4(0.831f, 0.851f, 0.882f, 1.00f); // #D4D9E1 hover
    const ImVec4 bg_popup    = ImVec4(1.000f, 1.000f, 1.000f, 0.98f); // white popups
    const ImVec4 none        = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

    c[ImGuiCol_Text]                  = text_main;
    c[ImGuiCol_TextDisabled]          = text_muted;
    c[ImGuiCol_WindowBg]              = bg_body;
    c[ImGuiCol_ChildBg]               = bg_sidebar;
    c[ImGuiCol_PopupBg]               = bg_popup;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = none;
    c[ImGuiCol_FrameBg]               = bg_frame;
    c[ImGuiCol_FrameBgHovered]        = bg_hover;
    c[ImGuiCol_FrameBgActive]         = indigo_dim;
    c[ImGuiCol_TitleBg]               = bg_sidebar;
    c[ImGuiCol_TitleBgActive]         = indigo_med;
    c[ImGuiCol_TitleBgCollapsed]      = bg_sidebar;
    c[ImGuiCol_MenuBarBg]             = bg_sidebar;
    c[ImGuiCol_ScrollbarBg]           = bg_sidebar;
    c[ImGuiCol_ScrollbarGrab]         = border;
    c[ImGuiCol_ScrollbarGrabHovered]  = indigo_med;
    c[ImGuiCol_ScrollbarGrabActive]   = indigo;
    c[ImGuiCol_CheckMark]             = indigo;
    c[ImGuiCol_SliderGrab]            = indigo;
    c[ImGuiCol_SliderGrabActive]      = indigo_h;
    c[ImGuiCol_Button]                = bg_sidebar;
    c[ImGuiCol_ButtonHovered]         = indigo_med;
    c[ImGuiCol_ButtonActive]          = indigo;
    c[ImGuiCol_Header]                = none;
    c[ImGuiCol_HeaderHovered]         = indigo_med;
    c[ImGuiCol_HeaderActive]          = indigo_dim;
    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = indigo_med;
    c[ImGuiCol_SeparatorActive]       = indigo;
    c[ImGuiCol_ResizeGrip]            = indigo_dim;
    c[ImGuiCol_ResizeGripHovered]     = indigo_med;
    c[ImGuiCol_ResizeGripActive]      = indigo;
    c[ImGuiCol_Tab]                   = bg_sidebar;
    c[ImGuiCol_TabHovered]            = indigo_med;
    c[ImGuiCol_TabSelected]           = bg_body;
    c[ImGuiCol_TabSelectedOverline]   = indigo;
    c[ImGuiCol_TabDimmed]             = bg_sidebar;
    c[ImGuiCol_TabDimmedSelected]     = bg_sidebar;
    c[ImGuiCol_PlotLines]             = cyan;
    c[ImGuiCol_PlotLinesHovered]      = indigo_h;
    c[ImGuiCol_PlotHistogram]         = orange;
    c[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.000f, 0.490f, 0.145f, 1.00f); // #FF7D25 brighter orange
    c[ImGuiCol_TableHeaderBg]         = bg_sidebar;
    c[ImGuiCol_TableBorderStrong]     = border;
    c[ImGuiCol_TableBorderLight]      = ImVec4(0.886f, 0.910f, 0.941f, 0.60f);
    c[ImGuiCol_TableRowBg]            = none;
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(0.310f, 0.275f, 0.898f, 0.04f);
    c[ImGuiCol_TextSelectedBg]        = indigo_med;
    c[ImGuiCol_DragDropTarget]        = indigo;
    c[ImGuiCol_NavHighlight]          = indigo;
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(0.310f, 0.275f, 0.898f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.10f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.20f);

    // --- Spectrum-inspired geometry (more rounding + padding than vanilla ImGui) ---
    s.WindowRounding    = 8.0f;
    s.FrameRounding     = 5.0f;
    s.ScrollbarRounding = 8.0f;
    s.GrabRounding      = 5.0f;
    s.TabRounding       = 5.0f;
    s.PopupRounding     = 6.0f;
    s.WindowPadding     = ImVec2(14.0f, 12.0f);
    s.FramePadding      = ImVec2(10.0f, 5.0f);
    s.ItemSpacing       = ImVec2(8.0f, 6.0f);
    s.IndentSpacing     = 18.0f;
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 0.0f;
}

/* ---------------------------------------------------------------------------
 * Ilseon theme — alternative dark theme (publishable separately)
 *
 * OLED-black backgrounds with a teal (#00BFA5) accent.
 * -------------------------------------------------------------------------*/
static void hk_apply_ilseon_theme() {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    // --- Palette ---
    const ImVec4 teal       = ImVec4(0.000f, 0.749f, 0.647f, 1.00f); // #00BFA5 TealAccent
    const ImVec4 teal_hover = ImVec4(0.200f, 0.820f, 0.730f, 1.00f); // brightened
    const ImVec4 teal_med   = ImVec4(0.000f, 0.749f, 0.647f, 0.60f);
    const ImVec4 teal_dim   = ImVec4(0.000f, 0.749f, 0.647f, 0.28f);
    const ImVec4 text_main  = ImVec4(0.910f, 0.910f, 0.925f, 1.00f); // #E8E8EC
    const ImVec4 text_muted = ImVec4(0.533f, 0.533f, 0.533f, 1.00f); // #888888 MutedDetail
    const ImVec4 border     = ImVec4(0.369f, 0.427f, 0.494f, 0.38f); // #5E6D7E SlateBlue
    const ImVec4 bg_deep    = ImVec4(0.047f, 0.047f, 0.063f, 1.00f); // #0C0C10 OLED black
    const ImVec4 bg_base    = ImVec4(0.071f, 0.071f, 0.090f, 1.00f); // #121217
    const ImVec4 bg_frame   = ImVec4(0.110f, 0.110f, 0.141f, 1.00f); // #1C1C24
    const ImVec4 bg_hover   = ImVec4(0.150f, 0.150f, 0.188f, 1.00f);
    const ImVec4 bg_popup   = ImVec4(0.102f, 0.102f, 0.125f, 0.97f); // #1A1A20
    const ImVec4 none       = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

    c[ImGuiCol_Text]                  = text_main;
    c[ImGuiCol_TextDisabled]          = text_muted;
    c[ImGuiCol_WindowBg]              = bg_base;
    c[ImGuiCol_ChildBg]               = bg_deep;
    c[ImGuiCol_PopupBg]               = bg_popup;
    c[ImGuiCol_Border]                = border;
    c[ImGuiCol_BorderShadow]          = none;
    c[ImGuiCol_FrameBg]               = bg_frame;
    c[ImGuiCol_FrameBgHovered]        = bg_hover;
    c[ImGuiCol_FrameBgActive]         = teal_dim;
    c[ImGuiCol_TitleBg]               = bg_deep;
    c[ImGuiCol_TitleBgActive]         = ImVec4(0.000f, 0.380f, 0.330f, 1.0f);
    c[ImGuiCol_TitleBgCollapsed]      = bg_deep;
    c[ImGuiCol_MenuBarBg]             = bg_deep;
    c[ImGuiCol_ScrollbarBg]           = bg_deep;
    c[ImGuiCol_ScrollbarGrab]         = ImVec4(0.369f, 0.427f, 0.494f, 0.55f);
    c[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.490f, 0.522f, 0.592f, 0.80f);
    c[ImGuiCol_ScrollbarGrabActive]   = teal;
    c[ImGuiCol_CheckMark]             = teal;
    c[ImGuiCol_SliderGrab]            = teal_med;
    c[ImGuiCol_SliderGrabActive]      = teal_hover;
    c[ImGuiCol_Button]                = bg_frame;
    c[ImGuiCol_ButtonHovered]         = teal_med;
    c[ImGuiCol_ButtonActive]          = teal;
    c[ImGuiCol_Header]                = teal_dim;
    c[ImGuiCol_HeaderHovered]         = teal_med;
    c[ImGuiCol_HeaderActive]          = teal;
    c[ImGuiCol_Separator]             = border;
    c[ImGuiCol_SeparatorHovered]      = teal_med;
    c[ImGuiCol_SeparatorActive]       = teal;
    c[ImGuiCol_ResizeGrip]            = teal_dim;
    c[ImGuiCol_ResizeGripHovered]     = teal_med;
    c[ImGuiCol_ResizeGripActive]      = teal;
    c[ImGuiCol_Tab]                   = bg_frame;
    c[ImGuiCol_TabHovered]            = teal_med;
    c[ImGuiCol_TabSelected]           = teal_dim;
    c[ImGuiCol_TabSelectedOverline]   = teal;
    c[ImGuiCol_TabDimmed]             = bg_deep;
    c[ImGuiCol_TabDimmedSelected]     = bg_frame;
    c[ImGuiCol_PlotLines]             = ImVec4(0.353f, 0.608f, 0.502f, 1.0f); // MutedTeal
    c[ImGuiCol_PlotLinesHovered]      = teal_hover;
    c[ImGuiCol_PlotHistogram]         = ImVec4(0.753f, 0.541f, 0.243f, 1.0f); // QuietAmber
    c[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.886f, 0.690f, 0.369f, 1.0f); // Ochre
    c[ImGuiCol_TableHeaderBg]         = bg_deep;
    c[ImGuiCol_TableBorderStrong]     = border;
    c[ImGuiCol_TableBorderLight]      = ImVec4(0.20f, 0.20f, 0.25f, 0.5f);
    c[ImGuiCol_TableRowBg]            = none;
    c[ImGuiCol_TableRowBgAlt]         = ImVec4(1.0f, 1.0f, 1.0f, 0.03f);
    c[ImGuiCol_TextSelectedBg]        = teal_dim;
    c[ImGuiCol_DragDropTarget]        = teal;
    c[ImGuiCol_NavHighlight]          = teal;
    c[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.20f);
    c[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);

    // --- Rounding & spacing ---
    s.WindowRounding    = 6.0f;
    s.FrameRounding     = 4.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding      = 4.0f;
    s.TabRounding       = 4.0f;
    s.WindowPadding     = ImVec2(12.0f, 10.0f);
    s.FramePadding      = ImVec2(8.0f, 4.0f);
    s.ItemSpacing       = ImVec2(8.0f, 6.0f);
    s.WindowBorderSize  = 1.0f;
    s.FrameBorderSize   = 0.0f;
}

/* ---------------------------------------------------------------------------
 * Lifecycle
 * -------------------------------------------------------------------------*/

void hk_gui_init(const char* title, int w, int h) {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return;
    }

    /* Request OpenGL 3.2 Core Profile (required for GLSL #version 150 on macOS) */
#ifdef __APPLE__
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    g_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        w, h,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );
    if (!g_window) {
        fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }

    g_gl_ctx = SDL_GL_CreateContext(g_window);
    if (!g_gl_ctx) {
        fprintf(stderr, "SDL_GL_CreateContext error: %s\n", SDL_GetError());
        SDL_DestroyWindow(g_window);
        SDL_Quit();
        return;
    }

    SDL_GL_MakeCurrent(g_window, g_gl_ctx);
    SDL_GL_SetSwapInterval(1); /* vsync */

    /* Dear ImGui setup */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    /* --- Inter font, HiDPI-aware --- */
    int ww, wh, dw, dh;
    SDL_GetWindowSize(g_window, &ww, &wh);
    SDL_GL_GetDrawableSize(g_window, &dw, &dh);
    float dpi_scale = (ww > 0) ? (float)dw / (float)ww : 1.0f;
    ImFontConfig font_cfg;
    font_cfg.FontDataOwnedByAtlas = false;  /* static array — do not free */
    io.Fonts->AddFontFromMemoryTTF(
        (void*)Inter_Regular_ttf, (int)Inter_Regular_ttf_len,
        16.0f * dpi_scale, &font_cfg);
    io.FontGlobalScale = 1.0f / dpi_scale;  /* render at logical pixels */

    /* --- Ilseon colour theme --- */
    hk_apply_hica_theme();

    /* Backends: GLSL version must match the context we requested above */
    ImGui_ImplSDL2_InitForOpenGL(g_window, g_gl_ctx);
    ImGui_ImplOpenGL3_Init("#version 150");

    g_quit = false;
}

int hk_gui_begin_frame(void) {
    if (g_quit || !g_window) return 0;

    /* Poll SDL events — ImGui needs them for mouse/keyboard */
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        ImGui_ImplSDL2_ProcessEvent(&e);
        if (e.type == SDL_QUIT) return 0;
        if (e.type == SDL_WINDOWEVENT
                && e.window.event == SDL_WINDOWEVENT_CLOSE
                && e.window.windowID == SDL_GetWindowID(g_window))
            return 0;
    }

    /* Start a new ImGui frame */
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame();
    ImGui::NewFrame();

    /* Full-screen root panel that fills the OS window.
     * The hica user draws widgets directly into this panel. */
    int w, h;
    SDL_GetWindowSize(g_window, &w, &h);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2((float)w, (float)h), ImGuiCond_Always);
    ImGui::Begin("##hica_root", nullptr,
        ImGuiWindowFlags_NoTitleBar  |
        ImGuiWindowFlags_NoResize    |
        ImGuiWindowFlags_NoMove      |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoSavedSettings);

    return 1;
}

void hk_gui_end_frame(void) {
    ImGui::End(); /* close the root panel */
    ImGui::Render();

    ImGuiIO& io = ImGui::GetIO();
    SDL_GL_MakeCurrent(g_window, g_gl_ctx);
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    glClearColor(g_clear_r, g_clear_g, g_clear_b, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(g_window);
}

void hk_gui_shutdown(void) {
    if (!g_window) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
    SDL_GL_DeleteContext(g_gl_ctx);
    SDL_DestroyWindow(g_window);
    SDL_Quit();
    g_window = nullptr;
    g_gl_ctx = nullptr;
}

/* ---------------------------------------------------------------------------
 * Widgets
 * -------------------------------------------------------------------------*/

void hk_gui_text(const char* s) {
    ImGui::TextUnformatted(s);
}

void hk_gui_text_colored(const char* s, double r, double g, double b, double a) {
    ImGui::TextColored(ImVec4((float)r, (float)g, (float)b, (float)a), "%s", s);
}

void hk_gui_text_wrapped(const char* s) {
    ImGui::TextWrapped("%s", s);
}

void hk_gui_bullet_text(const char* s) {
    ImGui::BulletText("%s", s);
}

int hk_gui_button(const char* label) {
    return ImGui::Button(label) ? 1 : 0;
}

int hk_gui_checkbox(const char* label, int def) {
    bool* v = bool_state(label, def != 0);
    if (!v) return def;
    *v = (def != 0);   /* sync from caller every frame so external model changes are reflected */
    ImGui::Checkbox(label, v);
    return *v ? 1 : 0;
}

int hk_gui_slider_int(const char* label, int min, int max, int def) {
    int* v = int_state(label, def);
    if (!v) return def;
    ImGui::SliderInt(label, v, min, max);
    return *v;
}

double hk_gui_slider_float(const char* label, double min, double max, double def) {
    float* v = float_state(label, (float)def);
    if (!v) return def;
    ImGui::SliderFloat(label, v, (float)min, (float)max);
    return (double)*v;
}

const char* hk_gui_input_text(const char* label, int capacity) {
    char* buf = text_state(label, capacity);
    if (!buf) return "";
    int cap = capacity < HK_TEXT_BUF ? capacity : HK_TEXT_BUF - 1;
    ImGui::InputText(label, buf, (size_t)cap);
    return buf;
}

int hk_gui_input_int(const char* label, int def) {
    int* v = int_state(label, def);
    if (!v) return def;
    ImGui::InputInt(label, v);
    return *v;
}

double hk_gui_input_float(const char* label, double def) {
    float* v = float_state(label, (float)def);
    if (!v) return def;
    ImGui::InputFloat(label, v);
    return (double)*v;
}

int hk_gui_drag_int(const char* label, int min, int max, int def, double speed) {
    int* v = int_state(label, def);
    if (!v) return def;
    ImGui::DragInt(label, v, (float)speed, min, max);
    return *v;
}

double hk_gui_drag_float(const char* label, double min, double max, double def, double speed) {
    float* v = float_state(label, (float)def);
    if (!v) return def;
    ImGui::DragFloat(label, v, (float)speed, (float)min, (float)max);
    return (double)*v;
}

void hk_gui_separator(void) { ImGui::Separator(); }
void hk_gui_same_line(void) { ImGui::SameLine(); }
void hk_gui_spacing(void)   { ImGui::Spacing(); }
void hk_gui_new_line(void)  { ImGui::NewLine(); }
void hk_gui_dummy(double w, double h) { ImGui::Dummy(ImVec2((float)w, (float)h)); }
void hk_gui_indent(void)    { ImGui::Indent(); }
void hk_gui_unindent(void)  { ImGui::Unindent(); }

int hk_gui_begin_panel(const char* label) {
    return ImGui::CollapsingHeader(label) ? 1 : 0;
}

void hk_gui_end_panel(void) {
    /* CollapsingHeader is a single widget — no explicit End call needed.
     * hk_gui_end_panel exists so the hica API is symmetric. */
}

int hk_gui_radio_button(const char* label, int active) {
    return ImGui::RadioButton(label, active != 0) ? 1 : 0;
}

int hk_gui_selectable(const char* label, int selected) {
    /* Stateless: caller owns the selection state.  Returns true when clicked. */
    return ImGui::Selectable(label, selected != 0) ? 1 : 0;
}

int hk_gui_begin_child(const char* id, double w, double h) {
    /* Always returns 1; EndChild must always be called. */
    ImGui::BeginChild(id, ImVec2((float)w, (float)h));
    return 1;
}

void hk_gui_end_child(void) {
    ImGui::EndChild();
}

int hk_gui_combo(const char* label, const char* items, int def_index) {
    int* v = int_state(label, def_index);
    if (!v) return def_index;
    /* ImGui::Combo expects items separated by '\0' and terminated by '\0\0'.
     * We accept a newline-separated string and convert it in a local buffer. */
    static char buf[4096];
    size_t len = strlen(items);
    if (len >= sizeof(buf) - 1) len = sizeof(buf) - 2;
    for (size_t i = 0; i < len; i++)
        buf[i] = (items[i] == '\n') ? '\0' : items[i];
    buf[len]     = '\0';
    buf[len + 1] = '\0'; /* double-null terminator */
    ImGui::Combo(label, v, buf);
    return *v;
}

void hk_gui_progress_bar(double fraction, const char* overlay) {
    /* ImVec2(-1,0) = fill available width; overlay NULL uses default "XX%" */
    ImGui::ProgressBar((float)fraction, ImVec2(-1.0f, 0.0f), overlay);
}

void hk_gui_begin_group(void)  { ImGui::BeginGroup(); }
void hk_gui_end_group(void)    { ImGui::EndGroup(); }

void hk_gui_push_item_width(double w) { ImGui::PushItemWidth((float)w); }
void hk_gui_pop_item_width(void)      { ImGui::PopItemWidth(); }

int hk_gui_tree_node(const char* label) {
    return ImGui::TreeNode(label) ? 1 : 0;
}

void hk_gui_tree_pop(void) { ImGui::TreePop(); }

int hk_gui_begin_tab_bar(const char* id) {
    return ImGui::BeginTabBar(id) ? 1 : 0;
}

void hk_gui_end_tab_bar(void) {
    ImGui::EndTabBar();
}

int hk_gui_begin_tab_item(const char* label) {
    return ImGui::BeginTabItem(label) ? 1 : 0;
}

void hk_gui_end_tab_item(void) {
    ImGui::EndTabItem();
}

/* ---------------------------------------------------------------------------
 * Tooltips
 * -------------------------------------------------------------------------*/

void hk_gui_set_tooltip(const char* text) {
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
        ImGui::SetTooltip("%s", text);
}

int hk_gui_begin_tooltip(void) {
    ImGui::BeginTooltip();
    return 1;
}

void hk_gui_end_tooltip(void) {
    ImGui::EndTooltip();
}

/* ---------------------------------------------------------------------------
 * Popups
 * -------------------------------------------------------------------------*/

void hk_gui_open_popup(const char* id) {
    ImGui::OpenPopup(id);
}

int hk_gui_begin_popup(const char* id) {
    return ImGui::BeginPopup(id) ? 1 : 0;
}

int hk_gui_begin_popup_modal(const char* id) {
    return ImGui::BeginPopupModal(id, nullptr, ImGuiWindowFlags_AlwaysAutoResize) ? 1 : 0;
}

void hk_gui_close_popup(void) {
    ImGui::CloseCurrentPopup();
}

void hk_gui_end_popup(void) {
    ImGui::EndPopup();
}

/* ---------------------------------------------------------------------------
 * Menu bar
 * -------------------------------------------------------------------------*/

int hk_gui_begin_main_menu_bar(void) {
    return ImGui::BeginMainMenuBar() ? 1 : 0;
}

void hk_gui_end_main_menu_bar(void) {
    ImGui::EndMainMenuBar();
}

int hk_gui_begin_menu(const char* label) {
    return ImGui::BeginMenu(label) ? 1 : 0;
}

void hk_gui_end_menu(void) {
    ImGui::EndMenu();
}

int hk_gui_menu_item(const char* label) {
    return ImGui::MenuItem(label) ? 1 : 0;
}

/* ---------------------------------------------------------------------------
 * Tables
 * -------------------------------------------------------------------------*/

int hk_gui_begin_table(const char* id, int columns, int flags) {
    return ImGui::BeginTable(id, columns, (ImGuiTableFlags)flags) ? 1 : 0;
}

void hk_gui_end_table(void) {
    ImGui::EndTable();
}

void hk_gui_table_setup_column(const char* label) {
    ImGui::TableSetupColumn(label);
}

void hk_gui_table_headers_row(void) {
    ImGui::TableHeadersRow();
}

void hk_gui_table_next_row(void) {
    ImGui::TableNextRow();
}

void hk_gui_table_next_column(void) {
    ImGui::TableNextColumn();
}

/* ---------------------------------------------------------------------------
 * Theme — runtime color and geometry overrides
 * -------------------------------------------------------------------------*/

void hk_gui_set_color_text(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_Text]         = ImVec4((float)r, (float)g, (float)b, 1.00f);
    s.Colors[ImGuiCol_TextDisabled] = ImVec4((float)r, (float)g, (float)b, 0.55f);
}

void hk_gui_set_color_bg(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    float fr = (float)r, fg = (float)g, fb = (float)b;
    float cr = fr > 0.04f ? fr - 0.04f : 0.0f;
    float cg = fg > 0.04f ? fg - 0.04f : 0.0f;
    float cb = fb > 0.04f ? fb - 0.04f : 0.0f;
    s.Colors[ImGuiCol_WindowBg]          = ImVec4(fr, fg, fb, 1.0f);
    s.Colors[ImGuiCol_ChildBg]           = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TitleBg]           = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TitleBgCollapsed]  = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_MenuBarBg]         = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_ScrollbarBg]       = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TableHeaderBg]     = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_Tab]               = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TabDimmed]         = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TabDimmedSelected] = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_Button]            = ImVec4(cr, cg, cb, 1.0f);
    s.Colors[ImGuiCol_TabSelected]       = ImVec4(fr, fg, fb, 1.0f); /* selected tab = window bg */
    /* Keep the GL clear colour in sync */
    g_clear_r = fr; g_clear_g = fg; g_clear_b = fb;
}

void hk_gui_set_color_surface(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_FrameBg]  = ImVec4((float)r, (float)g, (float)b, 1.0f);
    s.Colors[ImGuiCol_PopupBg]  = ImVec4((float)r, (float)g, (float)b, 0.98f);
}

void hk_gui_set_color_border(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    float fr = (float)r, fg = (float)g, fb = (float)b;
    s.Colors[ImGuiCol_Border]              = ImVec4(fr, fg, fb, 1.0f);
    s.Colors[ImGuiCol_Separator]           = ImVec4(fr, fg, fb, 1.0f);
    s.Colors[ImGuiCol_ScrollbarGrab]       = ImVec4(fr, fg, fb, 0.80f);
    s.Colors[ImGuiCol_TableBorderStrong]   = ImVec4(fr, fg, fb, 1.0f);
    s.Colors[ImGuiCol_TableBorderLight]    = ImVec4(fr, fg, fb, 0.60f);
    s.Colors[ImGuiCol_FrameBgHovered]      = ImVec4(fr, fg, fb, 1.0f);
}

void hk_gui_set_color_accent(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    float fr = (float)r, fg = (float)g, fb = (float)b;
    /* Brighter variant for active/pressed states */
    float br = fr + 0.12f < 1.0f ? fr + 0.12f : 1.0f;
    float bg_ = fg + 0.12f < 1.0f ? fg + 0.12f : 1.0f;
    float bb = fb + 0.12f < 1.0f ? fb + 0.12f : 1.0f;
    /* Solid (checks, grabs, active) */
    s.Colors[ImGuiCol_CheckMark]             = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_SliderGrab]            = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_SliderGrabActive]      = ImVec4(br, bg_, bb, 1.00f);
    s.Colors[ImGuiCol_ButtonActive]          = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_HeaderActive]          = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_ResizeGripActive]      = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_SeparatorActive]       = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_TabSelectedOverline]   = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_NavHighlight]          = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_DragDropTarget]        = ImVec4(fr, fg, fb, 1.00f);
    /* Hover (18% alpha) */
    s.Colors[ImGuiCol_ButtonHovered]         = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_HeaderHovered]         = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_ResizeGripHovered]     = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_SeparatorHovered]      = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_TabHovered]            = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_TextSelectedBg]        = ImVec4(fr, fg, fb, 0.18f);
    s.Colors[ImGuiCol_TitleBgActive]         = ImVec4(fr, fg, fb, 0.18f);
    /* Dim (rest/subtle states) */
    s.Colors[ImGuiCol_ResizeGrip]            = ImVec4(fr, fg, fb, 0.10f);
    s.Colors[ImGuiCol_FrameBgActive]         = ImVec4(fr, fg, fb, 0.10f);
    s.Colors[ImGuiCol_Header]                = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    s.Colors[ImGuiCol_TableRowBgAlt]         = ImVec4(fr, fg, fb, 0.04f);
    s.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(fr, fg, fb, 0.70f);
}

void hk_gui_set_style_rounding(double window, double frame, double tab) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowRounding    = (float)window;
    s.FrameRounding     = (float)frame;
    s.TabRounding       = (float)tab;
    s.GrabRounding      = (float)frame;
    s.ScrollbarRounding = (float)window;
    s.PopupRounding     = (float)((window + frame) * 0.5f);
}

void hk_gui_set_style_padding(double frame_x, double frame_y) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.FramePadding = ImVec2((float)frame_x, (float)frame_y);
}

int hk_gui_color_button(const char* id, double r, double g, double b, double a,
                         double w, double h) {
    return ImGui::ColorButton(id, ImVec4((float)r,(float)g,(float)b,(float)a),
                               0, ImVec2((float)w,(float)h)) ? 1 : 0;
}

void hk_gui_set_clipboard(const char* text) {
    ImGui::SetClipboardText(text);
}

void hk_gui_set_color_plot(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    float fr = (float)r, fg = (float)g, fb = (float)b;
    float br = fr + 0.15f < 1.0f ? fr + 0.15f : 1.0f;
    float bg_ = fg + 0.15f < 1.0f ? fg + 0.15f : 1.0f;
    float bb = fb + 0.15f < 1.0f ? fb + 0.15f : 1.0f;
    s.Colors[ImGuiCol_PlotLines]        = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(br, bg_, bb, 1.00f);
}

void hk_gui_set_color_plot_bar(double r, double g, double b) {
    ImGuiStyle& s = ImGui::GetStyle();
    float fr = (float)r, fg = (float)g, fb = (float)b;
    float br = fr + 0.15f < 1.0f ? fr + 0.15f : 1.0f;
    float bg_ = fg + 0.15f < 1.0f ? fg + 0.15f : 1.0f;
    float bb = fb + 0.15f < 1.0f ? fb + 0.15f : 1.0f;
    s.Colors[ImGuiCol_PlotHistogram]        = ImVec4(fr, fg, fb, 1.00f);
    s.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(br, bg_, bb, 1.00f);
}

void hk_gui_set_color_modal_dim(double alpha) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.Colors[ImGuiCol_ModalWindowDimBg]  = ImVec4(0.0f, 0.0f, 0.0f, (float)alpha);
    s.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.0f, 0.0f, 0.0f, (float)alpha * 0.5f);
}

void hk_gui_set_style_window_padding(double x, double y) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = ImVec2((float)x, (float)y);
}

void hk_gui_set_style_spacing(double item_x, double item_y, double indent) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.ItemSpacing   = ImVec2((float)item_x, (float)item_y);
    s.IndentSpacing = (float)indent;
}

void hk_gui_set_style_borders(double window, double frame) {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowBorderSize = (float)window;
    s.FrameBorderSize  = (float)frame;
}

/* ---------------------------------------------------------------------------
 * ImPlot — 2-D plotting
 *
 * Series are identified by a FNV-1a hash of their label string.
 * Up to HK_MAX_SERIES distinct series are supported simultaneously.
 * Each series grows dynamically; data is retained across frames.
 * -------------------------------------------------------------------------*/

#define HK_MAX_SERIES 64

struct hk_plot_series {
    uint32_t            hash;
    std::vector<double> xs;
    std::vector<double> ys;
};

static hk_plot_series g_plot_series[HK_MAX_SERIES];
static int            g_plot_series_n = 0;

static hk_plot_series* plot_get_series(const char* label) {
    uint32_t h = fnv1a(label);
    for (int i = 0; i < g_plot_series_n; i++)
        if (g_plot_series[i].hash == h) return &g_plot_series[i];
    if (g_plot_series_n < HK_MAX_SERIES) {
        g_plot_series[g_plot_series_n] = { h, {}, {} };
        return &g_plot_series[g_plot_series_n++];
    }
    fprintf(stderr, "hica imgui: plot series table full (max %d)\n", HK_MAX_SERIES);
    return nullptr;
}

void hk_plot_push(const char* label, double y) {
    auto* s = plot_get_series(label);
    if (!s) return;
    s->xs.push_back((double)s->xs.size());
    s->ys.push_back(y);
}

void hk_plot_push_xy(const char* label, double x, double y) {
    auto* s = plot_get_series(label);
    if (!s) return;
    s->xs.push_back(x);
    s->ys.push_back(y);
}

void hk_plot_clear(const char* label) {
    auto* s = plot_get_series(label);
    if (!s) return;
    s->xs.clear();
    s->ys.clear();
}

void hk_plot_clear_all(void) {
    for (int i = 0; i < g_plot_series_n; i++) {
        g_plot_series[i].xs.clear();
        g_plot_series[i].ys.clear();
    }
}

int hk_plot_begin(const char* title, double w, double h) {
    return ImPlot::BeginPlot(title, ImVec2((float)w, (float)h)) ? 1 : 0;
}

void hk_plot_end(void) {
    ImPlot::EndPlot();
}

void hk_plot_setup_axes(const char* x_label, const char* y_label) {
    ImPlot::SetupAxes(x_label, y_label);
}

void hk_plot_setup_axis_limits(int axis, double v_min, double v_max, int cond) {
    ImPlotCond c = (cond == HK_PLOT_COND_ONCE) ? ImPlotCond_Once : ImPlotCond_Always;
    ImPlot::SetupAxisLimits((ImAxis)axis, v_min, v_max, c);
}

void hk_plot_line(const char* label) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotLine(label, s->xs.data(), s->ys.data(), (int)s->ys.size());
}

void hk_plot_shaded(const char* label, double y_ref) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotShaded(label, s->xs.data(), s->ys.data(), (int)s->ys.size(), y_ref);
}

void hk_plot_bars(const char* label, double bar_width) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotBars(label, s->xs.data(), s->ys.data(), (int)s->ys.size(), bar_width);
}

void hk_plot_scatter(const char* label) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotScatter(label, s->xs.data(), s->ys.data(), (int)s->ys.size());
}

void hk_plot_pie_chart(const char* labels_nl, const char* values_csv,
                       double cx, double cy, double radius) {
    /* Parse newline-separated labels */
    std::vector<std::string> label_strs;
    std::string tok;
    for (const char* p = labels_nl; ; ++p) {
        if (*p == '\n' || *p == '\0') {
            if (!tok.empty()) { label_strs.push_back(tok); tok.clear(); }
            if (*p == '\0') break;
        } else {
            tok += *p;
        }
    }

    /* Parse comma-separated values */
    std::vector<double> vals;
    std::string vtok;
    for (const char* p = values_csv; ; ++p) {
        if (*p == ',' || *p == '\0') {
            if (!vtok.empty()) { vals.push_back(std::stod(vtok)); vtok.clear(); }
            if (*p == '\0') break;
        } else {
            vtok += *p;
        }
    }

    int n = (int)std::min(label_strs.size(), vals.size());
    if (n <= 0) return;

    std::vector<const char*> label_ptrs;
    label_ptrs.reserve(n);
    for (int i = 0; i < n; i++) label_ptrs.push_back(label_strs[i].c_str());

    ImPlot::PlotPieChart(label_ptrs.data(), vals.data(), n, cx, cy, radius);
}

void hk_plot_stairs(const char* label) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotStairs(label, s->xs.data(), s->ys.data(), (int)s->ys.size());
}

void hk_plot_stems(const char* label, double ref) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    ImPlot::PlotStems(label, s->xs.data(), s->ys.data(), (int)s->ys.size(), ref);
}

void hk_plot_histogram(const char* label, int bins) {
    auto* s = plot_get_series(label);
    if (!s || s->ys.empty()) return;
    int b = (bins <= 0) ? ImPlotBin_Sturges : bins;
    ImPlot::PlotHistogram(label, s->ys.data(), (int)s->ys.size(), b);
}

void hk_plot_heatmap(const char* label, const char* values_csv,
                     int rows, int cols,
                     double scale_min, double scale_max) {
    /* Parse comma-separated values into a flat array */
    std::vector<double> vals;
    std::string tok;
    for (const char* p = values_csv; ; ++p) {
        if (*p == ',' || *p == '\0') {
            if (!tok.empty()) { vals.push_back(std::stod(tok)); tok.clear(); }
            if (*p == '\0') break;
        } else {
            tok += *p;
        }
    }
    if ((int)vals.size() < rows * cols) return;
    /* If scale_min == scale_max, let ImPlot auto-scale */
    double smin = scale_min, smax = scale_max;
    if (smin == smax) {
        smin = *std::min_element(vals.begin(), vals.end());
        smax = *std::max_element(vals.begin(), vals.end());
    }
    ImPlot::PlotHeatmap(label, vals.data(), rows, cols, smin, smax);
}

void hk_plot_inf_lines(const char* label) {
    auto* s = plot_get_series(label);
    if (!s || s->xs.empty()) return;
    ImPlot::PlotInfLines(label, s->xs.data(), (int)s->xs.size());
}

void hk_gui_open_url(const char* url) {
    SDL_OpenURL(url);
}

void hk_gui_hyperlink(const char* label, const char* url) {
    if (ImGui::TextLink(label))
        SDL_OpenURL(url);
}

int hk_gui_button_colored(const char* label,
                          double r, double g, double b,
                          double text_r, double text_g, double text_b) {
    auto clamp01 = [](float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); };
    float fr = (float)r, fg = (float)g, fb = (float)b;
    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(fr,                   fg,                   fb,                   1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(clamp01(fr + 0.15f), clamp01(fg + 0.15f), clamp01(fb + 0.15f), 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(clamp01(fr - 0.10f), clamp01(fg - 0.10f), clamp01(fb - 0.10f), 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4((float)text_r, (float)text_g, (float)text_b, 1.0f));
    bool clicked = ImGui::Button(label);
    ImGui::PopStyleColor(4);
    return clicked ? 1 : 0;
}
