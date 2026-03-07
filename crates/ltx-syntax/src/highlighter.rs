use serde::{Deserialize, Serialize};
use thiserror::Error;
use tree_sitter::{Language, Parser, Tree};

#[derive(Debug, Error)]
pub enum SyntaxError {
    #[error("Failed to set parser language: {0}")]
    LanguageError(String),
    #[error("Failed to parse document")]
    ParseError,
}

/// Token types for syntax highlighting.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum TokenKind {
    Command,
    Environment,
    MathDelimiter,
    MathContent,
    Comment,
    Text,
    Bracket,
    Operator,
    Number,
    String,
    Key,
    Error,
}

/// A syntax highlight event with byte range and token type.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct HighlightEvent {
    pub start_byte: usize,
    pub end_byte: usize,
    pub start_row: usize,
    pub start_col: usize,
    pub end_row: usize,
    pub end_col: usize,
    pub kind: TokenKind,
}

/// Incremental syntax highlighter using tree-sitter.
pub struct Highlighter {
    parser: Parser,
    tree: Option<Tree>,
    language: Language,
}

impl Highlighter {
    /// Create a new LaTeX highlighter.
    pub fn new() -> Result<Self, SyntaxError> {
        let language = tree_sitter_latex::LANGUAGE;
        let mut parser = Parser::new();
        parser
            .set_language(&language.into())
            .map_err(|e| SyntaxError::LanguageError(e.to_string()))?;

        Ok(Self {
            parser,
            tree: None,
            language: language.into(),
        })
    }

    /// Parse or re-parse the full document.
    pub fn parse(&mut self, source: &str) -> Result<Vec<HighlightEvent>, SyntaxError> {
        let tree = self
            .parser
            .parse(source, self.tree.as_ref())
            .ok_or(SyntaxError::ParseError)?;

        let events = Self::walk_tree(&tree, source);
        self.tree = Some(tree);
        Ok(events)
    }

    /// Notify of an edit for incremental reparsing.
    pub fn edit(&mut self, edit: &tree_sitter::InputEdit) {
        if let Some(tree) = &mut self.tree {
            tree.edit(edit);
        }
    }

    /// Re-parse after edits have been applied.
    pub fn reparse(&mut self, source: &str) -> Result<Vec<HighlightEvent>, SyntaxError> {
        self.parse(source)
    }

    /// Get the tree-sitter language.
    pub fn language(&self) -> &Language {
        &self.language
    }

    /// Walk the syntax tree and produce highlight events.
    fn walk_tree(tree: &Tree, source: &str) -> Vec<HighlightEvent> {
        let mut events = Vec::new();
        let mut cursor = tree.walk();
        let mut did_visit_children = false;

        loop {
            let node = cursor.node();

            if !did_visit_children {
                let kind = Self::classify_node(node.kind(), &node, source);

                if let Some(kind) = kind {
                    let start = node.start_position();
                    let end = node.end_position();
                    events.push(HighlightEvent {
                        start_byte: node.start_byte(),
                        end_byte: node.end_byte(),
                        start_row: start.row,
                        start_col: start.column,
                        end_row: end.row,
                        end_col: end.column,
                        kind,
                    });
                }

                if cursor.goto_first_child() {
                    continue;
                }
                did_visit_children = true;
            }

            if cursor.goto_next_sibling() {
                did_visit_children = false;
            } else if !cursor.goto_parent() {
                break;
            }
        }

        events
    }

    /// Classify a tree-sitter node into our token types.
    fn classify_node(
        node_kind: &str,
        node: &tree_sitter::Node,
        _source: &str,
    ) -> Option<TokenKind> {
        match node_kind {
            // Commands
            "command_name" | "generic_command" => Some(TokenKind::Command),

            // Environments
            "begin" | "end" | "environment_name" => Some(TokenKind::Environment),

            // Math
            "inline_formula" | "displayed_equation" | "math_environment" => {
                Some(TokenKind::MathDelimiter)
            }
            "math_content" => Some(TokenKind::MathContent),

            // Comments
            "line_comment" | "block_comment" | "comment" => Some(TokenKind::Comment),

            // Brackets and delimiters
            "{" | "}" | "[" | "]" | "(" | ")" => Some(TokenKind::Bracket),

            // Operators
            "&" | "=" | "_" | "^" => Some(TokenKind::Operator),

            // Numbers
            "number" => Some(TokenKind::Number),

            // Errors
            "ERROR" => Some(TokenKind::Error),

            // Text nodes in leaf positions
            "text" | "word" => {
                if node.child_count() == 0 {
                    Some(TokenKind::Text)
                } else {
                    None
                }
            }

            _ => None,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_highlighter_creation() {
        let hl = Highlighter::new();
        assert!(hl.is_ok());
    }

    #[test]
    fn test_parse_simple_document() {
        let mut hl = Highlighter::new().unwrap();
        let source = r#"\documentclass{article}
\begin{document}
Hello, world!
\end{document}"#;
        let events = hl.parse(source).unwrap();
        assert!(!events.is_empty());
    }

    #[test]
    fn test_parse_math() {
        let mut hl = Highlighter::new().unwrap();
        let source = r#"The equation $E = mc^2$ is famous."#;
        let events = hl.parse(source).unwrap();
        assert!(!events.is_empty());
    }

    #[test]
    fn test_parse_comment() {
        let mut hl = Highlighter::new().unwrap();
        let source = "% This is a comment\n\\section{Title}";
        let events = hl.parse(source).unwrap();

        let comment_events: Vec<_> = events
            .iter()
            .filter(|e| e.kind == TokenKind::Comment)
            .collect();
        assert!(!comment_events.is_empty(), "Should detect comment tokens");
    }

    #[test]
    fn test_incremental_reparse() {
        let mut hl = Highlighter::new().unwrap();
        let source1 = r#"\begin{document}
Hello
\end{document}"#;
        hl.parse(source1).unwrap();

        let source2 = r#"\begin{document}
Hello, world!
\end{document}"#;
        let events = hl.reparse(source2).unwrap();
        assert!(!events.is_empty());
    }

    #[test]
    fn test_empty_document() {
        let mut hl = Highlighter::new().unwrap();
        let events = hl.parse("").unwrap();
        // Empty document may produce no events
        let _ = events;
    }

    #[test]
    fn test_environment_detection() {
        let mut hl = Highlighter::new().unwrap();
        let source = r#"\begin{enumerate}
\item First
\item Second
\end{enumerate}"#;
        let events = hl.parse(source).unwrap();
        let env_events: Vec<_> = events
            .iter()
            .filter(|e| e.kind == TokenKind::Environment)
            .collect();
        assert!(!env_events.is_empty(), "Should detect environment tokens");
    }
}
