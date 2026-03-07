use ropey::Rope;
use serde::{Deserialize, Serialize};

use crate::history::{EditOperation, History};
use crate::selection::Selection;

/// Core text buffer backed by a rope data structure.
/// Provides O(log n) editing operations on arbitrarily large files.
#[derive(Debug, Clone)]
pub struct Buffer {
    rope: Rope,
    selections: Vec<Selection>,
    history: History,
    modified: bool,
}

/// A single edit transaction that can be applied atomically.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Transaction {
    pub edits: Vec<Edit>,
}

/// A single text edit operation.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Edit {
    /// Byte offset of the start of the range to replace.
    pub start: usize,
    /// Byte offset of the end of the range to replace.
    pub end: usize,
    /// The replacement text.
    pub text: String,
}

/// Represents a change delta emitted after an edit.
#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct BufferDelta {
    pub edits: Vec<EditDelta>,
    pub content: String,
}

#[derive(Debug, Clone, PartialEq, Serialize, Deserialize)]
pub struct EditDelta {
    pub start_line: usize,
    pub start_col: usize,
    pub end_line: usize,
    pub end_col: usize,
    pub text: String,
}

impl Buffer {
    /// Create a new empty buffer.
    pub fn new() -> Self {
        Self {
            rope: Rope::new(),
            selections: vec![Selection::cursor(0)],
            history: History::new(),
            modified: false,
        }
    }

    /// Create a buffer from existing text.
    pub fn from_text(text: &str) -> Self {
        Self {
            rope: Rope::from_str(text),
            selections: vec![Selection::cursor(0)],
            history: History::new(),
            modified: false,
        }
    }

    /// Get the full text content.
    pub fn text(&self) -> String {
        self.rope.to_string()
    }

    /// Get the number of characters in the buffer.
    pub fn len_chars(&self) -> usize {
        self.rope.len_chars()
    }

    /// Get the number of lines in the buffer.
    pub fn len_lines(&self) -> usize {
        self.rope.len_lines()
    }

    /// Check if the buffer is empty.
    pub fn is_empty(&self) -> bool {
        self.rope.len_chars() == 0
    }

    /// Get a specific line's text (0-indexed).
    pub fn line(&self, line_idx: usize) -> Option<String> {
        if line_idx < self.rope.len_lines() {
            Some(self.rope.line(line_idx).to_string())
        } else {
            None
        }
    }

    /// Convert a char index to (line, col) position.
    pub fn char_to_line_col(&self, char_idx: usize) -> (usize, usize) {
        let char_idx = char_idx.min(self.rope.len_chars());
        let line = self.rope.char_to_line(char_idx);
        let line_start = self.rope.line_to_char(line);
        (line, char_idx - line_start)
    }

    /// Convert a (line, col) position to char index.
    pub fn line_col_to_char(&self, line: usize, col: usize) -> usize {
        if line >= self.rope.len_lines() {
            return self.rope.len_chars();
        }
        let line_start = self.rope.line_to_char(line);
        let line_len = self.rope.line(line).len_chars();
        line_start + col.min(line_len)
    }

    /// Whether the buffer has been modified since last save.
    pub fn is_modified(&self) -> bool {
        self.modified
    }

    /// Mark the buffer as saved.
    pub fn mark_saved(&mut self) {
        self.modified = false;
    }

    /// Get current selections.
    pub fn selections(&self) -> &[Selection] {
        &self.selections
    }

    /// Set selections.
    pub fn set_selections(&mut self, selections: Vec<Selection>) {
        self.selections = if selections.is_empty() {
            vec![Selection::cursor(0)]
        } else {
            selections
        };
    }

    /// Apply a transaction to the buffer atomically.
    /// Returns the delta for the frontend.
    pub fn apply_transaction(&mut self, transaction: &Transaction) -> BufferDelta {
        let mut inverse_ops = Vec::new();
        let mut delta_edits = Vec::new();
        // Track cumulative offset as edits shift positions
        let mut offset: isize = 0;

        for edit in &transaction.edits {
            let adjusted_start = (edit.start as isize + offset) as usize;
            let adjusted_end = (edit.end as isize + offset) as usize;

            let adjusted_start = adjusted_start.min(self.rope.len_chars());
            let adjusted_end = adjusted_end.min(self.rope.len_chars());

            // Record inverse operation for undo
            let old_text: String = self.rope.slice(adjusted_start..adjusted_end).to_string();
            inverse_ops.push(EditOperation {
                start: adjusted_start,
                end: adjusted_start + edit.text.len(),
                text: old_text,
            });

            // Compute line/col for delta
            let (start_line, start_col) = self.char_to_line_col(adjusted_start);
            let (end_line, end_col) = self.char_to_line_col(adjusted_end);

            // Apply the edit
            self.rope.remove(adjusted_start..adjusted_end);
            if !edit.text.is_empty() {
                self.rope.insert(adjusted_start, &edit.text);
            }

            delta_edits.push(EditDelta {
                start_line,
                start_col,
                end_line,
                end_col,
                text: edit.text.clone(),
            });

            let old_len = adjusted_end - adjusted_start;
            let new_len = edit.text.len();
            offset += new_len as isize - old_len as isize;
        }

        // Push to history
        inverse_ops.reverse();
        self.history.push(inverse_ops);
        self.modified = true;

        BufferDelta {
            edits: delta_edits,
            content: self.rope.to_string(),
        }
    }

    /// Insert text at a character position.
    pub fn insert(&mut self, char_idx: usize, text: &str) -> BufferDelta {
        let transaction = Transaction {
            edits: vec![Edit {
                start: char_idx,
                end: char_idx,
                text: text.to_string(),
            }],
        };
        self.apply_transaction(&transaction)
    }

    /// Delete a range of characters.
    pub fn delete(&mut self, start: usize, end: usize) -> BufferDelta {
        let transaction = Transaction {
            edits: vec![Edit {
                start,
                end,
                text: String::new(),
            }],
        };
        self.apply_transaction(&transaction)
    }

    /// Replace a range with new text.
    pub fn replace(&mut self, start: usize, end: usize, text: &str) -> BufferDelta {
        let transaction = Transaction {
            edits: vec![Edit {
                start,
                end,
                text: text.to_string(),
            }],
        };
        self.apply_transaction(&transaction)
    }

    /// Apply operations and compute their inverse, returning delta and inverse ops.
    fn apply_ops(&mut self, ops: &[EditOperation]) -> (Vec<EditDelta>, Vec<EditOperation>) {
        let mut delta_edits = Vec::new();
        let mut inverse_ops = Vec::new();

        for op in ops {
            let start = op.start.min(self.rope.len_chars());
            let end = op.end.min(self.rope.len_chars());

            // Record inverse before applying
            let old_text: String = self.rope.slice(start..end).to_string();
            inverse_ops.push(EditOperation {
                start,
                end: start + op.text.len(),
                text: old_text,
            });

            let (start_line, start_col) = self.char_to_line_col(start);
            let (end_line, end_col) = self.char_to_line_col(end);

            self.rope.remove(start..end);
            if !op.text.is_empty() {
                self.rope.insert(start, &op.text);
            }

            delta_edits.push(EditDelta {
                start_line,
                start_col,
                end_line,
                end_col,
                text: op.text.clone(),
            });
        }

        inverse_ops.reverse();
        (delta_edits, inverse_ops)
    }

    /// Undo the last operation.
    pub fn undo(&mut self) -> Option<BufferDelta> {
        let ops = self.history.pop_undo()?;
        let (delta_edits, inverse_ops) = self.apply_ops(&ops);
        self.history.push_redo(inverse_ops);

        Some(BufferDelta {
            edits: delta_edits,
            content: self.rope.to_string(),
        })
    }

    /// Redo the last undone operation.
    pub fn redo(&mut self) -> Option<BufferDelta> {
        let ops = self.history.pop_redo()?;
        let (delta_edits, inverse_ops) = self.apply_ops(&ops);
        self.history.push_undo(inverse_ops);

        Some(BufferDelta {
            edits: delta_edits,
            content: self.rope.to_string(),
        })
    }
}

impl Default for Buffer {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_new_buffer_is_empty() {
        let buf = Buffer::new();
        assert!(buf.is_empty());
        assert_eq!(buf.len_chars(), 0);
        assert_eq!(buf.len_lines(), 1);
    }

    #[test]
    fn test_from_text() {
        let buf = Buffer::from_text("hello\nworld");
        assert_eq!(buf.len_chars(), 11);
        assert_eq!(buf.len_lines(), 2);
        assert_eq!(buf.text(), "hello\nworld");
    }

    #[test]
    fn test_insert_at_beginning() {
        let mut buf = Buffer::new();
        buf.insert(0, "hello");
        assert_eq!(buf.text(), "hello");
        assert!(buf.is_modified());
    }

    #[test]
    fn test_insert_at_end() {
        let mut buf = Buffer::from_text("hello");
        buf.insert(5, " world");
        assert_eq!(buf.text(), "hello world");
    }

    #[test]
    fn test_insert_in_middle() {
        let mut buf = Buffer::from_text("helo");
        buf.insert(2, "l");
        assert_eq!(buf.text(), "hello");
    }

    #[test]
    fn test_delete_range() {
        let mut buf = Buffer::from_text("hello world");
        buf.delete(5, 11);
        assert_eq!(buf.text(), "hello");
    }

    #[test]
    fn test_replace_range() {
        let mut buf = Buffer::from_text("hello world");
        buf.replace(6, 11, "rust");
        assert_eq!(buf.text(), "hello rust");
    }

    #[test]
    fn test_undo_redo() {
        let mut buf = Buffer::new();
        buf.insert(0, "hello");
        assert_eq!(buf.text(), "hello");

        buf.undo();
        assert_eq!(buf.text(), "");

        buf.redo();
        assert_eq!(buf.text(), "hello");
    }

    #[test]
    fn test_multiple_undo() {
        let mut buf = Buffer::new();
        buf.insert(0, "a");
        buf.insert(1, "b");
        buf.insert(2, "c");
        assert_eq!(buf.text(), "abc");

        buf.undo();
        assert_eq!(buf.text(), "ab");
        buf.undo();
        assert_eq!(buf.text(), "a");
        buf.undo();
        assert_eq!(buf.text(), "");
    }

    #[test]
    fn test_char_to_line_col() {
        let buf = Buffer::from_text("hello\nworld\nfoo");
        assert_eq!(buf.char_to_line_col(0), (0, 0));
        assert_eq!(buf.char_to_line_col(5), (0, 5));
        assert_eq!(buf.char_to_line_col(6), (1, 0));
        assert_eq!(buf.char_to_line_col(11), (1, 5));
        assert_eq!(buf.char_to_line_col(12), (2, 0));
    }

    #[test]
    fn test_line_col_to_char() {
        let buf = Buffer::from_text("hello\nworld");
        assert_eq!(buf.line_col_to_char(0, 0), 0);
        assert_eq!(buf.line_col_to_char(1, 0), 6);
        assert_eq!(buf.line_col_to_char(1, 3), 9);
    }

    #[test]
    fn test_get_line() {
        let buf = Buffer::from_text("line1\nline2\nline3");
        assert_eq!(buf.line(0), Some("line1\n".to_string()));
        assert_eq!(buf.line(2), Some("line3".to_string()));
        assert_eq!(buf.line(3), None);
    }

    #[test]
    fn test_unicode_handling() {
        let mut buf = Buffer::from_text("cafe\u{0301}");
        assert_eq!(buf.len_chars(), 5);
        buf.insert(5, " \u{1f600}");
        assert!(buf.text().contains('\u{1f600}'));
    }

    #[test]
    fn test_mark_saved() {
        let mut buf = Buffer::new();
        assert!(!buf.is_modified());
        buf.insert(0, "x");
        assert!(buf.is_modified());
        buf.mark_saved();
        assert!(!buf.is_modified());
    }

    #[test]
    fn test_transaction_multiple_edits() {
        let mut buf = Buffer::from_text("aXbXc");
        let tx = Transaction {
            edits: vec![
                Edit {
                    start: 1,
                    end: 2,
                    text: String::new(),
                },
                Edit {
                    start: 3,
                    end: 4,
                    text: String::new(),
                },
            ],
        };
        buf.apply_transaction(&tx);
        assert_eq!(buf.text(), "abc");
    }

    #[test]
    fn test_empty_buffer_operations() {
        let mut buf = Buffer::new();
        assert_eq!(buf.undo(), None);
        assert_eq!(buf.redo(), None);
        assert_eq!(buf.line(0), Some("".to_string()));
    }

    #[test]
    fn test_selections_default() {
        let buf = Buffer::new();
        assert_eq!(buf.selections().len(), 1);
        assert_eq!(buf.selections()[0].anchor, 0);
    }
}
