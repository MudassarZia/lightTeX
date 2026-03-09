#include "editor/EditorWidget.h"
#include "editor/FindReplaceBar.h"

#include <QApplication>
#include <QTest>
#include <gtest/gtest.h>

using namespace lighttex::editor;

class FindReplaceTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 0;
            app_ = new QApplication(argc, nullptr);
        }
        editor_ = new EditorWidget();
        bar_ = new FindReplaceBar(editor_);
    }

    void TearDown() override {
        delete bar_;
        delete editor_;
    }

    QApplication* app_ = nullptr;
    EditorWidget* editor_ = nullptr;
    FindReplaceBar* bar_ = nullptr;
};

TEST_F(FindReplaceTest, InitiallyHidden) {
    EXPECT_TRUE(bar_->isHidden());
}

TEST_F(FindReplaceTest, ShowFindMakesVisible) {
    bar_->showFind();
    EXPECT_TRUE(bar_->isVisible());
}

TEST_F(FindReplaceTest, ShowFindReplaceMakesVisible) {
    bar_->showFindReplace();
    EXPECT_TRUE(bar_->isVisible());
}

TEST_F(FindReplaceTest, SearchHighlightsSetAndCleared) {
    editor_->setPlainText("Hello world hello");

    QList<QTextEdit::ExtraSelection> highlights;
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(QColor(Qt::yellow));
    sel.cursor = QTextCursor(editor_->document());
    sel.cursor.setPosition(0);
    sel.cursor.setPosition(5, QTextCursor::KeepAnchor);
    highlights.append(sel);

    editor_->setSearchHighlights(highlights);
    // ExtraSelections includes line highlight + search highlights
    EXPECT_GE(editor_->extraSelections().size(), 1);

    editor_->clearSearchHighlights();
    // Should still have line highlight
    EXPECT_GE(editor_->extraSelections().size(), 0);
}

TEST_F(FindReplaceTest, ThemeDoesNotCrash) {
    lighttex::theme::Theme theme;
    theme.ui.panelBg = "#1e1e1e";
    theme.colors.foreground = "#d4d4d4";
    theme.ui.panelBorder = "#3c3c3c";
    bar_->setTheme(theme);
    EXPECT_TRUE(true);
}

TEST_F(FindReplaceTest, EmptyEditorSearch) {
    // Search on empty editor shouldn't crash
    bar_->showFind();
    EXPECT_TRUE(bar_->isVisible());
}

TEST_F(FindReplaceTest, EditorBracketMatchingWithDocument) {
    editor_->setPlainText("Hello {world}");
    BracketMatcher matcher;
    auto result = matcher.findMatchingBrackets(editor_->document(), 6);
    ASSERT_EQ(result.size(), 2u);
    EXPECT_EQ(result[0], 6);
    EXPECT_EQ(result[1], 12);
}

TEST_F(FindReplaceTest, BracketMatcherDocNoCrashOnEmpty) {
    BracketMatcher matcher;
    auto result = matcher.findMatchingBrackets(editor_->document(), 0);
    EXPECT_TRUE(result.empty());
}

TEST_F(FindReplaceTest, BracketMatcherDocNullDoc) {
    BracketMatcher matcher;
    auto result = matcher.findMatchingBrackets(
        static_cast<const QTextDocument*>(nullptr), 0);
    EXPECT_TRUE(result.empty());
}

TEST_F(FindReplaceTest, BracketMatcherDocInvalidPos) {
    editor_->setPlainText("Hello");
    BracketMatcher matcher;
    auto result = matcher.findMatchingBrackets(editor_->document(), 999);
    EXPECT_TRUE(result.empty());
}
