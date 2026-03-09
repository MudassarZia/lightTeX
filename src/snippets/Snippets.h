#pragma once

#include <string>
#include <vector>

namespace lighttex::snippets {

struct Snippet {
  std::string trigger;
  std::string label;
  std::string body;
};

class SnippetManager {
public:
  SnippetManager();

  bool loadFromToml(const std::string &tomlStr);
  bool loadFromFile(const std::string &path);

  [[nodiscard]] const Snippet *findByTrigger(const std::string &trigger) const;
  [[nodiscard]] const std::vector<Snippet> &snippets() const {
    return snippets_;
  }

  static std::string expandBody(const std::string &body);

  void loadDefaults();

private:
  std::vector<Snippet> snippets_;
};

} // namespace lighttex::snippets
