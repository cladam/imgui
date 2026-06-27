// stairstep.hc — Stairstep & Stem plot demo for hica
//
// Stairstep: a step-line chart that holds each value constant until the next
// sample — ideal for discrete state machines, protocol timings, or PWM signals.
//
// Stems: vertical lines from a baseline to each sample — classic for showing
// impulse responses or discrete event distributions.
//
// Run:
//   hica run examples/stairstep/stairstep.hc

import "../../src/imgui"

// ── data: simulated CPU utilisation over 16 seconds ─────────────────────────

fun push_cpu_data() {
  plot_push_xy("CPU %",  0.0, 12.0)
  plot_push_xy("CPU %",  1.0, 15.0)
  plot_push_xy("CPU %",  2.0, 68.0)
  plot_push_xy("CPU %",  3.0, 72.0)
  plot_push_xy("CPU %",  4.0, 71.0)
  plot_push_xy("CPU %",  5.0, 20.0)
  plot_push_xy("CPU %",  6.0, 18.0)
  plot_push_xy("CPU %",  7.0, 85.0)
  plot_push_xy("CPU %",  8.0, 90.0)
  plot_push_xy("CPU %",  9.0, 88.0)
  plot_push_xy("CPU %", 10.0, 35.0)
  plot_push_xy("CPU %", 11.0, 30.0)
  plot_push_xy("CPU %", 12.0, 10.0)
  plot_push_xy("CPU %", 13.0, 10.0)
  plot_push_xy("CPU %", 14.0,  8.0)
  plot_push_xy("CPU %", 15.0, 14.0)
}

// ── data: simulated network packet inter-arrival times (ms) ─────────────────

fun push_packet_data() {
  plot_push_xy("Packets",  0.0, 2.3)
  plot_push_xy("Packets",  1.0, 5.1)
  plot_push_xy("Packets",  2.0, 1.8)
  plot_push_xy("Packets",  3.0, 8.4)
  plot_push_xy("Packets",  4.0, 3.2)
  plot_push_xy("Packets",  5.0, 6.7)
  plot_push_xy("Packets",  6.0, 0.9)
  plot_push_xy("Packets",  7.0, 4.5)
  plot_push_xy("Packets",  8.0, 7.1)
  plot_push_xy("Packets",  9.0, 2.6)
  plot_push_xy("Packets", 10.0, 5.8)
  plot_push_xy("Packets", 11.0, 1.4)
}

// ── main ─────────────────────────────────────────────────────────────────────

fun main() {
  push_cpu_data()
  push_packet_data()

  gui_window("Stairstep & Stem Plots", 860, 640, () => {
    gui_text("Stairstep Plot — CPU Utilisation over Time")
    gui_text_colored("Each sample holds its value until the next — no interpolation.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##cpu_stairs", 0.0, 180.0, () => {
      plot_setup_axes("time (s)", "CPU %")
      plot_setup_axis_limits(0,  0.0, 15.0, 1)   // X1 → 0..15 s
      plot_setup_axis_limits(3,  0.0, 100.0, 1)  // Y1 → 0..100 %
      plot_stairs("CPU %")
    })

    gui_separator()
    gui_spacing()

    gui_text("Stem Plot — Network Packet Inter-Arrival Times")
    gui_text_colored("Vertical lines from baseline 0 to each sample value.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##packets_stems", 0.0, 180.0, () => {
      plot_setup_axes("sample", "delay (ms)")
      plot_setup_axis_limits(0,  -0.5, 11.5, 1)  // X1 → sample range
      plot_setup_axis_limits(3,   0.0, 10.0, 1)  // Y1 → 0..10 ms
      plot_stems("Packets", 0.0)
    })

    gui_separator()
    gui_spacing()

    gui_text("Combined — CPU as stairstep + packet stems side by side")
    gui_text_colored("Both series share the x-axis; y-axes auto-fit.", 0.5, 0.5, 0.5, 1.0)
    gui_spacing()

    gui_plot("##combined", 0.0, 180.0, () => {
      plot_setup_axes("index", "value")
      plot_stairs("CPU %")
      plot_stems("Packets", 0.0)
    })
  })
}
