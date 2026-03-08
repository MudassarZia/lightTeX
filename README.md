# lightTex

A fast, keyboard-first LaTeX editor built with C++ and Qt 6. Starts in milliseconds, compiles in the background, and stays out of your way.

**Status:** v0.1.0-beta (C++ Qt rewrite — editor + compiler pipeline + PDF preview working)

## Why lightTex?

| | VS Code + LaTeX Workshop | Overleaf | TeXstudio | **lightTex** |
|---|---|---|---|---|
| **Stack** | Electron + TS | Web | C++ / Qt5 | **C++20 / Qt 6** |
| **RAM at idle** | ~400MB | ~500MB (browser) | ~200MB | **~50MB** |
| **Offline** | Yes | No | Yes | **Yes** |
| **Vim mode** | Plugin | No | No | **Planned (v0.3)** |
| **Plugin system** | Extensions | No | No | **Planned (v0.4, Lua)** |
| **License** | MIT | Proprietary | GPL-3.0 | **AGPL-3.0** |

## What works today (v0.1)

- **Editor**: QPlainTextEdit subclass with line numbers, bracket matching, current line highlight
- **Syntax highlighting**: Tree-sitter incremental parsing for LaTeX via QSyntaxHighlighter bridge
- **Compilation**: pdfLaTeX / XeLaTeX / LuaLaTeX pipeline via QProcess with structured error/warning/badbox parsing
- **PDF preview**: Split pane with Poppler-Qt6 or Qt6::Pdf rendering (stub fallback if neither installed)
- **SyncTeX**: Forward/inverse output parsing (UI wiring in v0.2)
- **Themes**: Dark + Light TOML themes applied via Qt stylesheets
- **Status bar**: Line/col, compile status (color-coded), engine name, encoding
- **Command palette**: Ctrl+Shift+P overlay with fuzzy filtering and keyboard navigation
- **File I/O**: Native file dialogs (QFileDialog), LF/CRLF detection + normalization
- **Undo/redo**: Full history with transaction support

## Quick start

### Prerequisites

- **C++20 compiler**: MSVC 2022, GCC 12+, or Clang 15+
- **CMake** 3.21+
- **Qt 6.6+**: Install via your system package manager, Qt Online Installer, or vcpkg
- **A LaTeX distribution**: [TeX Live](https://www.tug.org/texlive/), [MiKTeX](https://miktex.org/), etc. for compilation

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
│   ├── editor/                    # EditorWidget (QPlainTextEdit), line numbers, brackets
│   ├── ui/                        # MainWindow, StatusBar, CommandPalette, CompilePanel
│   ├── app/                       # AppState, Actions (QAction registry)
│   └── {lsp,vim,plugin,...}/      # Stubs for future versions
├── themes/                        # dark.toml, light.toml
├── tests/                         # 95 tests (Google Test + QTest)
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
| Ctrl+Z / Ctrl+Y | Undo / Redo |

## Test suite

95 tests covering all modules:

| Test file | Tests | Module |
|---|---|---|
| test_piecetable | 18 | Piece table insert/delete/replace/lines/unicode |
| test_selection | 7 | Cursor, range, contains, overlaps, normalize, clip |
| test_history | 5 | Undo/redo stacks, clear |
| test_document | 5 | File I/O, line ending detection, save roundtrip |
| test_highlighter | 17 | Tree-sitter parse, reparse, node classification, error handling |
| test_logparser | 8 | Error/warning/badbox regex, fixture files |
| test_synctex | 5 | Forward/inverse output parsing |
| test_pagecache | 5 | LRU eviction, access refresh |
| test_pdfrenderer | 6 | Renderer lifecycle, error handling, invalid files |
| test_theme | 8 | TOML parsing, stylesheet generation, dark/light themes |
| test_commandpalette | 5 | Show/hide/toggle, theme switching (widget test) |
| test_editorwidget | 7 | Creation, text, cursor, theme switching (widget test) |

## Roadmap

### v0.2 — "Actually Usable"
- [ ] texlab LSP via QProcess + JSON-RPC (completions, hover, goto-def)
- [ ] Auto-compile on save (QFileSystemWatcher)
- [ ] File tree sidebar (QTreeView + QFileSystemModel)
- [ ] Built-in LaTeX snippets (TOML-defined, tab-trigger)
- [ ] Find/replace with regex
- [ ] Configurable keybindings (TOML -> QKeySequence)
- [ ] Error/warning panel with click-to-jump

### v0.3 — "Power User"
- [ ] Vim mode — C++ state machine (Normal/Insert/Visual/Command/Search)
- [ ] LaTeX text objects: `i$`/`a$`, `ie`/`ae`, `ic`/`ac`
- [ ] Registers, macros, marks, `.` repeat, ex-commands
- [ ] SyncTeX wired to UI (click PDF -> jump to editor, and vice versa)
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
- [ ] Auto-updater

### v1.0 — "Release"
- [ ] >70% test coverage, benchmarks published
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
| Testing | Google Test + QTest |
| Build | CMake |
| Future: Plugins | Sol2 + LuaJIT (v0.4) |
| Future: LSP | texlab via JSON-RPC (v0.2) |

## Contributing

Contributions welcome. Please run `clang-format` before submitting PRs.

## License

[AGPL-3.0](LICENSE) — enables direct Poppler and MuPDF usage for PDF rendering.
