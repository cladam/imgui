// heatmap.hc — Heatmap demo for hica
//
// A heatmap encodes a 2-D grid of values as colour intensity — useful for
// correlation matrices, confusion matrices, or any 2-D measurement grid.
//
// This example shows a 6×6 feature-correlation matrix where 1.0 = perfectly
// correlated and 0.0 = no correlation.
//
// Run:
//   hica run examples/heatmap/heatmap.hc

import "../../src/imgui"

// ── 6×6 correlation matrix (row-major, comma-separated) ─────────────────────
//
//        F1    F2    F3    F4    F5    F6
// F1 [ 1.00, 0.82, 0.61, 0.37, 0.15, 0.05 ]
// F2 [ 0.82, 1.00, 0.74, 0.52, 0.28, 0.10 ]
// F3 [ 0.61, 0.74, 1.00, 0.69, 0.44, 0.20 ]
// F4 [ 0.37, 0.52, 0.69, 1.00, 0.71, 0.39 ]
// F5 [ 0.15, 0.28, 0.44, 0.71, 1.00, 0.63 ]
// F6 [ 0.05, 0.10, 0.20, 0.39, 0.63, 1.00 ]

fun corr_csv() {
  "1.00,0.82,0.61,0.37,0.15,0.05,0.82,1.00,0.74,0.52,0.28,0.10,0.61,0.74,1.00,0.69,0.44,0.20,0.37,0.52,0.69,1.00,0.71,0.39,0.15,0.28,0.44,0.71,1.00,0.63,0.05,0.10,0.20,0.39,0.63,1.00"
}

// ── 4×4 confusion matrix: Actual (rows) vs Predicted (cols) ─────────────────
//
//              Dog   Cat  Bird  Fish
// Dog  [ 95,   3,   1,   1 ]
// Cat  [  4,  88,   5,   3 ]
// Bird [  2,   6,  89,   3 ]
// Fish [  1,   2,   4,  93 ]

fun confusion_csv() {
  "95,3,1,1,4,88,5,3,2,6,89,3,1,2,4,93"
}

// ── main ─────────────────────────────────────────────────────────────────────

fun main() {
  gui_window("Heatmap Examples", 900, 680, () => {
    gui_text("Feature Correlation Matrix (6 x 6)")
    gui_text_colored("Diagonal = 1.0 (self-correlation).  Brighter = more correlated.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##corr", 0.0, 280.0, () => {
      plot_setup_axes("feature", "feature")
      plot_setup_axis_limits(0, -0.5, 5.5, 1)   // X1 → 6 columns
      plot_setup_axis_limits(3, -0.5, 5.5, 1)   // Y1 → 6 rows
      // scale 0.0..1.0 — auto (pass equal values 0.0,0.0)
      plot_heatmap("Correlation", corr_csv(), 6, 6, 0.0, 1.0)
    })

    gui_separator()
    gui_spacing()

    gui_text("Confusion Matrix — 4-class Classifier (Dog / Cat / Bird / Fish)")
    gui_text_colored("Rows = actual class, Columns = predicted class.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##confusion", 0.0, 260.0, () => {
      plot_setup_axes("predicted", "actual")
      plot_setup_axis_limits(0, -0.5, 3.5, 1)   // X1 → 4 columns
      plot_setup_axis_limits(3, -0.5, 3.5, 1)   // Y1 → 4 rows
      // Auto scale → pass 0.0, 0.0
      plot_heatmap("Confusion", confusion_csv(), 4, 4, 0.0, 0.0)
    })

    gui_spacing()
    gui_text_colored("Tip: hover over a cell to read its exact value.", 0.4, 0.6, 1.0, 1.0)
  })
}
