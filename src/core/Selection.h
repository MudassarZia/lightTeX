#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace lighttex::core {

struct Selection {
  size_t anchor = 0;
  size_t head = 0;

  static Selection cursor(size_t pos) { return {pos, pos}; }
  static Selection range(size_t anchor, size_t head) { return {anchor, head}; }

  [[nodiscard]] bool isCursor() const { return anchor == head; }
  [[nodiscard]] bool isEmpty() const { return isCursor(); }
  [[nodiscard]] size_t start() const { return std::min(anchor, head); }
  [[nodiscard]] size_t end() const { return std::max(anchor, head); }
  [[nodiscard]] size_t length() const { return end() - start(); }

  [[nodiscard]] bool contains(size_t pos) const {
    return pos >= start() && pos < end();
  }

  [[nodiscard]] bool overlaps(const Selection &other) const {
    return start() < other.end() && other.start() < end();
  }

  [[nodiscard]] Selection merge(const Selection &other) const {
    return {std::min(anchor, other.anchor), std::max(head, other.head)};
  }

  [[nodiscard]] Selection clip(size_t maxChars) const {
    return {std::min(anchor, maxChars), std::min(head, maxChars)};
  }

  bool operator==(const Selection &other) const = default;
};

void normalizeSelections(std::vector<Selection> &selections);

} // namespace lighttex::core
