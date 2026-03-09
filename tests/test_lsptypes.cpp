#include "lsp/LspTypes.h"

#include <gtest/gtest.h>

using namespace lighttex::lsp;

TEST(LspTypesTest, PositionToJson) {
    Position pos{10, 5};
    QJsonObject json = pos.toJson();
    EXPECT_EQ(json["line"].toInt(), 10);
    EXPECT_EQ(json["character"].toInt(), 5);
}

TEST(LspTypesTest, PositionFromJson) {
    QJsonObject json{{"line", 42}, {"character", 7}};
    Position pos = Position::fromJson(json);
    EXPECT_EQ(pos.line, 42);
    EXPECT_EQ(pos.character, 7);
}

TEST(LspTypesTest, RangeRoundTrip) {
    Range range{{1, 2}, {3, 4}};
    QJsonObject json = range.toJson();
    Range parsed = Range::fromJson(json);
    EXPECT_EQ(parsed.start.line, 1);
    EXPECT_EQ(parsed.start.character, 2);
    EXPECT_EQ(parsed.end.line, 3);
    EXPECT_EQ(parsed.end.character, 4);
}

TEST(LspTypesTest, CompletionItemFromJson) {
    QJsonObject json{
        {"label", "\\section"},
        {"kind", 14},
        {"detail", "Section header"},
        {"insertText", "\\section{$1}"}
    };
    CompletionItem item = CompletionItem::fromJson(json);
    EXPECT_EQ(item.label, "\\section");
    EXPECT_EQ(item.kind, 14);
    EXPECT_EQ(item.detail, "Section header");
    EXPECT_EQ(item.insertText, "\\section{$1}");
}

TEST(LspTypesTest, CompletionItemFallbackInsertText) {
    QJsonObject json{{"label", "\\begin"}};
    CompletionItem item = CompletionItem::fromJson(json);
    EXPECT_EQ(item.insertText, "\\begin");
}

TEST(LspTypesTest, HoverFromJsonString) {
    QJsonObject json{{"contents", "Some hover text"}};
    Hover h = Hover::fromJson(json);
    EXPECT_EQ(h.contents, "Some hover text");
}

TEST(LspTypesTest, HoverFromJsonMarkupContent) {
    QJsonObject json{
        {"contents", QJsonObject{
            {"kind", "markdown"},
            {"value", "**bold** text"}
        }}
    };
    Hover h = Hover::fromJson(json);
    EXPECT_EQ(h.contents, "**bold** text");
}

TEST(LspTypesTest, DiagnosticFromJson) {
    QJsonObject json{
        {"range", QJsonObject{
            {"start", QJsonObject{{"line", 5}, {"character", 0}}},
            {"end", QJsonObject{{"line", 5}, {"character", 10}}}
        }},
        {"severity", 1},
        {"message", "Undefined control sequence"},
        {"source", "texlab"}
    };
    Diagnostic d = Diagnostic::fromJson(json);
    EXPECT_EQ(d.range.start.line, 5);
    EXPECT_EQ(d.severity, DiagnosticSeverity::Error);
    EXPECT_EQ(d.message, "Undefined control sequence");
    EXPECT_EQ(d.source, "texlab");
}

TEST(LspTypesTest, LocationFromJson) {
    QJsonObject json{
        {"uri", "file:///tmp/test.tex"},
        {"range", QJsonObject{
            {"start", QJsonObject{{"line", 0}, {"character", 0}}},
            {"end", QJsonObject{{"line", 0}, {"character", 5}}}
        }}
    };
    Location loc = Location::fromJson(json);
    EXPECT_EQ(loc.uri, "file:///tmp/test.tex");
    EXPECT_EQ(loc.range.start.line, 0);
}
