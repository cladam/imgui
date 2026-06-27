// histogram.hc — Histogram & Infinite Lines demo for hica
//
// Histogram: shows the distribution of a dataset by counting how many values
// fall into each bin.  ImPlot auto-sizes bins using Sturges' rule (bins=0).
//
// Infinite lines: vertical reference lines spanning the full plot height —
// handy for marking thresholds, means, or event times.
//
// Run:
//   hica run examples/histogram/histogram.hc

import "../../src/imgui"

// ── data: response-time measurements (ms) — roughly log-normal ──────────────

fun push_response_times() {
  // Fast responses — concentrated around 12–40 ms
  plot_push("Response",  8.2)
  plot_push("Response", 11.4)
  plot_push("Response", 13.0)
  plot_push("Response", 14.5)
  plot_push("Response", 15.1)
  plot_push("Response", 15.8)
  plot_push("Response", 16.3)
  plot_push("Response", 17.0)
  plot_push("Response", 18.2)
  plot_push("Response", 19.4)
  plot_push("Response", 20.1)
  plot_push("Response", 21.3)
  plot_push("Response", 22.0)
  plot_push("Response", 23.5)
  plot_push("Response", 24.1)
  plot_push("Response", 25.6)
  plot_push("Response", 26.2)
  plot_push("Response", 27.8)
  plot_push("Response", 29.0)
  plot_push("Response", 31.4)
  plot_push("Response", 33.9)
  plot_push("Response", 36.0)
  plot_push("Response", 38.5)
  // Slow tail — occasional outliers
  plot_push("Response", 55.0)
  plot_push("Response", 62.3)
  plot_push("Response", 78.0)
  plot_push("Response", 91.4)
  plot_push("Response", 120.0)
}

// ── threshold lines ───────────────────────────────────────────────────────────

fun push_p95_line() {
  // Mark the p95 threshold at x = 78 ms
  plot_push_xy("p95", 78.0, 0.0)
}

fun push_slo_line() {
  // Mark the SLO boundary at x = 100 ms
  plot_push_xy("SLO (100 ms)", 100.0, 0.0)
}

// ── main ─────────────────────────────────────────────────────────────────────

fun main() {
  push_response_times()
  push_p95_line()
  push_slo_line()

  gui_window("Histogram & Infinite Lines", 860, 560, () => {
    gui_text("Histogram — HTTP Response Times")
    gui_text_colored("Auto-binned (Sturges' rule).  Vertical lines mark p95 and SLO.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##resp_hist", 0.0, 280.0, () => {
      plot_setup_axes("latency (ms)", "count")
      plot_setup_axis_limits(0,   0.0, 140.0, 1)  // X1 → 0..140 ms
      // Y1 auto-fit — don't set limits so ImPlot scales to bar heights
      plot_histogram("Response", 0)
      plot_inf_lines("p95")
      plot_inf_lines("SLO (100 ms)")
    })

    gui_separator()
    gui_spacing()

    gui_text("Interpreting the chart")
    gui_bullet_text("Most requests complete in the 12–40 ms range.")
    gui_bullet_text("The distribution has a long tail — a few requests exceed 100 ms.")
    gui_bullet_text("The p95 line (78 ms) and SLO line (100 ms) are infinite vertical markers.")
    gui_spacing()
    gui_text_colored("Tip: hover over the plot to read exact bin counts.", 0.4, 0.6, 1.0, 1.0)
  })
}
