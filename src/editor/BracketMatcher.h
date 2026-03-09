#pragma once

#include <QString>
#include <QTextDocument>
#include <vector>

namespace lighttex::editor {

class BracketMatcher {
public:
    BracketMatcher() = default;

    // Returns positions of matching brackets (0 or 2 entries)
    std::vector<int> findMatchingBrackets(const QString& text, int cursorPos) const;

    // Document-based version — avoids full toPlainText() copy
    std::vector<int> findMatchingBrackets(const QTextDocument* doc, int cursorPos) const;

private:
    int findMatchingForward(const QString& text, int pos, QChar open, QChar close) const;
    int findMatchingBackward(const QString& text, int pos, QChar open, QChar close) const;
    int findMatchingForward(const QTextDocument* doc, int pos, QChar open, QChar close) const;
    int findMatchingBackward(const QTextDocument* doc, int pos, QChar open, QChar close) const;
};

} // namespace lighttex::editor
