use std::sync::Mutex;

use ltx_compiler::{CompileResult, Compiler};
use ltx_core::Document;
use ltx_syntax::Highlighter;
use ltx_theme::Theme;

/// Global application state managed by Tauri.
pub struct AppState {
    pub document: Mutex<Document>,
    pub compiler: Mutex<Compiler>,
    pub highlighter: Mutex<Option<Highlighter>>,
    pub theme: Mutex<Theme>,
    pub last_compile: Mutex<Option<CompileResult>>,
}

impl AppState {
    pub fn new() -> Self {
        let highlighter = Highlighter::new().ok();
        Self {
            document: Mutex::new(Document::new()),
            compiler: Mutex::new(Compiler::default()),
            highlighter: Mutex::new(highlighter),
            theme: Mutex::new(ltx_theme::dark_theme()),
            last_compile: Mutex::new(None),
        }
    }
}
