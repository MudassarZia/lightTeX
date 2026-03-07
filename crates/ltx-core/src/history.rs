/// An individual edit operation stored in history.
#[derive(Debug, Clone)]
pub struct EditOperation {
    pub start: usize,
    pub end: usize,
    pub text: String,
}

/// Undo/redo history using a linear stack model.
/// Each entry is a group of edit operations that form one logical change.
#[derive(Debug, Clone)]
pub struct History {
    undo_stack: Vec<Vec<EditOperation>>,
    redo_stack: Vec<Vec<EditOperation>>,
}

impl History {
    pub fn new() -> Self {
        Self {
            undo_stack: Vec::new(),
            redo_stack: Vec::new(),
        }
    }

    /// Push a new set of inverse operations onto the undo stack.
    /// Clears the redo stack (new edit branch).
    pub fn push(&mut self, inverse_ops: Vec<EditOperation>) {
        self.undo_stack.push(inverse_ops);
        self.redo_stack.clear();
    }

    /// Push directly onto the undo stack without clearing redo.
    pub fn push_undo(&mut self, ops: Vec<EditOperation>) {
        self.undo_stack.push(ops);
    }

    /// Push onto the redo stack.
    pub fn push_redo(&mut self, ops: Vec<EditOperation>) {
        self.redo_stack.push(ops);
    }

    /// Pop from the undo stack.
    pub fn pop_undo(&mut self) -> Option<Vec<EditOperation>> {
        self.undo_stack.pop()
    }

    /// Pop from the redo stack.
    pub fn pop_redo(&mut self) -> Option<Vec<EditOperation>> {
        self.redo_stack.pop()
    }

    /// Pop and return the last undo entry, moving it to redo.
    pub fn undo(&mut self) -> Option<Vec<EditOperation>> {
        let ops = self.undo_stack.pop()?;
        self.redo_stack.push(ops.clone());
        Some(ops)
    }

    /// Pop and return the last redo entry, moving it to undo.
    pub fn redo(&mut self) -> Option<Vec<EditOperation>> {
        let ops = self.redo_stack.pop()?;
        self.undo_stack.push(ops.clone());
        Some(ops)
    }

    /// Check if undo is available.
    pub fn can_undo(&self) -> bool {
        !self.undo_stack.is_empty()
    }

    /// Check if redo is available.
    pub fn can_redo(&self) -> bool {
        !self.redo_stack.is_empty()
    }

    /// Clear all history.
    pub fn clear(&mut self) {
        self.undo_stack.clear();
        self.redo_stack.clear();
    }
}

impl Default for History {
    fn default() -> Self {
        Self::new()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_empty_history() {
        let history = History::new();
        assert!(!history.can_undo());
        assert!(!history.can_redo());
    }

    #[test]
    fn test_push_and_undo() {
        let mut history = History::new();
        history.push(vec![EditOperation {
            start: 0,
            end: 5,
            text: "hello".to_string(),
        }]);
        assert!(history.can_undo());

        let ops = history.undo().unwrap();
        assert_eq!(ops.len(), 1);
        assert_eq!(ops[0].text, "hello");
        assert!(!history.can_undo());
        assert!(history.can_redo());
    }

    #[test]
    fn test_redo_after_undo() {
        let mut history = History::new();
        history.push(vec![EditOperation {
            start: 0,
            end: 0,
            text: String::new(),
        }]);
        history.undo();
        assert!(history.can_redo());

        history.redo();
        assert!(history.can_undo());
        assert!(!history.can_redo());
    }

    #[test]
    fn test_new_edit_clears_redo() {
        let mut history = History::new();
        history.push(vec![EditOperation {
            start: 0,
            end: 0,
            text: String::new(),
        }]);
        history.undo();
        assert!(history.can_redo());

        history.push(vec![EditOperation {
            start: 0,
            end: 0,
            text: "new".to_string(),
        }]);
        assert!(!history.can_redo());
    }

    #[test]
    fn test_clear() {
        let mut history = History::new();
        history.push(vec![EditOperation {
            start: 0,
            end: 0,
            text: String::new(),
        }]);
        history.clear();
        assert!(!history.can_undo());
        assert!(!history.can_redo());
    }
}
