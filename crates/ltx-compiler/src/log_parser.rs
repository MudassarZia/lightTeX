use regex::Regex;
use serde::{Deserialize, Serialize};
use std::sync::LazyLock;

/// Kind of compiler message.
#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub enum MessageKind {
    Error,
    Warning,
    BadBox,
    Info,
}

/// A parsed message from the LaTeX compiler log.
#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct CompileMessage {
    pub kind: MessageKind,
    pub file: Option<String>,
    pub line: Option<usize>,
    pub message: String,
}

static ERROR_RE: LazyLock<Regex> =
    LazyLock::new(|| Regex::new(r"^(?:(.+?):(\d+): )?! (.+)$").unwrap());

static WARNING_RE: LazyLock<Regex> =
    LazyLock::new(|| Regex::new(r"(?i)^(?:LaTeX |Package (?:\w+) )?Warning[:\s]*(.+)$").unwrap());

static BADBOX_RE: LazyLock<Regex> =
    LazyLock::new(|| Regex::new(r"^((?:Over|Under)full \\[hv]box .+)$").unwrap());

static LINE_RE: LazyLock<Regex> = LazyLock::new(|| Regex::new(r"on input line (\d+)").unwrap());

/// Parse a LaTeX log file into structured messages.
pub fn parse_log(log_content: &str) -> Vec<CompileMessage> {
    let mut messages = Vec::new();

    for line in log_content.lines() {
        let line = line.trim();

        // Match errors: "./file.tex:12: ! Undefined control sequence."
        // or just "! Error message"
        if let Some(caps) = ERROR_RE.captures(line) {
            let file = caps.get(1).map(|m| m.as_str().to_string());
            let line_num = caps.get(2).and_then(|m| m.as_str().parse().ok());
            let msg = caps[3].to_string();
            messages.push(CompileMessage {
                kind: MessageKind::Error,
                file,
                line: line_num,
                message: msg,
            });
            continue;
        }

        // Match warnings
        if let Some(caps) = WARNING_RE.captures(line) {
            let msg = caps[1].to_string();
            let line_num = LINE_RE.captures(&msg).and_then(|c| c[1].parse().ok());
            messages.push(CompileMessage {
                kind: MessageKind::Warning,
                file: None,
                line: line_num,
                message: msg,
            });
            continue;
        }

        // Match bad boxes
        if let Some(caps) = BADBOX_RE.captures(line) {
            let msg = caps[1].to_string();
            let line_num = LINE_RE.captures(&msg).and_then(|c| c[1].parse().ok());
            messages.push(CompileMessage {
                kind: MessageKind::BadBox,
                file: None,
                line: line_num,
                message: msg,
            });
        }
    }

    messages
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_error() {
        let log = "./main.tex:10: ! Undefined control sequence.";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 1);
        assert_eq!(msgs[0].kind, MessageKind::Error);
        assert_eq!(msgs[0].file.as_deref(), Some("./main.tex"));
        assert_eq!(msgs[0].line, Some(10));
        assert_eq!(msgs[0].message, "Undefined control sequence.");
    }

    #[test]
    fn test_parse_error_no_file() {
        let log = "! Missing $ inserted.";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 1);
        assert_eq!(msgs[0].kind, MessageKind::Error);
        assert!(msgs[0].file.is_none());
        assert_eq!(msgs[0].message, "Missing $ inserted.");
    }

    #[test]
    fn test_parse_warning() {
        let log = "LaTeX Warning: Reference `fig:1' on page 3 undefined on input line 42.";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 1);
        assert_eq!(msgs[0].kind, MessageKind::Warning);
        assert_eq!(msgs[0].line, Some(42));
    }

    #[test]
    fn test_parse_package_warning() {
        let log = "Package hyperref Warning: Token not allowed in a PDF string.";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 1);
        assert_eq!(msgs[0].kind, MessageKind::Warning);
    }

    #[test]
    fn test_parse_badbox() {
        let log = "Overfull \\hbox (3.45pt too wide) in paragraph at lines 10--15";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 1);
        assert_eq!(msgs[0].kind, MessageKind::BadBox);
    }

    #[test]
    fn test_parse_empty_log() {
        let msgs = parse_log("");
        assert!(msgs.is_empty());
    }

    #[test]
    fn test_parse_success_log() {
        let log = "This is pdfTeX, Version 3.14159265\nOutput written on main.pdf (3 pages, 45678 bytes).";
        let msgs = parse_log(log);
        assert!(msgs.is_empty());
    }

    #[test]
    fn test_parse_multiple_errors() {
        let log = "! Missing $ inserted.\n! Extra alignment tab has been changed to \\cr.";
        let msgs = parse_log(log);
        assert_eq!(msgs.len(), 2);
        assert!(msgs.iter().all(|m| m.kind == MessageKind::Error));
    }
}
