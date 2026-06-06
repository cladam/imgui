// file-browser.hc — a minimal file browser built with hica + Dear ImGui
//
// Demonstrates: gui_begin_child / gui_end_child (scrollable panel),
// gui_selectable (stateless / caller-owns state),
// exec() for filesystem access.
//
// macOS / Linux only — uses ls / dirname shell commands.
//
// Build and run (from repo root):
//   ./build.sh
//   hica run examples/file-browser/file-browser.hc

import "../../src/imgui"

// ── Filesystem helpers ────────────────────────────────────────────────────

// List the contents of path using "ls -1Fp".
// Directory entries get a trailing "/" appended by ls.
fun list_dir(path: string) : list<string> {
  match exec("ls -1Fp \"" + path + "\" 2>/dev/null") {
    Ok(out) => filter(split(out, "\n"), (s) => s != "" && s != "./" && s != "../")
    Err(_) => []
  }
}

// Return the parent of path (never goes above "/").
fun parent_dir(path: string) : string {
  if path == "/" {
    "/"
  } else {
    match exec("dirname \"" + path + "\"") {
      Ok(p) =>
        // dirname output ends with a newline — grab only the first line
        match split(p, "\n") {
          [] => "/"
          [h, ..] => h
        }
      Err(_) => "/"
    }
  }
}

// True when an "ls -1Fp" entry represents a directory (trailing "/").
fun is_dir_entry(name: string) : bool {
  length(split(name, "/")) > 1
}

// Strip the trailing "/" from a directory entry name.
fun strip_slash(name: string) : string {
  match split(name, "/") {
    [] => name
    [h, ..] => h
  }
}

// Join a directory path with a basename entry (strips trailing "/" from name).
fun join_path(dir: string, name: string) : string {
  let clean = strip_slash(name)
  if dir == "/" { "/" + clean }
  else { dir + "/" + clean }
}

// ── Main ──────────────────────────────────────────────────────────────────
//
// Design notes:
//   - "ls -1Fp" appends "/" to directory entries; is_dir_entry checks for it.
//   - File list is rebuilt only when path changes (not every frame).
//   - gui_begin_child / gui_end_child are used directly (not gui_child) so
//     foreach is a top-level statement in the frame lambda — this avoids the
//     deeply-nested fn-inside-fn codegen that confuses Koka's layout parser.
//   - var assignments inside lambdas are safe; only "let" bindings hit the
//     open let-in-lambda codegen bug, so we use var for visible too.
//   - is_dir_entry is called inline in the foreach body to avoid "let".

fun main() {
  var path       = "/"
  var last_path  = "__init__"   // differs from path → triggers first refresh
  var files      = list_dir("/")
  var visible    = files        // filtered view, updated each frame
  var selected   = ""
  var filter_str = ""

  gui_window("File Browser", 520, 540, () => {
    // Rebuild the file list whenever the current path changes
    if path != last_path {
      files = list_dir(path)
      last_path = path
      selected = ""
    }

    // Recompute the filtered view each frame
    visible = filter(files, (f) => filter_str == "" || f.contains(filter_str))

    // ── Header ─────────────────────────────────────────────────────────────
    gui_text_colored("File Browser", 0.0, 0.749, 0.647, 1.0)
    gui_separator()
    gui_spacing()

    // ── Navigation bar ─────────────────────────────────────────────────────
    if gui_button("Up") { path = parent_dir(path) }
    gui_same_line()
    gui_text(path)
    gui_separator()
    gui_spacing()

    // ── Filter field ───────────────────────────────────────────────────────
    filter_str = gui_input_text("Filter##fb", 128)
    gui_spacing()

    // ── Scrollable file list ───────────────────────────────────────────────
    // gui_begin_child / gui_end_child are called at the frame level so the
    // foreach loop is a direct statement here (not inside a nested closure).
    if gui_begin_child("##filelist", 0.0, 300.0) { }
    foreach(visible, (f) => {
      if gui_selectable(
           (if is_dir_entry(f) { "> " } else { "  " }) + f,
           (if is_dir_entry(f) { "> " } else { "  " }) + f == selected) {
        if is_dir_entry(f) {
          path = join_path(path, f)
        } else {
          selected = (if is_dir_entry(f) { "> " } else { "  " }) + f
        }
      }
    })
    gui_end_child()

    // ── Status / action bar ─────────────────────────────────────────────────
    gui_separator()
    gui_spacing()
    if selected != "" {
      gui_text("Selected: " + strip_slash(selected))
      gui_same_line()
    } else {
      gui_text("> to navigate, click file to select")
      gui_same_line()
    }
    if gui_button("Open##fb") { }
    gui_same_line()
    if gui_button("Cancel##fb") { }
  })
}
