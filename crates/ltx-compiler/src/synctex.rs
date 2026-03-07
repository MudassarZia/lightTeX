use serde::{Deserialize, Serialize};
use std::path::Path;

/// A position in the source file.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct SourcePosition {
    pub file: String,
    pub line: usize,
    pub column: usize,
}

/// A position in the PDF.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct PdfPosition {
    pub page: usize,
    pub x: f64,
    pub y: f64,
}

/// SyncTeX integration for forward/inverse search.
pub struct SyncTeX {
    synctex_path: Option<String>,
}

impl SyncTeX {
    pub fn new() -> Self {
        Self { synctex_path: None }
    }

    /// Load a SyncTeX file associated with a PDF.
    pub fn load(&mut self, pdf_path: &Path) -> Result<(), std::io::Error> {
        let synctex_gz = pdf_path.with_extension("synctex.gz");
        let synctex = pdf_path.with_extension("synctex");

        if synctex_gz.exists() {
            self.synctex_path = Some(synctex_gz.to_string_lossy().to_string());
            Ok(())
        } else if synctex.exists() {
            self.synctex_path = Some(synctex.to_string_lossy().to_string());
            Ok(())
        } else {
            Err(std::io::Error::new(
                std::io::ErrorKind::NotFound,
                "SyncTeX file not found",
            ))
        }
    }

    /// Forward search: source position -> PDF position.
    /// Uses the synctex command-line tool.
    pub async fn forward(
        &self,
        source: &SourcePosition,
        pdf_path: &Path,
    ) -> Result<Option<PdfPosition>, std::io::Error> {
        let output = tokio::process::Command::new("synctex")
            .arg("view")
            .arg("-i")
            .arg(format!("{}:{}:{}", source.line, source.column, source.file))
            .arg("-o")
            .arg(pdf_path.to_string_lossy().as_ref())
            .output()
            .await?;

        let stdout = String::from_utf8_lossy(&output.stdout);
        Ok(Self::parse_forward_output(&stdout))
    }

    /// Inverse search: PDF position -> source position.
    pub async fn inverse(
        &self,
        pdf_pos: &PdfPosition,
        pdf_path: &Path,
    ) -> Result<Option<SourcePosition>, std::io::Error> {
        let output = tokio::process::Command::new("synctex")
            .arg("edit")
            .arg("-o")
            .arg(format!("{}:{}:{}", pdf_pos.page, pdf_pos.x, pdf_pos.y))
            .arg("-i")
            .arg(pdf_path.to_string_lossy().as_ref())
            .output()
            .await?;

        let stdout = String::from_utf8_lossy(&output.stdout);
        Ok(Self::parse_inverse_output(&stdout))
    }

    /// Whether a SyncTeX file is loaded.
    pub fn is_loaded(&self) -> bool {
        self.synctex_path.is_some()
    }

    fn parse_forward_output(output: &str) -> Option<PdfPosition> {
        let mut page = None;
        let mut x = None;
        let mut y = None;

        for line in output.lines() {
            if let Some(val) = line.strip_prefix("Page:") {
                page = val.trim().parse().ok();
            } else if let Some(val) = line.strip_prefix("x:") {
                x = val.trim().parse().ok();
            } else if let Some(val) = line.strip_prefix("y:") {
                y = val.trim().parse().ok();
            }
        }

        match (page, x, y) {
            (Some(page), Some(x), Some(y)) => Some(PdfPosition { page, x, y }),
            _ => None,
        }
    }

    fn parse_inverse_output(output: &str) -> Option<SourcePosition> {
        let mut file = None;
        let mut line = None;
        let mut column = None;

        for l in output.lines() {
            if let Some(val) = l.strip_prefix("Input:") {
                file = Some(val.trim().to_string());
            } else if let Some(val) = l.strip_prefix("Line:") {
                line = val.trim().parse().ok();
            } else if let Some(val) = l.strip_prefix("Column:") {
                column = val.trim().parse().ok();
            }
        }

        match (file, line) {
            (Some(file), Some(line)) => Some(SourcePosition {
                file,
                line,
                column: column.unwrap_or(0),
            }),
            _ => None,
        }
    }
}

impl Default for SyncTeX {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_forward_output() {
        let output = "SyncTeX result:\nPage:1\nx:72.0\ny:100.5\nh:72.0\nv:100.5\n";
        let pos = SyncTeX::parse_forward_output(output).unwrap();
        assert_eq!(pos.page, 1);
        assert!((pos.x - 72.0).abs() < f64::EPSILON);
        assert!((pos.y - 100.5).abs() < f64::EPSILON);
    }

    #[test]
    fn test_parse_forward_output_missing() {
        let output = "SyncTeX result:\nPage:1\n";
        let pos = SyncTeX::parse_forward_output(output);
        assert!(pos.is_none());
    }

    #[test]
    fn test_parse_inverse_output() {
        let output = "SyncTeX result:\nInput:./main.tex\nLine:42\nColumn:0\n";
        let pos = SyncTeX::parse_inverse_output(output).unwrap();
        assert_eq!(pos.file, "./main.tex");
        assert_eq!(pos.line, 42);
        assert_eq!(pos.column, 0);
    }

    #[test]
    fn test_parse_inverse_output_no_column() {
        let output = "Input:./main.tex\nLine:10\n";
        let pos = SyncTeX::parse_inverse_output(output).unwrap();
        assert_eq!(pos.column, 0);
    }

    #[test]
    fn test_synctex_not_loaded() {
        let sync = SyncTeX::new();
        assert!(!sync.is_loaded());
    }
}
