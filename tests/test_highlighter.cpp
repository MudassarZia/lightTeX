#include "syntax/Highlighter.h"
#include <gtest/gtest.h>

using namespace lighttex::syntax;

TEST(Highlighter, Creation) {
    Highlighter h;
    EXPECT_TRUE(h.isValid());
}

TEST(Highlighter, ParseSimpleDocument) {
    Highlighter h;
    std::string source = R"(\documentclass{article}
\begin{document}
\textbf{Hello World}
\end{document})";

    auto events = h.parse(source);
    EXPECT_FALSE(events.empty());

    // Should find at least one command token (from \textbf's command_name)
    bool hasCommand = false;
    for (const auto& e : events) {
        if (e.kind == TokenKind::Command) {
            hasCommand = true;
            break;
        }
    }
    EXPECT_TRUE(hasCommand);
}

TEST(Highlighter, ParseMath) {
    Highlighter h;
    std::string source = "$E = mc^2$";
    auto events = h.parse(source);
    EXPECT_FALSE(events.empty());

    // Should contain math-related tokens
    bool hasMath = false;
    for (const auto& e : events) {
        if (e.kind == TokenKind::MathDelimiter || e.kind == TokenKind::Operator) {
            hasMath = true;
            break;
        }
    }
    EXPECT_TRUE(hasMath);
}

TEST(Highlighter, ParseComment) {
    Highlighter h;
    std::string source = "% This is a comment\nText";
    auto events = h.parse(source);

    bool hasComment = false;
    for (const auto& e : events) {
        if (e.kind == TokenKind::Comment) {
            hasComment = true;
            break;
        }
    }
    EXPECT_TRUE(hasComment);
}

TEST(Highlighter, IncrementalReparse) {
    Highlighter h;
    std::string source1 = "\\textbf{Hello}";
    auto events1 = h.parse(source1);
    EXPECT_FALSE(events1.empty());

    std::string source2 = "\\textbf{Hello World}";
    auto events2 = h.reparse(source2);
    EXPECT_FALSE(events2.empty());
}

TEST(Highlighter, EmptyDocument) {
    Highlighter h;
    auto events = h.parse("");
    // Empty doc may or may not produce events depending on grammar
    // Just ensure it doesn't crash
    (void)events;
}

TEST(Highlighter, EnvironmentDetection) {
    Highlighter h;
    std::string source = R"(\begin{enumerate}
\item First
\item Second
\end{enumerate})";

    auto events = h.parse(source);
    bool hasEnvironment = false;
    for (const auto& e : events) {
        if (e.kind == TokenKind::Environment) {
            hasEnvironment = true;
            break;
        }
    }
    EXPECT_TRUE(hasEnvironment);
}

TEST(Highlighter, ClassifyNodeCommand) {
    auto kind = Highlighter::classifyNode("command_name", true, false);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, TokenKind::Command);
}

TEST(Highlighter, GenericCommandNotClassifiedAsCommand) {
    // generic_command is a parent node wrapping \command{args} —
    // classifying it would paint the entire thing (including arguments) one color
    auto kind = Highlighter::classifyNode("generic_command", true, true);
    EXPECT_FALSE(kind.has_value());
}

TEST(Highlighter, LeafErrorClassified) {
    // Leaf ERROR nodes (no children) should be marked as errors
    auto kind = Highlighter::classifyNode("ERROR", true, false);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, TokenKind::Error);
}

TEST(Highlighter, ParentErrorNotClassified) {
    // ERROR nodes with children should NOT be classified —
    // their children should be walked and classified normally
    auto kind = Highlighter::classifyNode("ERROR", true, true);
    EXPECT_FALSE(kind.has_value());
}

TEST(Highlighter, CommandArgumentsNotAllBlue) {
    // Parse \textbf{Hello} — "Hello" should NOT be classified as Command
    Highlighter h;
    std::string source = "\\textbf{Hello}";
    auto events = h.parse(source);

    // Verify that not everything is Command — there should be brackets, text, etc.
    int commandEvents = 0;
    int nonCommandEvents = 0;
    for (const auto& e : events) {
        if (e.kind == TokenKind::Command) {
            commandEvents++;
        } else {
            nonCommandEvents++;
        }
    }
    EXPECT_GT(commandEvents, 0);    // should have at least one command
    EXPECT_GT(nonCommandEvents, 0); // should have other token types too
}

TEST(Highlighter, NestedMathDoesNotPaintAllError) {
    // Complex nested math like $\vcenter{\hbox{...}}$ should not
    // make the rest of the document all red
    Highlighter h;
    std::string source = "Before $x^2$ middle $\\vcenter{\\hbox{test}}$ after \\textbf{end}";
    auto events = h.parse(source);

    // Count how much of the source is classified as Error
    size_t errorBytes = 0;
    for (const auto& e : events) {
        if (e.kind == TokenKind::Error) {
            errorBytes += (e.endByte - e.startByte);
        }
    }
    // Error highlighting should not dominate (less than half the source)
    EXPECT_LT(errorBytes, source.size() / 2);
}

TEST(Highlighter, ClassifyNodeEnvironmentName) {
    auto kind = Highlighter::classifyNode("environment_name", true, false);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, TokenKind::Environment);
}

TEST(Highlighter, ClassifyNodeComment) {
    auto kind = Highlighter::classifyNode("line_comment", true, false);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, TokenKind::Comment);
}

TEST(Highlighter, ClassifyNodeBracket) {
    auto kind = Highlighter::classifyNode("{", false, false);
    ASSERT_TRUE(kind.has_value());
    EXPECT_EQ(*kind, TokenKind::Bracket);
}

TEST(Highlighter, ClassifyNodeUnknown) {
    auto kind = Highlighter::classifyNode("some_random_node", true, false);
    EXPECT_FALSE(kind.has_value());
}
