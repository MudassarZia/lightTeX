#include "editor/BracketMatcher.h"

namespace lighttex::editor {

static const QChar openBrackets[] = {'{', '[', '('};
static const QChar closeBrackets[] = {'}', ']', ')'};
static constexpr int bracketCount = 3;

std::vector<int> BracketMatcher::findMatchingBrackets(const QString& text, int cursorPos) const {
    if (cursorPos < 0 || cursorPos >= text.length()) return {};

    QChar ch = text[cursorPos];

    // Check if cursor is on an open bracket
    for (int i = 0; i < bracketCount; ++i) {
        if (ch == openBrackets[i]) {
            int match = findMatchingForward(text, cursorPos, openBrackets[i], closeBrackets[i]);
            if (match >= 0) return {cursorPos, match};
            return {};
        }
    }

    // Check if cursor is on a close bracket
    for (int i = 0; i < bracketCount; ++i) {
        if (ch == closeBrackets[i]) {
            int match = findMatchingBackward(text, cursorPos, openBrackets[i], closeBrackets[i]);
            if (match >= 0) return {match, cursorPos};
            return {};
        }
    }

    // Also check character before cursor
    if (cursorPos > 0) {
        QChar prev = text[cursorPos - 1];
        for (int i = 0; i < bracketCount; ++i) {
            if (prev == closeBrackets[i]) {
                int match = findMatchingBackward(text, cursorPos - 1, openBrackets[i], closeBrackets[i]);
                if (match >= 0) return {match, cursorPos - 1};
            }
        }
    }

    return {};
}

int BracketMatcher::findMatchingForward(const QString& text, int pos,
                                         QChar open, QChar close) const {
    int depth = 0;
    for (int i = pos; i < text.length(); ++i) {
        if (text[i] == open) ++depth;
        else if (text[i] == close) {
            --depth;
            if (depth == 0) return i;
        }
    }
    return -1;
}

int BracketMatcher::findMatchingBackward(const QString& text, int pos,
                                          QChar open, QChar close) const {
    int depth = 0;
    for (int i = pos; i >= 0; --i) {
        if (text[i] == close) ++depth;
        else if (text[i] == open) {
            --depth;
            if (depth == 0) return i;
        }
    }
    return -1;
}

} // namespace lighttex::editor
