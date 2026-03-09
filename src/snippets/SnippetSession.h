#pragma once

#include "snippets/Snippets.h"

#include <string>
#include <vector>

namespace lighttex::snippets {

struct TabStop {
  int number;
  int offset; // position in expanded text
  int length; // length of placeholder
  std::string placeholder;
};

class SnippetSession {
public:
  SnippetSession() = default;

  // Parse body into expanded text + tabstops
  bool start(const std::string &body, int insertOffset);

  // Navigate tabstops
  bool nextTabStop();
  bool prevTabStop();
  void cancel() { active_ = false; }

  [[nodiscard]] bool isActive() const { return active_; }
  [[nodiscard]] const std::string &expandedText() const {
    return expandedText_;
  }
  [[nodiscard]] int currentOffset() const;
  [[nodiscard]] int currentLength() const;
  [[nodiscard]] int currentTabStopIndex() const { return currentIndex_; }

private:
  void parseBody(const std::string &body);

  bool active_ = false;
  int insertOffset_ = 0;
  int currentIndex_ = 0;
  std::string expandedText_;
  std::vector<TabStop> tabStops_; // sorted by number
};

} // namespace lighttex::snippets
