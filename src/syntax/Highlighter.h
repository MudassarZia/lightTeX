#pragma once

#include "syntax/TokenKind.h"

#include <optional>
#include <string>
#include <string_view>
#include <vector>

// tree-sitter C API forward declarations
extern "C" {
    typedef struct TSParser TSParser;
    typedef struct TSTree TSTree;
    typedef struct TSLanguage TSLanguage;
    typedef struct TSNode TSNode;
    const TSLanguage* tree_sitter_latex();
}

namespace lighttex::syntax {

struct HighlightEvent {
    size_t startByte;
    size_t endByte;
    size_t startRow;
    size_t startCol;
    size_t endRow;
    size_t endCol;
    TokenKind kind;
};

class Highlighter {
public:
    Highlighter();
    ~Highlighter();

    Highlighter(const Highlighter&) = delete;
    Highlighter& operator=(const Highlighter&) = delete;
    Highlighter(Highlighter&& other) noexcept;
    Highlighter& operator=(Highlighter&& other) noexcept;

    [[nodiscard]] bool isValid() const { return parser_ != nullptr; }

    std::vector<HighlightEvent> parse(const std::string& source);
    std::vector<HighlightEvent> reparse(const std::string& source);

    static std::optional<TokenKind> classifyNode(const char* nodeType,
                                                  bool isNamed,
                                                  bool hasChildren);

private:
    static std::vector<HighlightEvent> walkTree(TSTree* tree, const std::string& source);

    TSParser* parser_ = nullptr;
    TSTree* tree_ = nullptr;
};

} // namespace lighttex::syntax
