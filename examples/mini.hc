import "../src/imgui"

fun main() {
  var count = 0

  gui_window("My App", 400, 300, () => {
    gui_text("Counter: " + show(count))
    if gui_button("+ Increment") {
      count = count + 1
    }
    gui_same_line()
    if gui_button("Reset") {
      count = 0
    }
  })
}