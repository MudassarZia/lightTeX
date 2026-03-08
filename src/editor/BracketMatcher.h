#pragma once

#include <QString>
#include <vector>

namespace lighttex::editor {

class BracketMatcher {
public:
    BracketMatcher() = default;

    // Returns positions of matching brackets (0 or 2 entries)
    std::vector<int> findMatchingBrackets(const QString& text, int cursorPos) const;

private:
    int findMatchingForward(const QString& text, int pos, QChar open, QChar close) const;
    int findMatchingBackward(const QString& text, int pos, QChar open, QChar close) const;
};

} // namespace lighttex::editor
