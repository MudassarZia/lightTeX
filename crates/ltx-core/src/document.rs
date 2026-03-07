use std::path::{Path, PathBuf};

use crate::buffer::Buffer;
use thiserror::Error;

#[derive(Debug, Error)]
pub enum DocumentError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    #[error("File not found: {0}")]
    NotFound(PathBuf),
}

/// Represents an open document with its file path and buffer.
#[derive(Debug, Clone)]
pub struct Document {
    pub buffer: Buffer,
    pub path: Option<PathBuf>,
    pub line_ending: LineEnding,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum LineEnding {
    Lf,
    CrLf,
}

impl LineEnding {
    /// Detect line ending from text content.
    pub fn detect(text: &str) -> Self {
        if text.contains("\r\n") {
            Self::CrLf
        } else {
            Self::Lf
        }
    }

    pub fn as_str(&self) -> &'static str {
        match self {
            Self::Lf => "\n",
            Self::CrLf => "\r\n",
        }
    }
}

impl Document {
    /// Create a new empty document.
    pub fn new() -> Self {
        Self {
            buffer: Buffer::new(),
            path: None,
            line_ending: if cfg!(windows) {
                LineEnding::CrLf
            } else {
                LineEnding::Lf
            },
        }
    }

    /// Open a document from a file path.
    pub fn open(path: &Path) -> Result<Self, DocumentError> {
        if !path.exists() {
            return Err(DocumentError::NotFound(path.to_path_buf()));
        }
        let content = std::fs::read_to_string(path)?;
        let line_ending = LineEnding::detect(&content);
        // Normalize to LF internally
        let normalized = content.replace("\r\n", "\n");
        Ok(Self {
            buffer: Buffer::from_text(&normalized),
            path: Some(path.to_path_buf()),
            line_ending,
        })
    }

    /// Save the document to its associated path.
    pub fn save(&mut self) -> Result<(), DocumentError> {
        let path = self
            .path
            .as_ref()
            .ok_or_else(|| {
                DocumentError::Io(std::io::Error::new(
                    std::io::ErrorKind::NotFound,
                    "No file path set",
                ))
            })?
            .clone();
        self.save_as(&path)
    }

    /// Save the document to a specific path.
    pub fn save_as(&mut self, path: &Path) -> Result<(), DocumentError> {
        let mut content = self.buffer.text();
        if self.line_ending == LineEnding::CrLf {
            content = content.replace('\n', "\r\n");
        }
        std::fs::write(path, &content)?;
        self.path = Some(path.to_path_buf());
        self.buffer.mark_saved();
        Ok(())
    }

    /// Get the file name for display.
    pub fn display_name(&self) -> String {
        self.path
            .as_ref()
            .and_then(|p| p.file_name())
            .map(|n| n.to_string_lossy().to_string())
            .unwrap_or_else(|| "Untitled".to_string())
    }
}

impl Default for Document {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::Write as IoWrite;
    use tempfile::NamedTempFile;

    #[test]
    fn test_new_document() {
        let doc = Document::new();
        assert!(doc.path.is_none());
        assert!(doc.buffer.is_empty());
        assert_eq!(doc.display_name(), "Untitled");
    }

    #[test]
    fn test_open_nonexistent() {
        let result = Document::open(Path::new("/nonexistent/file.tex"));
        assert!(result.is_err());
    }

    #[test]
    fn test_open_and_save_roundtrip() {
        let mut tmp = NamedTempFile::new().unwrap();
        writeln!(tmp, "\\documentclass{{article}}").unwrap();
        writeln!(tmp, "\\begin{{document}}").unwrap();
        writeln!(tmp, "Hello, world!").unwrap();
        writeln!(tmp, "\\end{{document}}").unwrap();

        let mut doc = Document::open(tmp.path()).unwrap();
        assert!(doc.buffer.text().contains("Hello, world!"));
        assert!(!doc.buffer.is_modified());

        doc.buffer.insert(0, "% comment\n");
        assert!(doc.buffer.is_modified());

        doc.save().unwrap();
        assert!(!doc.buffer.is_modified());

        // Re-read and verify
        let doc2 = Document::open(tmp.path()).unwrap();
        assert!(doc2.buffer.text().starts_with("% comment"));
    }

    #[test]
    fn test_line_ending_detection() {
        assert_eq!(LineEnding::detect("hello\nworld"), LineEnding::Lf);
        assert_eq!(LineEnding::detect("hello\r\nworld"), LineEnding::CrLf);
    }

    #[test]
    fn test_display_name() {
        let mut doc = Document::new();
        assert_eq!(doc.display_name(), "Untitled");
        doc.path = Some(PathBuf::from("/tmp/test.tex"));
        assert_eq!(doc.display_name(), "test.tex");
    }
}
