## lightTex v0.1.0-beta — "Hello, LaTeX"

First release. A fast, keyboard-first LaTeX editor built with C++20 and Qt 6.

### What works

- **Editor**: QPlainTextEdit subclass with line numbers, bracket matching, current line highlight
- **Syntax highlighting**: tree-sitter incremental parsing for LaTeX — commands, environments, math, comments, brackets all get distinct colors
- **Compilation**: pdfLaTeX / XeLaTeX / LuaLaTeX pipeline via QProcess with structured error/warning/badbox log parsing
- **PDF preview**: Split-pane rendering via Poppler-Qt6 or Qt6::Pdf (stub fallback if neither installed)
- **Themes**: Dark + Light TOML themes applied via Qt stylesheets to all widgets
- **Command palette**: Ctrl+Shift+P overlay with fuzzy filtering and keyboard navigation
- **Status bar**: Line/col, compile status (color-coded), engine, encoding
- **File I/O**: Native file dialogs, LF/CRLF detection + normalization
- **Undo/redo**: Full history with transaction support
- **Tests**: 95 tests across 12 test files (Google Test + QTest)

### Known limitations

- No auto-compile on save
- No LSP completions/diagnostics
- No vim mode, snippets, or keybinding customization
- Single file only — no multi-file project support
- PDF preview requires Poppler-Qt6 or Qt6::Pdf module; falls back to gray stub otherwise

### What's next (v0.2)

- texlab LSP integration (completions, hover, goto-def)
- Auto-compile on save (QFileSystemWatcher)
- File tree sidebar
- Find/replace with regex
- Error panel with click-to-jump
