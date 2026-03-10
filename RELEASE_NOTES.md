## lightTex v0.2.1-beta — Snippet Expansion & LSP Integration

Snippets are now fully wired into the editor with Tab-to-expand, tabstop cycling, and LSP argument completion.

### What's new since v0.2.0-beta

- **Snippet tab-expansion**: Type `\frac`, `\begin`, `\sec`, etc. and press Tab to expand with placeholders
- **Tabstop cycling**: Tab / Shift+Tab through placeholders ($1 → $2 → $0), session cancels on typing/arrows/Escape
- **Snippets override LSP on Tab**: When a snippet trigger matches, it takes priority over the LSP completion popup
- **LSP argument completion after snippets**: Expanding `\begin` triggers environment name suggestions automatically
- **LSP completion on cursor-into-braces**: Moving cursor into empty `{}` after a command (e.g. `\begin{}`) triggers argument completion
- **Auto-indent tests**: 14 new tests for auto-indentation and `\begin`/`\end` auto-pairing

### Test suite

~189 tests across 22 test suites, all passing.

### Known limitations

- Linked tabstop mirroring not implemented (editing `\begin{env}` does not sync to `\end{env}`)
- No vim mode (planned for v0.3)
- No multi-file project support (planned for v0.3)
- texlab must be installed separately and on PATH
