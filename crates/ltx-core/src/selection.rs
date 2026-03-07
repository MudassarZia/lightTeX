use serde::{Deserialize, Serialize};

/// Represents a selection or cursor position in the buffer.
/// When anchor == head, it's a cursor (no selection).
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct Selection {
    /// The fixed end of the selection.
    pub anchor: usize,
    /// The moving end of the selection (cursor position).
    pub head: usize,
}

impl Selection {
    /// Create a cursor (zero-width selection) at the given position.
    pub fn cursor(pos: usize) -> Self {
        Self {
            anchor: pos,
            head: pos,
        }
    }

    /// Create a selection from anchor to head.
    pub fn new(anchor: usize, head: usize) -> Self {
        Self { anchor, head }
    }

    /// Whether this is a cursor (no text selected).
    pub fn is_cursor(&self) -> bool {
        self.anchor == self.head
    }

    /// Get the smaller of anchor and head.
    pub fn start(&self) -> usize {
        self.anchor.min(self.head)
    }

    /// Get the larger of anchor and head.
    pub fn end(&self) -> usize {
        self.anchor.max(self.head)
    }

    /// Get the length of the selection.
    pub fn len(&self) -> usize {
        self.end() - self.start()
    }

    /// Whether the selection is empty (same as is_cursor).
    pub fn is_empty(&self) -> bool {
        self.is_cursor()
    }

    /// Check if a position is within this selection.
    pub fn contains(&self, pos: usize) -> bool {
        pos >= self.start() && pos < self.end()
    }

    /// Check if two selections overlap.
    pub fn overlaps(&self, other: &Selection) -> bool {
        self.start() < other.end() && other.start() < self.end()
    }

    /// Merge two overlapping selections.
    pub fn merge(&self, other: &Selection) -> Selection {
        Selection {
            anchor: self.anchor.min(other.anchor),
            head: self.head.max(other.head),
        }
    }

    /// Clip the selection to be within buffer bounds.
    pub fn clip(&self, max_chars: usize) -> Selection {
        Selection {
            anchor: self.anchor.min(max_chars),
            head: self.head.min(max_chars),
        }
    }
}

/// Merge overlapping selections and sort them.
pub fn normalize_selections(selections: &mut Vec<Selection>) {
    if selections.is_empty() {
        return;
    }
    selections.sort_by_key(|s| s.start());
    let mut merged = vec![selections[0]];
    for sel in selections.iter().skip(1) {
        let last = merged.last().unwrap();
        if sel.start() <= last.end() {
            let m = last.merge(sel);
            *merged.last_mut().unwrap() = m;
        } else {
            merged.push(*sel);
        }
    }
    *selections = merged;
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_cursor() {
        let sel = Selection::cursor(5);
        assert!(sel.is_cursor());
        assert!(sel.is_empty());
        assert_eq!(sel.start(), 5);
        assert_eq!(sel.end(), 5);
        assert_eq!(sel.len(), 0);
    }

    #[test]
    fn test_selection_range() {
        let sel = Selection::new(3, 10);
        assert!(!sel.is_cursor());
        assert_eq!(sel.start(), 3);
        assert_eq!(sel.end(), 10);
        assert_eq!(sel.len(), 7);
    }

    #[test]
    fn test_backward_selection() {
        let sel = Selection::new(10, 3);
        assert_eq!(sel.start(), 3);
        assert_eq!(sel.end(), 10);
    }

    #[test]
    fn test_contains() {
        let sel = Selection::new(5, 10);
        assert!(sel.contains(5));
        assert!(sel.contains(7));
        assert!(!sel.contains(10));
        assert!(!sel.contains(4));
    }

    #[test]
    fn test_overlaps() {
        let a = Selection::new(0, 5);
        let b = Selection::new(3, 8);
        let c = Selection::new(5, 10);
        assert!(a.overlaps(&b));
        assert!(!a.overlaps(&c));
    }

    #[test]
    fn test_normalize_selections() {
        let mut sels = vec![
            Selection::new(5, 10),
            Selection::new(0, 3),
            Selection::new(8, 15),
        ];
        normalize_selections(&mut sels);
        assert_eq!(sels.len(), 2);
        assert_eq!(sels[0].start(), 0);
        assert_eq!(sels[0].end(), 3);
        assert_eq!(sels[1].start(), 5);
        assert_eq!(sels[1].end(), 15);
    }

    #[test]
    fn test_clip() {
        let sel = Selection::new(5, 100);
        let clipped = sel.clip(50);
        assert_eq!(clipped.end(), 50);
    }
}
