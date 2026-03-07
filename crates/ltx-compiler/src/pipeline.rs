use std::path::{Path, PathBuf};
use std::process::Stdio;

use serde::{Deserialize, Serialize};
use thiserror::Error;
use tokio::process::Command;

use crate::engine::Engine;
use crate::log_parser::{self, CompileMessage};

#[derive(Debug, Error)]
pub enum CompileError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Compiler not found: {0}")]
    CompilerNotFound(String),
    #[error("Compilation failed")]
    Failed,
}

/// Status of a compilation.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum CompileStatus {
    Idle,
    Compiling,
    Success,
    Error,
}

/// Result of a compilation.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CompileResult {
    pub status: CompileStatus,
    pub messages: Vec<CompileMessage>,
    pub pdf_path: Option<String>,
    pub log_output: String,
}

/// LaTeX compiler pipeline.
pub struct Compiler {
    engine: Engine,
    output_dir: Option<PathBuf>,
}

impl Compiler {
    pub fn new(engine: Engine) -> Self {
        Self {
            engine,
            output_dir: None,
        }
    }

    pub fn with_output_dir(mut self, dir: PathBuf) -> Self {
        self.output_dir = Some(dir);
        self
    }

    pub fn engine(&self) -> Engine {
        self.engine
    }

    pub fn set_engine(&mut self, engine: Engine) {
        self.engine = engine;
    }

    /// Compile a LaTeX file.
    pub async fn compile(&self, source_path: &Path) -> Result<CompileResult, CompileError> {
        let source_path = source_path.canonicalize().map_err(CompileError::Io)?;
        let parent_dir = source_path.parent().unwrap_or(Path::new("."));

        let mut cmd = Command::new(self.engine.command());
        for arg in self.engine.default_args() {
            cmd.arg(arg);
        }

        if let Some(ref out_dir) = self.output_dir {
            std::fs::create_dir_all(out_dir)?;
            cmd.arg(format!("-output-directory={}", out_dir.display()));
        }

        cmd.arg(&source_path);
        cmd.current_dir(parent_dir);
        cmd.stdout(Stdio::piped());
        cmd.stderr(Stdio::piped());

        let output = cmd.output().await.map_err(|e| {
            if e.kind() == std::io::ErrorKind::NotFound {
                CompileError::CompilerNotFound(self.engine.command().to_string())
            } else {
                CompileError::Io(e)
            }
        })?;

        let stdout = String::from_utf8_lossy(&output.stdout).to_string();
        let stderr = String::from_utf8_lossy(&output.stderr).to_string();
        let log_output = format!("{}\n{}", stdout, stderr);

        // Also try to read the .log file for more detailed parsing
        let log_path = self.log_path(&source_path);
        let log_content = if log_path.exists() {
            std::fs::read_to_string(&log_path).unwrap_or_default()
        } else {
            log_output.clone()
        };

        let messages = log_parser::parse_log(&log_content);
        let has_errors = messages
            .iter()
            .any(|m| m.kind == log_parser::MessageKind::Error);

        let pdf_path = self.pdf_path(&source_path);
        let pdf_exists = pdf_path.exists();

        let status = if output.status.success() && !has_errors && pdf_exists {
            CompileStatus::Success
        } else {
            CompileStatus::Error
        };

        Ok(CompileResult {
            status,
            messages,
            pdf_path: if pdf_exists {
                Some(pdf_path.to_string_lossy().to_string())
            } else {
                None
            },
            log_output,
        })
    }

    fn pdf_path(&self, source: &Path) -> PathBuf {
        let base = source.with_extension("pdf");
        if let Some(ref out_dir) = self.output_dir {
            out_dir.join(base.file_name().unwrap())
        } else {
            base
        }
    }

    fn log_path(&self, source: &Path) -> PathBuf {
        let base = source.with_extension("log");
        if let Some(ref out_dir) = self.output_dir {
            out_dir.join(base.file_name().unwrap())
        } else {
            base
        }
    }
}

impl Default for Compiler {
    fn default() -> Self {
        Self::new(Engine::default())
    }
}
