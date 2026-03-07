use serde::{Deserialize, Serialize};
use std::path::{Path, PathBuf};
use thiserror::Error;

#[derive(Debug, Error)]
pub enum PdfError {
    #[error("Failed to open PDF: {0}")]
    OpenError(String),
    #[error("Failed to render page {0}")]
    RenderError(usize),
    #[error("Page {0} out of range (total: {1})")]
    PageOutOfRange(usize, usize),
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
}

/// A rendered PDF page as an RGBA bitmap.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct RenderedPage {
    pub page_num: usize,
    pub width: u32,
    pub height: u32,
    /// Base64-encoded RGBA pixel data.
    pub data_base64: String,
}

/// PDF renderer abstraction.
/// For v0.1, uses a stub implementation.
/// MuPDF integration will be added when mupdf-rs builds are resolved.
pub struct PdfRenderer {
    path: Option<PathBuf>,
    page_count: usize,
}

impl PdfRenderer {
    pub fn new() -> Self {
        Self {
            path: None,
            page_count: 0,
        }
    }

    /// Open a PDF file.
    pub fn open(&mut self, path: &Path) -> Result<(), PdfError> {
        if !path.exists() {
            return Err(PdfError::OpenError(format!(
                "File not found: {}",
                path.display()
            )));
        }

        // Read file to verify it's a valid PDF (check magic bytes)
        let file_bytes = std::fs::read(path)?;
        if !file_bytes.starts_with(b"%PDF") {
            return Err(PdfError::OpenError("Not a valid PDF file".to_string()));
        }

        self.path = Some(path.to_path_buf());
        // Stub: estimate page count from file size (will be replaced by MuPDF)
        self.page_count = 1.max(file_bytes.len() / 50000);
        tracing::info!(
            "Opened PDF: {} ({} estimated pages)",
            path.display(),
            self.page_count
        );
        Ok(())
    }

    /// Get the number of pages.
    pub fn page_count(&self) -> usize {
        self.page_count
    }

    /// Get the file path.
    pub fn path(&self) -> Option<&Path> {
        self.path.as_deref()
    }

    /// Whether a PDF is loaded.
    pub fn is_loaded(&self) -> bool {
        self.path.is_some()
    }

    /// Render a page to an RGBA bitmap.
    /// Stub implementation - returns a placeholder.
    pub fn render_page(&self, page_num: usize, _dpi: f32) -> Result<RenderedPage, PdfError> {
        if page_num >= self.page_count {
            return Err(PdfError::PageOutOfRange(page_num, self.page_count));
        }

        // Placeholder: create a small gray rectangle
        // Real implementation will use MuPDF
        let width: u32 = 612;
        let height: u32 = 792;
        let pixel_count = (width * height) as usize;
        let mut rgba = Vec::with_capacity(pixel_count * 4);
        for _ in 0..pixel_count {
            rgba.extend_from_slice(&[240, 240, 240, 255]); // light gray
        }

        let encoded = flate2_stub_base64_encode(&rgba);

        Ok(RenderedPage {
            page_num,
            width,
            height,
            data_base64: encoded,
        })
    }

    /// Reload the currently opened PDF (e.g., after recompilation).
    pub fn reload(&mut self) -> Result<(), PdfError> {
        if let Some(path) = self.path.clone() {
            self.open(&path)
        } else {
            Err(PdfError::OpenError("No PDF loaded".to_string()))
        }
    }
}

impl Default for PdfRenderer {
    fn default() -> Self {
        Self::new()
    }
}

/// Simple base64 encoding for the stub renderer.
fn flate2_stub_base64_encode(_data: &[u8]) -> String {
    // Stub: real implementation will use MuPDF bitmaps
    String::from("STUB_PLACEHOLDER")
}

#[allow(dead_code)]
fn base64_encode_impl(data: &[u8]) -> String {
    const CHARS: &[u8] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    let mut result = String::with_capacity(data.len().div_ceil(3) * 4);

    for chunk in data.chunks(3) {
        let b0 = chunk[0] as u32;
        let b1 = chunk.get(1).copied().unwrap_or(0) as u32;
        let b2 = chunk.get(2).copied().unwrap_or(0) as u32;
        let triple = (b0 << 16) | (b1 << 8) | b2;

        result.push(CHARS[((triple >> 18) & 0x3F) as usize] as char);
        result.push(CHARS[((triple >> 12) & 0x3F) as usize] as char);
        if chunk.len() > 1 {
            result.push(CHARS[((triple >> 6) & 0x3F) as usize] as char);
        } else {
            result.push('=');
        }
        if chunk.len() > 2 {
            result.push(CHARS[(triple & 0x3F) as usize] as char);
        } else {
            result.push('=');
        }
    }

    result
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new_renderer() {
        let renderer = PdfRenderer::new();
        assert!(!renderer.is_loaded());
        assert_eq!(renderer.page_count(), 0);
    }

    #[test]
    fn test_open_nonexistent() {
        let mut renderer = PdfRenderer::new();
        assert!(renderer.open(Path::new("/nonexistent.pdf")).is_err());
    }

    #[test]
    fn test_render_no_pdf() {
        let renderer = PdfRenderer::new();
        assert!(renderer.render_page(0, 72.0).is_err());
    }
}
