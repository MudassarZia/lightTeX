use serde::{Deserialize, Serialize};

/// Supported LaTeX engines.
#[derive(Debug, Clone, Copy, Default, PartialEq, Eq, Serialize, Deserialize)]
pub enum Engine {
    #[default]
    PdfLatex,
    XeLatex,
    LuaLatex,
}

impl Engine {
    /// Get the command name for this engine.
    pub fn command(&self) -> &'static str {
        match self {
            Self::PdfLatex => "pdflatex",
            Self::XeLatex => "xelatex",
            Self::LuaLatex => "lualatex",
        }
    }

    /// Get the display name.
    pub fn display_name(&self) -> &'static str {
        match self {
            Self::PdfLatex => "pdfLaTeX",
            Self::XeLatex => "XeLaTeX",
            Self::LuaLatex => "LuaLaTeX",
        }
    }

    /// Get default compilation arguments.
    pub fn default_args(&self) -> Vec<&'static str> {
        vec!["-synctex=1", "-interaction=nonstopmode", "-file-line-error"]
    }
}

impl std::fmt::Display for Engine {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}", self.display_name())
    }
}
