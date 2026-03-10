# lightTex

A fast, keyboard-first LaTeX editor built with C++ and Qt 6. Starts in milliseconds, compiles in the background, and stays out of your way.

**Status:** v0.2.1-beta (snippet tab-expansion, LSP autocomplete, find/replace, file tree, auto-compile)

## Why lightTex?

| | VS Code + LaTeX Workshop | Overleaf | TeXstudio | **lightTex** |
|---|---|---|---|---|
| **Stack** | Electron + TS | Web | C++ / Qt5 | **C++20 / Qt 6** |
| **RAM at idle** | ~400MB | ~500MB (browser) | ~200MB | **~50MB** |
| **Offline** | Yes | No | Yes | **Yes** |
| **Vim mode** | Plugin | No | No | **Planned (v0.3)** |
| **Plugin system** | Extensions | No | No | **Planned (v0.4, Lua)** |
| **License** | MIT | Proprietary | GPL-3.0 | **AGPL-3.0** |

## What works today (v0.2)

- **Editor**: Line numbers, bracket matching, current line highlight
- **Syntax highlighting**: Tree-sitter incremental parsing with sorted event cache + binary search
- **Compilation**: pdfLaTeX / XeLaTeX / LuaLaTeX via QProcess with error/warning/badbox parsing
- **PDF preview**: Split pane with Poppler-Qt6 or Qt6::Pdf (stub fallback)
- **Find/Replace**: Ctrl+F/Ctrl+H with regex support, match highlighting, replace all
- **File tree sidebar**: Ctrl+B toggle, filtered to LaTeX-relevant files, auto-detects project root
- **Configurable keybindings**: TOML-based (`keybindings.toml`), 8 defaults
- **Command palette**: Ctrl+Shift+P with fuzzy subsequence matching
- **Error panel**: Click-to-jump to error line in editor
- **Auto-compile**: Toggle via command palette, compiles on save
- **LaTeX snippets**: 18 built-in snippets — type `\frac`, `\begin`, etc. and press Tab to expand, then Tab/Shift+Tab through placeholders
- **texlab LSP**: Autocomplete (commands + arguments), hover, go-to-definition, diagnostics (requires texlab on PATH). LSP argument completion auto-triggers after snippet expansion and when cursor enters empty `{}` after a command
- **Themes**: Dark + Light TOML themes
- **Status bar**: Cursor position, compile status, engine, auto-compile indicator, LSP status
- **Undo/redo**: Full history with transaction support
- **201 tests** across 22 test suites, all passing

## Quick start

### Prerequisites

- **C++20 compiler**: MSVC 2022, GCC 12+, or Clang 15+
- **CMake** 3.21+
- **Qt 6.6+**: Install via your system package manager, Qt Online Installer, or vcpkg
- **A LaTeX distribution**: [TeX Live](https://www.tug.org/texlive/) or [MiKTeX](https://miktex.org/) for compilation
- **texlab** (optional): For LSP features — [download from GitHub releases](https://github.com/latex-lsp/texlab/releases)

### Build and run

```bash
# Configure (downloads tree-sitter, googletest, toml++ automatically via FetchContent)
cmake -B build

# Build
cmake --build build --config Release

# Run
./build/lightTex                    # Linux/macOS
.\build\Release\lightTex.exe       # Windows

# Open a file directly
./build/lightTex path/to/file.tex
```

### Run tests

```bash
cmake -B build -DLIGHTTEX_BUILD_TESTS=ON
cmake --build build --config Release
ctest --test-dir build --output-on-failure -C Release
```

On headless CI, set `QT_QPA_PLATFORM=offscreen` for widget tests.

### Run benchmarks

```bash
cmake -B build -DLIGHTTEX_BUILD_BENCHMARKS=ON
cmake --build build --config Release
./build/benchmarks/bench_piecetable
```

### Install dependencies by platform

**Ubuntu/Debian:**
```bash
sudo apt-get install qt6-base-dev libgl1-mesa-dev
# Optional for PDF rendering:
sudo apt-get install libpoppler-qt6-dev
```

**macOS:**
```bash
brew install qt@6
# Optional: brew install poppler
```

**Windows:**
Use the [Qt Online Installer](https://www.qt.io/download-qt-installer) or vcpkg:
```bash
vcpkg install qt6-base tomlplusplus
```

### Installing texlab (optional, for LSP)

texlab provides autocomplete, hover info, and go-to-definition for LaTeX.

**Download binary** (recommended): grab `texlab-x86_64-windows.zip` (or your platform) from [texlab releases](https://github.com/latex-lsp/texlab/releases), extract, and add to PATH.

**Scoop (Windows):** `scoop install texlab`

**Cargo (any platform):** `cargo install --git https://github.com/latex-lsp/texlab`

**Homebrew (macOS):** `brew install texlab`

lightTex detects texlab automatically. If not found, the status bar shows "texlab: not found" — everything else still works.

## Architecture

```
lightTex/
├── CMakeLists.txt                 # Top-level CMake
├── vcpkg.json                     # vcpkg manifest
├── cmake/                         # CMake modules (compiler flags, FetchContent)
├── src/
│   ├── main.cpp                   # QApplication entry point
│   ├── core/                      # PieceTable buffer, Selection, History, Document
│   ├── syntax/                    # tree-sitter LaTeX highlighting + QSyntaxHighlighter
│   ├── compiler/                  # QProcess compilation, log parser, SyncTeX
│   ├── pdf/                       # Poppler-Qt6 renderer, LRU page cache, PdfWidget
│   ├── theme/                     # TOML theme parser (toml++), ThemeManager
│   ├── editor/                    # EditorWidget, line numbers, brackets, FindReplaceBar
│   ├── ui/                        # MainWindow, StatusBar, CommandPalette, CompilePanel, FileTree
│   ├── shortcuts/                 # ShortcutManager (TOML keybindings)
│   ├── snippets/                  # SnippetManager, SnippetSession (tabstop cycling)
│   ├── lsp/                       # LspClient (texlab), JsonRpc, LspTypes, CompletionWidget
│   ├── app/                       # AppState, Actions (QAction registry)
│   └── {vim,plugin,i18n}/         # Stubs for future versions
├── themes/                        # dark.toml, light.toml
├── snippets/                      # latex.toml (18 default snippets)
├── tests/                         # 201 tests (Google Test + QTest)
├── benchmarks/                    # Google Benchmark for piece table
└── .github/workflows/             # CI + Release pipelines
```

**Key design decisions:**
- **Piece table buffer**: O(log n) edits — the same approach VS Code uses internally
- **Tree-sitter**: Incremental reparsing — sub-ms re-highlights after edits
- **Qt 6 Widgets**: Native look, no web runtime, ~50MB RAM at idle
- **QProcess**: Async compilation via Qt's event loop — no external async runtime needed
- **PDF rendering**: Poppler-Qt6 or Qt6::Pdf (native Qt), with stub fallback

## Keyboard shortcuts

| Shortcut | Action |
|---|---|
| Ctrl+O | Open file |
| Ctrl+S | Save file |
| Ctrl+Enter | Compile document |
| Ctrl+Shift+P | Command palette |
| Ctrl+F | Find |
| Ctrl+H | Find and replace |
| Ctrl+B | Toggle file tree sidebar |
| F12 | Go to definition (LSP) |
| Tab | Expand snippet / next tabstop |
| Shift+Tab | Previous tabstop |
| Ctrl+Z / Ctrl+Y | Undo / Redo |

All shortcuts are configurable via `keybindings.toml` (see `%APPDATA%/lighttex/keybindings.toml` on Windows, `~/.config/lighttex/keybindings.toml` on Linux/Mac).

## Test suite

201 tests covering all modules:

| Test file | Tests | Module |
|---|---|---|
| test_piecetable | 17 | Piece table insert/delete/replace/lines/unicode |
| test_selection | 7 | Cursor, range, contains, overlaps, normalize, clip |
| test_history | 5 | Undo/redo stacks, clear |
| test_document | 5 | File I/O, line ending detection, save roundtrip |
| test_highlighter | 17 | Tree-sitter parse, reparse, node classification, error handling |
| test_logparser | 8 | Error/warning/badbox regex, fixture files |
| test_synctex | 5 | Forward/inverse output parsing |
| test_pagecache | 5 | LRU eviction, access refresh |
| test_pdfrenderer | 6 | Renderer lifecycle, error handling, invalid files |
| test_theme | 8 | TOML parsing, stylesheet generation, dark/light themes |
| test_commandpalette | 5 | Show/hide/toggle, fuzzy matching, theme switching |
| test_editorwidget | 7 | Creation, text, cursor, theme switching |
| test_shortcuts | 8 | Default bindings, override, TOML loading |
| test_compilepanel | 4 | Messages, click-to-jump signal, show/hide |
| test_findreplace | 10 | Search highlights, bracket matcher, theme |
| test_filetree | 5 | Root path, theme, signal, header hidden |
| test_snippets | 13 | Snippet manager, expansion, tabstop session |
| test_jsonrpc | 6 | Encode/decode, Content-Length framing, partial data |
| test_lsptypes | 9 | Position, Range, CompletionItem, Hover, Diagnostic |
| test_completionwidget | 24 | LSP completion popup, triggers, signals, fromJson |
| test_autoindent | 12 | Auto-indentation, `\begin`/`\end` auto-pairing, undo |
| test_snippet_expansion | 15 | Tab-to-expand, tabstop cycling, cancel, undo, LSP signal |

## Benchmarks

Piece table performance (Google Benchmark, Release build, 12-core 3.7 GHz):

| Benchmark | 100 ops | 1,000 ops | 10,000 ops |
|---|---|---|---|
| Insert at end | 5.9 us | — | 35.9 ms |
| Insert at beginning | 3.1 us | — | 18.5 ms |
| Insert at random | 6.1 us | — | 40.8 ms |
| Delete + insert | 76.6 us | 906 us | 18.3 ms |

| Benchmark | 100 lines | 1,000 lines | 10,000 lines |
|---|---|---|---|
| Full text extraction | 0.46 us | — | 42.0 us |
| Char-to-line/col lookup | — | 8.75 ns | — |

```bash
cmake -B build -DLIGHTTEX_BUILD_BENCHMARKS=ON
cmake --build build --config Release
./build/benchmarks/Release/bench_piecetable    # Windows
./build/benchmarks/bench_piecetable             # Linux/macOS
```

## Roadmap

### v0.2 — "Actually Usable" (done)
- [x] Configurable keybindings (TOML -> QKeySequence)
- [x] Error/warning panel with click-to-jump
- [x] Auto-compile on save
- [x] Find/replace with regex
- [x] File tree sidebar
- [x] Built-in LaTeX snippets (TOML-defined, tab-trigger)
- [x] texlab LSP via QProcess + JSON-RPC (autocomplete, hover, goto-def, diagnostics)
- [x] Fuzzy command palette matching with scoring

### v0.3 — "Power User"
- [ ] Vim mode — C++ state machine (Normal/Insert/Visual/Command/Search)
- [ ] LaTeX text objects: `i$`/`a$`, `ie`/`ae`, `ic`/`ac`
- [ ] Registers, macros, marks, `.` repeat, ex-commands
- [ ] SyncTeX inverse search: double-click text in PDF preview → jump to that line in the editor
- [ ] SyncTeX forward search: Ctrl+click in editor → highlight corresponding position in PDF
- [ ] Multi-file projects (\input/\include tracking)
- [ ] BibTeX integration + citation completion
- [ ] i18n via QTranslator

### v0.4 — "Extensible"
- [ ] Lua plugin system (Sol2 + LuaJIT)
- [ ] Plugin API: buffer ops, UI, commands, keybindings, filesystem
- [ ] `vim` namespace mimics `vim.api.*` for Neovim plugin portability
- [ ] Visual table editor, git integration
- [ ] Multiple tabs (QTabWidget), session persistence

### v0.5 — "AI & Polish"
- [ ] AI plugin (Lua, wrapping Ollama/cloud APIs)
- [ ] Performance optimization pass
- [ ] Accessibility (Qt Accessibility framework)
- [ ] Autosave (debounced, configurable interval, dirty indicator, backup copies)
- [ ] Auto-updater

### v1.0 — "Release"
- [ ] >70% test coverage, benchmarks published
- [ ] First-run setup wizard: auto-detect and install missing prerequisites (LaTeX distribution, texlab, etc.)
- [ ] Platform installers (.msi, .dmg, .deb, .AppImage)
- [ ] Homebrew, Scoop, Flathub distribution
- [ ] User guide + Lua plugin developer guide

## Tech stack

| Layer | Technology |
|---|---|
| Language | C++20 |
| GUI | Qt 6 Widgets |
| Text buffer | Piece table (custom) |
| Syntax | tree-sitter (C API) |
| PDF rendering | Poppler-Qt6 or Qt6::Pdf |
| TOML parsing | toml++ |
| Compilation | QProcess |
| Source-PDF sync | SyncTeX |
| LSP | texlab via JSON-RPC |
| Testing | Google Test + QTest |
| Build | CMake |
| Future: Plugins | Sol2 + LuaJIT (v0.4) |

## Contributing

Contributions welcome. Please run `clang-format` before submitting PRs.

## License

[AGPL-3.0](LICENSE) — enables direct Poppler and MuPDF usage for PDF rendering.
