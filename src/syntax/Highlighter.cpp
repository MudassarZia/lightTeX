#include "syntax/Highlighter.h"

#include <tree_sitter/api.h>
#include <cstring>

namespace lighttex::syntax {

Highlighter::Highlighter() {
    parser_ = ts_parser_new();
    if (parser_) {
        const TSLanguage* lang = tree_sitter_latex();
        if (!ts_parser_set_language(parser_, lang)) {
            ts_parser_delete(parser_);
            parser_ = nullptr;
        }
    }
}

Highlighter::~Highlighter() {
    if (tree_) ts_tree_delete(tree_);
    if (parser_) ts_parser_delete(parser_);
}

Highlighter::Highlighter(Highlighter&& other) noexcept
    : parser_(other.parser_), tree_(other.tree_) {
    other.parser_ = nullptr;
    other.tree_ = nullptr;
}

Highlighter& Highlighter::operator=(Highlighter&& other) noexcept {
    if (this != &other) {
        if (tree_) ts_tree_delete(tree_);
        if (parser_) ts_parser_delete(parser_);
        parser_ = other.parser_;
        tree_ = other.tree_;
        other.parser_ = nullptr;
        other.tree_ = nullptr;
    }
    return *this;
}

std::vector<HighlightEvent> Highlighter::parse(const std::string& source) {
    if (!parser_) return {};

    if (tree_) {
        ts_tree_delete(tree_);
        tree_ = nullptr;
    }

    tree_ = ts_parser_parse_string(parser_, nullptr, source.c_str(),
                                    static_cast<uint32_t>(source.size()));
    if (!tree_) return {};

    return walkTree(tree_, source);
}

std::vector<HighlightEvent> Highlighter::reparse(const std::string& source) {
    if (!parser_) return {};

    TSTree* oldTree = tree_;
    tree_ = ts_parser_parse_string(parser_, oldTree, source.c_str(),
                                    static_cast<uint32_t>(source.size()));

    if (oldTree) ts_tree_delete(oldTree);
    if (!tree_) return {};

    return walkTree(tree_, source);
}

std::optional<TokenKind> Highlighter::classifyNode(const char* nodeType,
                                                    bool isNamed,
                                                    bool hasChildren) {
    if (!nodeType) return std::nullopt;

    // Commands — only the command name itself (e.g. \textbf), not the
    // parent generic_command which includes arguments
    if (std::strcmp(nodeType, "command_name") == 0) {
        return TokenKind::Command;
    }

    // Environments
    if (std::strcmp(nodeType, "begin") == 0 ||
        std::strcmp(nodeType, "end") == 0 ||
        std::strcmp(nodeType, "environment_name") == 0) {
        return TokenKind::Environment;
    }

    // Math
    if (std::strcmp(nodeType, "inline_formula") == 0 ||
        std::strcmp(nodeType, "displayed_equation") == 0 ||
        std::strcmp(nodeType, "math_environment") == 0) {
        return TokenKind::MathDelimiter;
    }

    // Comments
    if (std::strcmp(nodeType, "line_comment") == 0 ||
        std::strcmp(nodeType, "block_comment") == 0 ||
        std::strcmp(nodeType, "comment") == 0) {
        return TokenKind::Comment;
    }

    // Brackets
    if (std::strcmp(nodeType, "{") == 0 ||
        std::strcmp(nodeType, "}") == 0 ||
        std::strcmp(nodeType, "[") == 0 ||
        std::strcmp(nodeType, "]") == 0 ||
        std::strcmp(nodeType, "(") == 0 ||
        std::strcmp(nodeType, ")") == 0) {
        return TokenKind::Bracket;
    }

    // Operators
    if (std::strcmp(nodeType, "&") == 0 ||
        std::strcmp(nodeType, "=") == 0 ||
        std::strcmp(nodeType, "_") == 0 ||
        std::strcmp(nodeType, "^") == 0) {
        return TokenKind::Operator;
    }

    // Numbers
    if (std::strcmp(nodeType, "number") == 0) {
        return TokenKind::Number;
    }

    // Text (leaf nodes only)
    if ((std::strcmp(nodeType, "text") == 0 ||
         std::strcmp(nodeType, "word") == 0) && !hasChildren) {
        return TokenKind::Text;
    }

    // Errors — only leaf ERROR nodes (no children)
    // Large ERROR subtrees from partial parses should still classify children normally
    if (std::strcmp(nodeType, "ERROR") == 0 && !hasChildren) {
        return TokenKind::Error;
    }

    return std::nullopt;
}

std::vector<HighlightEvent> Highlighter::walkTree(TSTree* tree,
                                                    const std::string& /*source*/) {
    std::vector<HighlightEvent> events;
    TSNode root = ts_tree_root_node(tree);

    struct StackEntry {
        TSNode node;
        bool visited;
    };

    std::vector<StackEntry> stack;
    stack.push_back({root, false});

    // Track parent ranges to avoid double-reporting
    std::vector<std::pair<size_t, size_t>> parentRanges;

    while (!stack.empty()) {
        auto& [node, visited] = stack.back();

        if (visited) {
            stack.pop_back();
            if (!parentRanges.empty()) {
                size_t startByte = ts_node_start_byte(node);
                size_t endByte = ts_node_end_byte(node);
                if (parentRanges.back().first == startByte &&
                    parentRanges.back().second == endByte) {
                    parentRanges.pop_back();
                }
            }
            continue;
        }

        visited = true;

        const char* type = ts_node_type(node);
        bool isNamed = ts_node_is_named(node);
        uint32_t childCount = ts_node_child_count(node);

        // Check if this node is inside a parent range we already classified
        size_t startByte = ts_node_start_byte(node);
        size_t endByte = ts_node_end_byte(node);

        bool insideParent = false;
        for (const auto& [ps, pe] : parentRanges) {
            if (startByte >= ps && endByte <= pe) {
                insideParent = true;
                break;
            }
        }

        if (!insideParent) {
            auto kind = classifyNode(type, isNamed, childCount > 0);
            if (kind) {
                TSPoint startPt = ts_node_start_point(node);
                TSPoint endPt = ts_node_end_point(node);

                events.push_back({
                    startByte,
                    endByte,
                    startPt.row,
                    startPt.column,
                    endPt.row,
                    endPt.column,
                    *kind
                });

                // Track this as a parent range to avoid double-reporting children
                parentRanges.push_back({startByte, endByte});
            }
        }

        // Push children in reverse order for DFS
        for (int i = static_cast<int>(childCount) - 1; i >= 0; --i) {
            TSNode child = ts_node_child(node, static_cast<uint32_t>(i));
            stack.push_back({child, false});
        }
    }

    return events;
}

} // namespace lighttex::syntax
