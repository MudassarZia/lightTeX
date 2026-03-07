pub mod theme;

pub use theme::{SyntaxColors, Theme, ThemeColors, UiColors};

/// Load the built-in dark theme.
pub fn dark_theme() -> Theme {
    let toml_str = include_str!("../../../themes/dark.toml");
    toml::from_str(toml_str).expect("built-in dark theme should be valid")
}

/// Load the built-in light theme.
pub fn light_theme() -> Theme {
    let toml_str = include_str!("../../../themes/light.toml");
    toml::from_str(toml_str).expect("built-in light theme should be valid")
}

/// Load a theme from a TOML string.
pub fn load_theme(toml_str: &str) -> Result<Theme, toml::de::Error> {
    toml::from_str(toml_str)
}

/// Load a theme from a file path.
pub fn load_theme_file(path: &std::path::Path) -> Result<Theme, ThemeLoadError> {
    let content = std::fs::read_to_string(path)?;
    let theme = toml::from_str(&content)?;
    Ok(theme)
}

#[derive(Debug, thiserror::Error)]
pub enum ThemeLoadError {
    #[error("IO error: {0}")]
    Io(#[from] std::io::Error),
    #[error("Parse error: {0}")]
    Parse(#[from] toml::de::Error),
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_load_dark_theme() {
        let theme = dark_theme();
        assert_eq!(theme.name, "Dark");
        assert!(!theme.colors.background.is_empty());
    }

    #[test]
    fn test_load_light_theme() {
        let theme = light_theme();
        assert_eq!(theme.name, "Light");
        assert!(!theme.colors.background.is_empty());
    }

    #[test]
    fn test_load_invalid_toml() {
        let result = load_theme("not valid toml {{{}}}");
        assert!(result.is_err());
    }

    #[test]
    fn test_theme_to_css_vars() {
        let theme = dark_theme();
        let vars = theme.to_css_variables();
        assert!(vars.contains_key("--editor-bg"));
        assert!(vars.contains_key("--editor-fg"));
    }
}
