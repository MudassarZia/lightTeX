## lightTex v0.1.0-beta.1 — "Hello, LaTeX"

First public beta. Core scaffold is in place — the editor opens, highlights LaTeX, and compiles to PDF.

### What works
- CodeMirror 6 editor with tree-sitter LaTeX syntax highlighting
- pdfLaTeX / XeLaTeX / LuaLaTeX compilation with structured error/warning parsing
- SyncTeX forward search (source → PDF position)
- Dark + Light themes (TOML-based, hot-swappable)
- Undo/redo with full history
- Status bar (line/col, compile status, engine)
- 13MB binary, 312KB frontend bundle, ~40MB RAM

### What doesn't work yet
- PDF preview is a placeholder (MuPDF integration pending)
- No file dialog — must type path manually
- No auto-compile on save
- No LSP completions/diagnostics
- No vim mode, snippets, or keybinding customization
- Single file only — no multi-file project support

### What's next (v0.2)
- texlab LSP integration (completions, hover, goto-def)
- MuPDF PDF rendering in the preview pane
- File tree sidebar, command palette, find & replace
- Auto-compile on save, inverse SyncTeX search
- Built-in LaTeX snippets
