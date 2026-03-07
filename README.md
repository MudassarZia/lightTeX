# lightTex

A fast, keyboard-first LaTeX editor built with Rust. Starts in milliseconds, compiles in the background, and stays out of your way.

**Status:** v0.1.0-beta (MVP scaffold — editor + compiler pipeline working)

## Why lightTex?

| | VS Code + LaTeX Workshop | Overleaf | TeXstudio | **lightTex** |
|---|---|---|---|---|
| **Installer size** | ~300MB | N/A (web) | ~150MB | **~13MB** |
| **RAM at idle** | ~400MB | ~500MB (browser) | ~200MB | **~40MB** |
| **Frontend bundle** | ~20MB (Electron) | N/A | N/A | **312KB** |
| **Insert into 100KB doc** | ~1ms | N/A | ~1ms | **41µs** |
| **Offline** | Yes | No | Yes | **Yes** |
| **Vim mode** | Plugin | No | No | **Planned (v0.3)** |
| **Plugin system** | Extensions | No | No | **Planned (v0.4)** |
| **License** | MIT | Proprietary | GPL-3.0 | **AGPL-3.0** |

## What works today (v0.1)

- **Editor**: CodeMirror 6 with syntax-aware editing, bracket matching, search, undo/redo
- **Syntax highlighting**: Tree-sitter incremental parsing for LaTeX
- **Compilation**: pdfLaTeX/XeLaTeX/LuaLaTeX pipeline with structured error/warning parsing
- **SyncTeX**: Forward search (source → PDF position)
- **PDF preview**: Split pane (stub renderer — MuPDF integration next)
- **Themes**: Dark + Light TOML themes with hot CSS variable mapping
- **Status bar**: Line/col, compile status, engine selector

## Performance

Benchmarked on a standard desktop (see `crates/ltx-core/benches/`):

| Operation | Time |
|---|---|
| Insert into 1KB document | **1.9µs** |
| Insert into 100KB document | **41µs** |
| Insert into 10MB document | **11ms** |
| Delete range (100KB doc) | **43µs** |
| 100× undo + 100× redo cycle | **343µs** |
| Binary size (release, stripped) | **13MB** |
| Frontend bundle | **312KB** |

## Quick start

### Prerequisites

- [Rust](https://rustup.rs/) (stable)
- [Node.js](https://nodejs.org/) (v20+)
- A LaTeX distribution ([TeX Live](https://www.tug.org/texlive/), [MiKTeX](https://miktex.org/), etc.) for compilation

### Run in development mode

```bash
# Install frontend dependencies
cd frontend && npm install && cd ..

# Run with hot-reload
cargo tauri dev
```

### Build for release

```bash
cargo tauri build
```

The binary appears in `target/release/`. Platform installers (.msi, .dmg, .deb, .AppImage) go to `target/release/bundle/`.

### Run tests

```bash
cargo fmt --check
cargo clippy --workspace -- -D warnings
cargo test --workspace
cd frontend && npm run check
```

### Run benchmarks

```bash
cargo bench --bench rope_bench
```

## Architecture

```
lightTex/
├── crates/
│   ├── ltx-core/        # Rope buffer, selections, undo/redo, document model
│   ├── ltx-syntax/      # Tree-sitter LaTeX highlighting
│   ├── ltx-compiler/    # Compilation pipeline, log parser, SyncTeX
│   ├── ltx-pdf/         # PDF renderer (MuPDF planned), page cache
│   ├── ltx-theme/       # TOML theme parser, built-in themes
│   ├── ltx-app/         # Tauri v2 entry point, IPC commands
│   ├── ltx-lsp/         # texlab LSP client (v0.2)
│   ├── ltx-vim/         # Modal editing (v0.3)
│   ├── ltx-plugin/      # WASM + Lua plugins (v0.4)
│   ├── ltx-i18n/        # Internationalization (v0.3)
│   ├── ltx-snippets/    # Snippet engine (v0.2)
│   └── ltx-shortcuts/   # Keybinding registry (v0.2)
├── frontend/            # SolidJS + CodeMirror 6
└── themes/              # dark.toml, light.toml
```

**Key design decisions:**
- **Rope buffer** (ropey): O(log n) edits — microsecond operations on GB files
- **Tree-sitter**: Incremental reparsing — sub-ms re-highlights after edits
- **Tauri v2**: Native webview, ~40MB RAM, no Electron overhead
- **EditorSurface abstraction**: CodeMirror 6 now, custom canvas renderer later (no backend changes needed)

## Roadmap

### v0.2 — "Actually Usable"
- [ ] texlab LSP: completions, hover, goto-def, diagnostics
- [ ] Multi-engine auto-compile on save
- [ ] SyncTeX inverse search (PDF → editor)
- [ ] File tree sidebar, document outline, command palette
- [ ] Built-in LaTeX snippets
- [ ] Find & replace (regex)
- [ ] Configurable keybindings (TOML)
- [ ] Error/warning panel

### v0.3 — "Power User"
- [ ] Full vim emulation (modes, operators, motions, registers, macros, marks)
- [ ] LaTeX-specific text objects (`i$`, `ae`, `ic`)
- [ ] Sioyek-style PDF: keyboard nav, smart jump, marks, text search
- [ ] Multi-file projects (\input/\include tracking)
- [ ] BibTeX integration + citation completion
- [ ] Inline math preview (KaTeX)
- [ ] Internationalization (English + 3 languages)

### v0.4 — "Extensible"
- [ ] WASM plugin host (Extism) + Lua scripting (mlua)
- [ ] Plugin API with manifest format
- [ ] Visual table editor
- [ ] Git integration (status, diff, commit)
- [ ] Find in project (multi-file search)
- [ ] Embedded terminal
- [ ] Multiple tabs/windows, session persistence

### v0.5 — "AI & Polish"
- [ ] AI plugin: Ollama (local) + cloud APIs
- [ ] LaTeX error explanation, smart completions
- [ ] Performance optimization pass
- [ ] Accessibility audit, auto-updater

### v1.0 — "Release"
- [ ] >70% test coverage, performance benchmarks vs competitors
- [ ] Windows .msi, macOS .dmg, Linux .deb + .AppImage
- [ ] Homebrew, Scoop, Flathub distribution
- [ ] User guide + plugin developer guide

### Post-v1.0
- [ ] Real-time collaboration (Loro CRDT + P2P)
- [ ] Custom canvas-based editor renderer (replace CodeMirror 6)
- [ ] Mobile support (Tauri v2 mobile)

## Tech stack

| Layer | Technology |
|---|---|
| Core | Rust |
| App framework | Tauri v2 |
| Frontend | SolidJS + TypeScript |
| Text editor | CodeMirror 6 |
| Text buffer | ropey |
| Syntax | tree-sitter + tree-sitter-latex |
| LaTeX intelligence | texlab (LSP) — planned |
| PDF rendering | MuPDF (mupdf-rs) — planned |
| Source-PDF sync | SyncTeX |
| Plugins | Extism (WASM) + mlua (Lua) — planned |
| Theming | TOML files |

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) (coming soon).

## License

[AGPL-3.0](LICENSE) — enables direct MuPDF usage for PDF rendering.
