## lightTex v0.2.0-beta — "Actually Usable"

Full-featured LaTeX editing with LSP autocomplete, snippets, find/replace, file tree, and 174 tests. Rust/Tauri code has been fully removed — lightTex is now a pure C++20/Qt 6 application.

### What's new since v0.1.0

- **LSP autocomplete**: Type `\` for command completion (inserts `\command{}`), type `{` for argument completion (package names, environment names, labels). Powered by texlab via JSON-RPC.
- **LSP hover & go-to-definition**: Alt+H for hover info, F12 for go-to-definition
- **LSP diagnostics**: Errors and warnings from texlab shown in the compile panel
- **Find/Replace**: Ctrl+F / Ctrl+H with regex support, match highlighting, replace all
- **File tree sidebar**: Ctrl+B toggle, filtered to LaTeX-relevant files, auto-detects project root
- **Configurable keybindings**: TOML-based (`keybindings.toml`), 8 default shortcuts
- **LaTeX snippets**: 18 built-in snippets with tabstop cycling (`\begin`, `\frac`, `\fig`, etc.)
- **Auto-compile**: Toggle via command palette, compiles on save
- **Error panel**: Click-to-jump to error line in editor
- **Command palette**: Fuzzy subsequence matching with scoring
- **Codebase cleanup**: Removed all legacy Rust/Tauri/frontend code

### Test suite

174 tests across 20 test suites, all passing:
- Core: piece table, selection, history, document (35 tests)
- Syntax: tree-sitter highlighting (17 tests)
- Compiler: log parser, SyncTeX (13 tests)
- PDF: renderer, page cache (11 tests)
- Theme: TOML parsing (8 tests)
- Editor: widget, find/replace, bracket matcher (17 tests)
- UI: command palette, compile panel, file tree, status bar (14 tests)
- Shortcuts & snippets (23 tests)
- LSP: JSON-RPC, types, completion widget (34 tests)

### Known limitations

- No vim mode (planned for v0.3)
- No multi-file project support (planned for v0.3)
- No plugin system (planned for v0.4)
- PDF preview requires Poppler-Qt6 or Qt6::Pdf; falls back to stub otherwise
- texlab must be installed separately and on PATH
