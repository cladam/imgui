// chart.hc — ImPlot demo for hica
//
// Demonstrates the four main plot types:
//   · Line + shaded area   (temperature trend)
//   · Bar chart            (monthly sales)
//   · Scatter plot         (two-cluster distribution)
//   · Pie chart            (market-share breakdown)
//
// Run:
//   cd examples/chart && hica run

import "../../src/imgui"

// ── seed data ───────────────────────────────────────────────────────────────

fun push_line_data() {
  // Temperature readings over 10 hours
  plot_push_xy("Temp",  0.0, 18.2)
  plot_push_xy("Temp",  1.0, 19.5)
  plot_push_xy("Temp",  2.0, 21.3)
  plot_push_xy("Temp",  3.0, 23.0)
  plot_push_xy("Temp",  4.0, 24.8)
  plot_push_xy("Temp",  5.0, 25.1)
  plot_push_xy("Temp",  6.0, 24.3)
  plot_push_xy("Temp",  7.0, 22.7)
  plot_push_xy("Temp",  8.0, 20.9)
  plot_push_xy("Temp",  9.0, 19.4)
  plot_push_xy("Temp", 10.0, 18.8)
}

fun push_bar_data() {
  // Monthly sales Jan–Jun (y values; x auto-increments 0..5)
  plot_push("Sales", 42.0)
  plot_push("Sales", 67.0)
  plot_push("Sales", 55.0)
  plot_push("Sales", 80.0)
  plot_push("Sales", 74.0)
  plot_push("Sales", 91.0)
}

fun push_scatter_data() {
  // Simple cluster of points
  plot_push_xy("Cluster A",  1.2, 3.4)
  plot_push_xy("Cluster A",  1.8, 2.9)
  plot_push_xy("Cluster A",  0.9, 3.8)
  plot_push_xy("Cluster A",  2.1, 3.1)
  plot_push_xy("Cluster A",  1.5, 4.0)
  plot_push_xy("Cluster B",  5.0, 1.5)
  plot_push_xy("Cluster B",  5.8, 2.0)
  plot_push_xy("Cluster B",  4.7, 1.2)
  plot_push_xy("Cluster B",  6.1, 1.8)
  plot_push_xy("Cluster B",  5.4, 0.9)
}

// ── main ────────────────────────────────────────────────────────────────────

fun main() {
  // Seed series data once before the window opens.
  push_line_data()
  push_bar_data()
  push_scatter_data()

  gui_window("ImPlot Demo", 900, 700, () => {
    // ── header ────────────────────────────────────────────────────────────
    gui_text("ImPlot integration — four chart types")
    gui_separator()
    gui_spacing()

    // ── line + shaded area ───────────────────────────────────────────────
    gui_text("Line & Shaded Area — Temperature (°C)")
    gui_plot("##temp", 0.0, 160.0, () => {
      plot_setup_axes("hour", "deg C")
      plot_setup_axis_limits(0, 0.0,  10.0, 1)   // X1 (=0): 0..10 h, set once
      plot_setup_axis_limits(3, 0.0,  30.0, 1)   // Y1 (=3): 0..30°C, set once
      plot_shaded("Temp", 0.0)
      plot_line("Temp")
    })

    gui_spacing()

    // ── bar chart ────────────────────────────────────────────────────────
    gui_text("Bar Chart — Monthly Sales (Jan–Jun)")
    gui_plot("##sales", 0.0, 160.0, () => {
      plot_setup_axes("month", "units")
      plot_setup_axis_limits(3, 0.0, 110.0, 1)   // Y1 (=3): 0..110, set once
      plot_bars("Sales", 0.67)
    })

    gui_spacing()

    // ── scatter plot ─────────────────────────────────────────────────────
    gui_text("Scatter Plot — Two Clusters")
    gui_plot("##scatter", 0.0, 160.0, () => {
      plot_setup_axes("x", "y")
      plot_scatter("Cluster A")
      plot_scatter("Cluster B")
    })

    gui_spacing()

    // ── pie chart ────────────────────────────────────────────────────────
    gui_text("Pie Chart — Market Share")
    gui_plot("##pie", 0.0, 180.0, () => {
      plot_pie_chart("hica\nKoka\nRust\nOther", "45,25,20,10", 0.5, 0.5, 0.4)
    })
  })
}
