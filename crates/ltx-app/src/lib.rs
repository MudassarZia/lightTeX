mod commands;
mod state;

use state::AppState;

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tracing_subscriber::fmt()
        .with_env_filter(
            tracing_subscriber::EnvFilter::from_default_env()
                .add_directive("ltx=debug".parse().unwrap()),
        )
        .init();

    tracing::info!("Starting lightTex...");

    tauri::Builder::default()
        .manage(AppState::new())
        .invoke_handler(tauri::generate_handler![
            commands::open_file,
            commands::save_file,
            commands::get_buffer_content,
            commands::apply_edit,
            commands::undo,
            commands::redo,
            commands::compile,
            commands::get_theme,
            commands::set_theme,
            commands::get_highlights,
            commands::get_compile_status,
        ])
        .run(tauri::generate_context!())
        .expect("error while running lightTex");
}
