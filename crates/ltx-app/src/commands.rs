use std::collections::HashMap;
use std::path::PathBuf;

use serde::{Deserialize, Serialize};
use tauri::State;

use ltx_compiler::{CompileResult, CompileStatus};
use ltx_core::buffer::BufferDelta;
use ltx_syntax::HighlightEvent;

use crate::state::AppState;

#[derive(Debug, Serialize, Deserialize)]
pub struct FileInfo {
    pub name: String,
    pub path: String,
    pub content: String,
}

#[tauri::command]
pub fn open_file(path: String, state: State<'_, AppState>) -> Result<FileInfo, String> {
    let file_path = PathBuf::from(&path);
    let doc = ltx_core::Document::open(&file_path).map_err(|e| e.to_string())?;
    let name = doc.display_name();
    let content = doc.buffer.text();
    *state.document.lock().unwrap() = doc;
    Ok(FileInfo {
        name,
        path,
        content,
    })
}

#[tauri::command]
pub fn save_file(state: State<'_, AppState>) -> Result<(), String> {
    state
        .document
        .lock()
        .unwrap()
        .save()
        .map_err(|e| e.to_string())
}

#[tauri::command]
pub fn get_buffer_content(state: State<'_, AppState>) -> String {
    state.document.lock().unwrap().buffer.text()
}

#[derive(Debug, Deserialize)]
pub struct EditRequest {
    pub start: usize,
    pub end: usize,
    pub text: String,
}

#[tauri::command]
pub fn apply_edit(edit: EditRequest, state: State<'_, AppState>) -> Result<BufferDelta, String> {
    let mut doc = state.document.lock().unwrap();
    let delta = doc.buffer.replace(edit.start, edit.end, &edit.text);
    Ok(delta)
}

#[tauri::command]
pub fn undo(state: State<'_, AppState>) -> Result<Option<BufferDelta>, String> {
    let mut doc = state.document.lock().unwrap();
    Ok(doc.buffer.undo())
}

#[tauri::command]
pub fn redo(state: State<'_, AppState>) -> Result<Option<BufferDelta>, String> {
    let mut doc = state.document.lock().unwrap();
    Ok(doc.buffer.redo())
}

#[tauri::command]
pub async fn compile(state: State<'_, AppState>) -> Result<CompileResult, String> {
    let path = {
        let doc = state.document.lock().unwrap();
        doc.path.clone().ok_or("No file open")?
    };

    let engine = {
        let compiler = state.compiler.lock().unwrap();
        compiler.engine()
    };

    // Create a temporary compiler outside the lock to avoid holding MutexGuard across await
    let compiler = ltx_compiler::Compiler::new(engine);
    let result = compiler.compile(&path).await.map_err(|e| e.to_string())?;

    *state.last_compile.lock().unwrap() = Some(result.clone());
    Ok(result)
}

#[tauri::command]
pub fn get_theme(state: State<'_, AppState>) -> HashMap<String, String> {
    state.theme.lock().unwrap().to_css_variables()
}

#[tauri::command]
pub fn set_theme(
    theme_name: String,
    state: State<'_, AppState>,
) -> Result<HashMap<String, String>, String> {
    let theme = match theme_name.as_str() {
        "dark" | "Dark" => ltx_theme::dark_theme(),
        "light" | "Light" => ltx_theme::light_theme(),
        _ => return Err(format!("Unknown theme: {}", theme_name)),
    };
    let vars = theme.to_css_variables();
    *state.theme.lock().unwrap() = theme;
    Ok(vars)
}

#[tauri::command]
pub fn get_highlights(content: String, state: State<'_, AppState>) -> Vec<HighlightEvent> {
    let mut hl = state.highlighter.lock().unwrap();
    if let Some(ref mut highlighter) = *hl {
        highlighter.parse(&content).unwrap_or_default()
    } else {
        Vec::new()
    }
}

#[tauri::command]
pub fn get_compile_status(state: State<'_, AppState>) -> CompileStatus {
    state
        .last_compile
        .lock()
        .unwrap()
        .as_ref()
        .map(|r| r.status)
        .unwrap_or(CompileStatus::Idle)
}
