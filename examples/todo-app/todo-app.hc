// todo-app.hc — MVC-style TODO app built with hica + Dear ImGui
//
// Architecture:
//   Model:      todos.hml  — HML file, one @todo(text: "...", done: bool) per item
//   View:       Dear ImGui widgets rendered each frame
//   Controller: Add / toggle / delete actions that update + persist the model
//
// Build and run (from repo root):
//   ./build.sh
//   hica run examples/todo-app/todo-app.hc

import "../../src/imgui"

// ── Model ──────────────────────────────────────────────────────────────────

struct Todo {
  text: string,
  done: bool
}

// ── HML model parsing ─────────────────────────────────────────────────────
//
// Each line in todos.hml looks like:
//   @todo(text: 'Buy groceries', done: false)
//
// Single-quoted text values (HML literal strings) let us split on "'"
// without any escape sequences.

// Parse a single @todo line into a Todo.
fun parse_line(line: string) : Todo {
  let done = line.contains("done: true")
  match split(line, "'") {
    [_, ..rest] =>
      match rest {
        [text, ..] => Todo { text: text, done: done }
        _ => Todo { text: "", done: done }
      }
    _ => Todo { text: "", done: done }
  }
}

fun todos_of_lines(lines) {
  match lines {
    [] => []
    [h, ..rest] => [parse_line(h)] + todos_of_lines(rest)
  }
}

fun parse_todos(content) {
  let lines = filter(split(content, "\n"), (l) => l.contains("@todo("))
  todos_of_lines(lines)
}

// Format a single Todo as an HML @todo element on one line.
fun format_todo(t) : string {
  let done_str = if t.done { "true" } else { "false" }
  "@todo(text: '" + t.text + "', done: " + done_str + ")"
}

fun format_loop(todos) : string {
  match todos {
    [] => ""
    [h] => format_todo(h)
    [h, ..rest] => format_todo(h) + "\n" + format_loop(rest)
  }
}

fun load_todos() {
  match read_file("examples/todo-app/todos.hml") {
    Ok(content) => parse_todos(content)
    Err(_) => []
  }
}

fun save_todos(todos) {
  match write_file("examples/todo-app/todos.hml", format_loop(todos)) {
    Ok(_) => { }
    Err(_) => { }
  }
}

// ── List mutators ──────────────────────────────────────────────────────────

// Toggle the done field of the item at index target.
fun toggle_loop(todos, target: int, i: int) {
  match todos {
    [] => []
    [h, ..rest] =>
      if i == target {
        [Todo { text: h.text, done: !h.done }] + toggle_loop(rest, target, i + 1)
      } else {
        [h] + toggle_loop(rest, target, i + 1)
      }
  }
}

fun toggle_at(todos, target: int) {
  toggle_loop(todos, target, 0)
}

// Remove the item at index target.
fun delete_loop(todos, target: int, i: int) {
  match todos {
    [] => []
    [h, ..rest] =>
      if i == target {
        delete_loop(rest, target, i + 1)
      } else {
        [h] + delete_loop(rest, target, i + 1)
      }
  }
}

fun delete_at(todos, target: int) {
  delete_loop(todos, target, 0)
}

fun count_done(todos) : int {
  length(filter(todos, (t) => t.done))
}

fun count_active(todos) : int {
  length(todos) - count_done(todos)
}

fun all_done(todos) : bool {
  length(todos) > 0 && count_done(todos) == length(todos)
}

fun set_all_done_loop(todos, target: bool) {
  match todos {
    [] => []
    [h, ..rest] => [Todo { text: h.text, done: target }] + set_all_done_loop(rest, target)
  }
}

fun set_all_done(todos, target: bool) {
  set_all_done_loop(todos, target)
}

fun clear_completed(todos) {
  filter(todos, (t) => !t.done)
}

// ── Main ───────────────────────────────────────────────────────────────────

fun main() {
  var todos       = load_todos()
  var idx         = 0
  var filter_mode = 0   // 0 = All, 1 = Active, 2 = Completed

  gui_window("TodoMVC", 480, 580, () => {
    // ── Title ──────────────────────────────────────────────────────────────
    gui_text_colored("todos", 0.76, 0.35, 0.35, 1.0)
    gui_separator()
    gui_spacing()

    // ── Add row: [toggle-all] [input..............] [Add] ─────────────────
    let all = all_done(todos)
    let tog = gui_checkbox("##toggleall", all)
    if tog != all {
      todos = set_all_done(todos, !all)
      save_todos(todos)
    }
    gui_same_line()
    gui_push_item_width(-80.0)
    let new_text = gui_input_text("##newtodo", 256)
    gui_pop_item_width()
    gui_same_line()
    if gui_button("Add") && new_text != "" {
      todos = todos + [Todo { text: new_text, done: false }]
      save_todos(todos)
    }

    // ── List + footer (hidden when no todos) ───────────────────────────────
    if length(todos) > 0 {
      gui_spacing()
      gui_separator()

      if gui_begin_child("##list", 0.0, 340.0) { }
      idx = 0
      foreach(todos, (t) => {
        let show_item = filter_mode == 0 || (filter_mode == 1 && !t.done) || (filter_mode == 2 && t.done)
        if show_item {
          let cur_done = gui_checkbox("##cb_" + t.text, t.done)
          if cur_done != t.done {
            todos = toggle_at(todos, idx)
            save_todos(todos)
          }
          gui_same_line()
          if t.done {
            gui_text_colored(t.text, 0.55, 0.55, 0.55, 1.0)
          } else {
            gui_text(t.text)
          }
          gui_same_line()
          if gui_button("x##del_" + t.text) {
            todos = delete_at(todos, idx)
            save_todos(todos)
          }
        }
        idx = idx + 1
      })
      gui_end_child()

      // ── Footer ─────────────────────────────────────────────────────────
      gui_separator()
      gui_spacing()
      let active = count_active(todos)
      let word   = if active == 1 { "item left" } else { "items left" }
      gui_text(show(active) + " " + word)
      gui_same_line()
      if gui_button("All##f0")       { filter_mode = 0 }
      gui_same_line()
      if gui_button("Active##f1")    { filter_mode = 1 }
      gui_same_line()
      if gui_button("Completed##f2") { filter_mode = 2 }
      if count_done(todos) > 0 {
        gui_same_line()
        if gui_button("Clear completed") {
          todos = clear_completed(todos)
          filter_mode = 0
          save_todos(todos)
        }
      }
    }
    gui_spacing()
    gui_text_colored("examples/todo-app/todos.hml", 0.45, 0.45, 0.45, 1.0)
  })
}
