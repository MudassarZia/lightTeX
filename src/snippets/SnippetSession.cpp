#include "snippets/SnippetSession.h"

#include <algorithm>

namespace lighttex::snippets {

bool SnippetSession::start(const std::string& body, int insertOffset) {
    insertOffset_ = insertOffset;
    currentIndex_ = 0;
    tabStops_.clear();
    expandedText_.clear();
    active_ = false;

    parseBody(body);

    if (tabStops_.empty()) {
        active_ = false;
        return false;
    }

    // Sort tabstops: $1, $2, ... with $0 last
    std::sort(tabStops_.begin(), tabStops_.end(),
              [](const TabStop& a, const TabStop& b) {
                  if (a.number == 0) return false;
                  if (b.number == 0) return true;
                  return a.number < b.number;
              });

    active_ = true;
    currentIndex_ = 0;
    return true;
}

bool SnippetSession::nextTabStop() {
    if (!active_) return false;
    if (currentIndex_ + 1 >= static_cast<int>(tabStops_.size())) {
        active_ = false;
        return false;
    }
    ++currentIndex_;
    // If we reached $0 (final position), deactivate
    if (tabStops_[static_cast<size_t>(currentIndex_)].number == 0) {
        active_ = false;
    }
    return true;
}

bool SnippetSession::prevTabStop() {
    if (!active_ || currentIndex_ <= 0) return false;
    --currentIndex_;
    return true;
}

int SnippetSession::currentOffset() const {
    if (currentIndex_ < 0 ||
        currentIndex_ >= static_cast<int>(tabStops_.size()))
        return insertOffset_;
    return insertOffset_ + tabStops_[static_cast<size_t>(currentIndex_)].offset;
}

int SnippetSession::currentLength() const {
    if (currentIndex_ < 0 ||
        currentIndex_ >= static_cast<int>(tabStops_.size()))
        return 0;
    return tabStops_[static_cast<size_t>(currentIndex_)].length;
}

void SnippetSession::parseBody(const std::string& body) {
    expandedText_.clear();
    tabStops_.clear();

    size_t i = 0;
    while (i < body.size()) {
        if (body[i] == '$' && i + 1 < body.size()) {
            if (body[i + 1] == '{') {
                // ${N:placeholder} or ${N}
                size_t numStart = i + 2;
                size_t numEnd = numStart;
                while (numEnd < body.size() &&
                       body[numEnd] >= '0' && body[numEnd] <= '9') {
                    ++numEnd;
                }

                int num = 0;
                if (numEnd > numStart) {
                    num = std::stoi(body.substr(numStart, numEnd - numStart));
                }

                if (numEnd < body.size() && body[numEnd] == ':') {
                    // Has placeholder
                    size_t closePos = body.find('}', numEnd + 1);
                    if (closePos != std::string::npos) {
                        std::string placeholder =
                            body.substr(numEnd + 1, closePos - numEnd - 1);
                        TabStop ts;
                        ts.number = num;
                        ts.offset = static_cast<int>(expandedText_.size());
                        ts.length = static_cast<int>(placeholder.size());
                        ts.placeholder = placeholder;
                        tabStops_.push_back(ts);
                        expandedText_ += placeholder;
                        i = closePos + 1;
                        continue;
                    }
                } else if (numEnd < body.size() && body[numEnd] == '}') {
                    // ${N} — no placeholder
                    TabStop ts;
                    ts.number = num;
                    ts.offset = static_cast<int>(expandedText_.size());
                    ts.length = 0;
                    tabStops_.push_back(ts);
                    i = numEnd + 1;
                    continue;
                }
            } else if (body[i + 1] >= '0' && body[i + 1] <= '9') {
                // $N
                int num = body[i + 1] - '0';
                TabStop ts;
                ts.number = num;
                ts.offset = static_cast<int>(expandedText_.size());
                ts.length = 0;
                tabStops_.push_back(ts);
                i += 2;
                continue;
            }
        }
        expandedText_ += body[i];
        ++i;
    }
}

} // namespace lighttex::snippets
