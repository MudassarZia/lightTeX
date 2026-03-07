use serde::{Deserialize, Serialize};
use std::collections::HashMap;

/// A complete theme definition.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Theme {
    pub name: String,
    pub kind: ThemeKind,
    pub colors: ThemeColors,
    #[serde(default)]
    pub syntax: SyntaxColors,
    #[serde(default)]
    pub ui: UiColors,
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
#[serde(rename_all = "lowercase")]
pub enum ThemeKind {
    Dark,
    Light,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct ThemeColors {
    pub background: String,
    pub foreground: String,
    pub cursor: String,
    pub selection: String,
    pub line_highlight: String,
    pub gutter: String,
    pub gutter_foreground: String,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct SyntaxColors {
    #[serde(default = "default_command")]
    pub command: String,
    #[serde(default = "default_environment")]
    pub environment: String,
    #[serde(default = "default_math")]
    pub math: String,
    #[serde(default = "default_comment")]
    pub comment: String,
    #[serde(default = "default_string")]
    pub string: String,
    #[serde(default = "default_number")]
    pub number: String,
    #[serde(default = "default_bracket")]
    pub bracket: String,
    #[serde(default = "default_error")]
    pub error: String,
}

#[derive(Debug, Clone, Serialize, Deserialize, Default)]
pub struct UiColors {
    #[serde(default = "default_statusbar_bg")]
    pub statusbar_bg: String,
    #[serde(default = "default_statusbar_fg")]
    pub statusbar_fg: String,
    #[serde(default = "default_sidebar_bg")]
    pub sidebar_bg: String,
    #[serde(default = "default_sidebar_fg")]
    pub sidebar_fg: String,
    #[serde(default = "default_panel_bg")]
    pub panel_bg: String,
    #[serde(default = "default_panel_border")]
    pub panel_border: String,
}

// Default color values
fn default_command() -> String {
    "#569cd6".to_string()
}
fn default_environment() -> String {
    "#c586c0".to_string()
}
fn default_math() -> String {
    "#dcdcaa".to_string()
}
fn default_comment() -> String {
    "#6a9955".to_string()
}
fn default_string() -> String {
    "#ce9178".to_string()
}
fn default_number() -> String {
    "#b5cea8".to_string()
}
fn default_bracket() -> String {
    "#ffd700".to_string()
}
fn default_error() -> String {
    "#f44747".to_string()
}
fn default_statusbar_bg() -> String {
    "#007acc".to_string()
}
fn default_statusbar_fg() -> String {
    "#ffffff".to_string()
}
fn default_sidebar_bg() -> String {
    "#252526".to_string()
}
fn default_sidebar_fg() -> String {
    "#cccccc".to_string()
}
fn default_panel_bg() -> String {
    "#1e1e1e".to_string()
}
fn default_panel_border() -> String {
    "#3c3c3c".to_string()
}

impl Theme {
    /// Convert the theme to CSS custom properties for the frontend.
    pub fn to_css_variables(&self) -> HashMap<String, String> {
        let mut vars = HashMap::new();

        // Editor colors
        vars.insert("--editor-bg".into(), self.colors.background.clone());
        vars.insert("--editor-fg".into(), self.colors.foreground.clone());
        vars.insert("--editor-cursor".into(), self.colors.cursor.clone());
        vars.insert("--editor-selection".into(), self.colors.selection.clone());
        vars.insert(
            "--editor-line-highlight".into(),
            self.colors.line_highlight.clone(),
        );
        vars.insert("--editor-gutter".into(), self.colors.gutter.clone());
        vars.insert(
            "--editor-gutter-fg".into(),
            self.colors.gutter_foreground.clone(),
        );

        // Syntax colors
        vars.insert("--syntax-command".into(), self.syntax.command.clone());
        vars.insert(
            "--syntax-environment".into(),
            self.syntax.environment.clone(),
        );
        vars.insert("--syntax-math".into(), self.syntax.math.clone());
        vars.insert("--syntax-comment".into(), self.syntax.comment.clone());
        vars.insert("--syntax-string".into(), self.syntax.string.clone());
        vars.insert("--syntax-number".into(), self.syntax.number.clone());
        vars.insert("--syntax-bracket".into(), self.syntax.bracket.clone());
        vars.insert("--syntax-error".into(), self.syntax.error.clone());

        // UI colors
        vars.insert("--ui-statusbar-bg".into(), self.ui.statusbar_bg.clone());
        vars.insert("--ui-statusbar-fg".into(), self.ui.statusbar_fg.clone());
        vars.insert("--ui-sidebar-bg".into(), self.ui.sidebar_bg.clone());
        vars.insert("--ui-sidebar-fg".into(), self.ui.sidebar_fg.clone());
        vars.insert("--ui-panel-bg".into(), self.ui.panel_bg.clone());
        vars.insert("--ui-panel-border".into(), self.ui.panel_border.clone());

        vars
    }
}
