#include "syntax/SyntaxHighlighter.h"

#include <QColor>

#include <algorithm>

namespace lighttex::syntax {

SyntaxHighlighterBridge::SyntaxHighlighterBridge(QTextDocument* parent)
    : QSyntaxHighlighter(parent) {
    // Default colors (dark theme)
    colorMap_[TokenKind::Command] = QColor("#569cd6");
    colorMap_[TokenKind::Environment] = QColor("#c586c0");
    colorMap_[TokenKind::MathDelimiter] = QColor("#dcdcaa");
    colorMap_[TokenKind::MathContent] = QColor("#dcdcaa");
    colorMap_[TokenKind::Comment] = QColor("#6a9955");
    colorMap_[TokenKind::Text] = QColor("#d4d4d4");
    colorMap_[TokenKind::Bracket] = QColor("#ffd700");
    colorMap_[TokenKind::Operator] = QColor("#d4d4d4");
    colorMap_[TokenKind::Number] = QColor("#b5cea8");
    colorMap_[TokenKind::String] = QColor("#ce9178");
    colorMap_[TokenKind::Key] = QColor("#9cdcfe");
    colorMap_[TokenKind::Error] = QColor("#f44747");

    // Debounce re-parse: wait 50ms after last keystroke
    reparseTimer_.setSingleShot(true);
    reparseTimer_.setInterval(50);
    connect(&reparseTimer_, &QTimer::timeout, this, &SyntaxHighlighterBridge::doReparse);
}

void SyntaxHighlighterBridge::setTheme(const lighttex::theme::Theme& theme) {
    colorMap_[TokenKind::Command] = QColor(QString::fromStdString(theme.syntax.command));
    colorMap_[TokenKind::Environment] = QColor(QString::fromStdString(theme.syntax.environment));
    colorMap_[TokenKind::MathDelimiter] = QColor(QString::fromStdString(theme.syntax.math));
    colorMap_[TokenKind::MathContent] = QColor(QString::fromStdString(theme.syntax.math));
    colorMap_[TokenKind::Comment] = QColor(QString::fromStdString(theme.syntax.comment));
    colorMap_[TokenKind::Text] = QColor(QString::fromStdString(theme.colors.foreground));
    colorMap_[TokenKind::Bracket] = QColor(QString::fromStdString(theme.syntax.bracket));
    colorMap_[TokenKind::Number] = QColor(QString::fromStdString(theme.syntax.number));
    colorMap_[TokenKind::String] = QColor(QString::fromStdString(theme.syntax.string));
    colorMap_[TokenKind::Error] = QColor(QString::fromStdString(theme.syntax.error));

    // Re-parse with new colors
    scheduleReparse();
}

void SyntaxHighlighterBridge::scheduleReparse() {
    reparseTimer_.start();
}

void SyntaxHighlighterBridge::doReparse() {
    if (isHighlighting_) return;

    auto* doc = document();
    if (!doc) return;

    std::string source = doc->toPlainText().toStdString();
    cachedEvents_ = highlighter_.parse(source);

    // Sort by startByte for binary search in highlightBlock
    std::sort(cachedEvents_.begin(), cachedEvents_.end(),
              [](const HighlightEvent& a, const HighlightEvent& b) {
                  return a.startByte < b.startByte;
              });

    isHighlighting_ = true;
    rehighlight();
    isHighlighting_ = false;
}

void SyntaxHighlighterBridge::highlightBlock(const QString& text) {
    if (text.isEmpty() || !isHighlighting_) return;

    int blockStart = currentBlock().position();
    int blockEnd = blockStart + text.length();

    // Binary search: find first event that could overlap this block
    // An event overlaps if evEnd > blockStart, so find first event
    // where startByte could be relevant (startByte < blockEnd)
    HighlightEvent probe;
    probe.startByte = static_cast<size_t>(blockStart);
    auto it = std::lower_bound(
        cachedEvents_.begin(), cachedEvents_.end(), probe,
        [](const HighlightEvent& ev, const HighlightEvent& val) {
            return ev.endByte <= val.startByte;
        });

    for (; it != cachedEvents_.end(); ++it) {
        int evStart = static_cast<int>(it->startByte);
        int evEnd = static_cast<int>(it->endByte);

        // Past this block — done
        if (evStart >= blockEnd) break;

        // Check overlap with current block
        if (evEnd <= blockStart) continue;

        int start = std::max(evStart - blockStart, 0);
        int end = std::min(evEnd - blockStart, static_cast<int>(text.length()));
        int length = end - start;

        if (length > 0) {
            setFormat(start, length, formatForKind(it->kind));
        }
    }
}

QTextCharFormat SyntaxHighlighterBridge::formatForKind(TokenKind kind) const {
    QTextCharFormat fmt;
    auto it = colorMap_.find(kind);
    if (it != colorMap_.end()) {
        fmt.setForeground(it.value());
    }
    if (kind == TokenKind::Comment) {
        fmt.setFontItalic(true);
    }
    if (kind == TokenKind::Error) {
        fmt.setFontUnderline(true);
        fmt.setUnderlineColor(QColor("#f44747"));
    }
    return fmt;
}

} // namespace lighttex::syntax
