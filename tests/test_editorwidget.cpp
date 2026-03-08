#include "editor/EditorWidget.h"

#include <QApplication>
#include <QTest>
#include <gtest/gtest.h>

using namespace lighttex::editor;

class EditorWidgetTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QApplication::instance()) {
            static int argc = 1;
            static char arg0[] = "test";
            static char* argv[] = {arg0, nullptr};
            static QApplication app(argc, argv);
        }
    }
};

TEST_F(EditorWidgetTest, Creation) {
    EditorWidget editor;
    EXPECT_TRUE(editor.toPlainText().isEmpty());
    EXPECT_GT(editor.lineNumberAreaWidth(), 0);
}

TEST_F(EditorWidgetTest, SetText) {
    EditorWidget editor;
    editor.setPlainText("Hello World");
    EXPECT_EQ(editor.toPlainText(), "Hello World");
}

TEST_F(EditorWidgetTest, CursorSignal) {
    EditorWidget editor;
    editor.setPlainText("Hello\nWorld");

    int lastLine = 0, lastCol = 0;
    QObject::connect(&editor, &EditorWidget::cursorPositionUpdated,
                     [&](int line, int col) {
        lastLine = line;
        lastCol = col;
    });

    // Move cursor
    QTextCursor cursor = editor.textCursor();
    cursor.movePosition(QTextCursor::End);
    editor.setTextCursor(cursor);

    EXPECT_EQ(lastLine, 2);
    EXPECT_EQ(lastCol, 6); // "World" has 5 chars, cursor after last = col 6 (1-based)
}

TEST_F(EditorWidgetTest, ThemeApplication) {
    EditorWidget editor;
    lighttex::theme::Theme theme;
    theme.colors.background = "#000000";
    theme.colors.foreground = "#ffffff";
    theme.colors.selection = "#333333";
    theme.colors.lineHighlight = "#111111";
    theme.colors.gutter = "#000000";
    theme.colors.gutterForeground = "#666666";

    // Should not crash
    editor.setTheme(theme);
}

TEST_F(EditorWidgetTest, LightThemeApplication) {
    EditorWidget editor;
    lighttex::theme::Theme light = lighttex::theme::lightTheme();

    // Should not crash, palette should update
    editor.setTheme(light);
    EXPECT_EQ(editor.palette().color(QPalette::Base), QColor("#ffffff"));
    EXPECT_EQ(editor.palette().color(QPalette::Text), QColor("#333333"));
}

TEST_F(EditorWidgetTest, DarkThemeApplication) {
    EditorWidget editor;
    lighttex::theme::Theme dark = lighttex::theme::darkTheme();

    editor.setTheme(dark);
    EXPECT_EQ(editor.palette().color(QPalette::Base), QColor("#1e1e1e"));
    EXPECT_EQ(editor.palette().color(QPalette::Text), QColor("#d4d4d4"));
}

TEST_F(EditorWidgetTest, ThemeSwitching) {
    EditorWidget editor;
    lighttex::theme::Theme dark = lighttex::theme::darkTheme();
    lighttex::theme::Theme light = lighttex::theme::lightTheme();

    editor.setTheme(dark);
    EXPECT_EQ(editor.palette().color(QPalette::Base), QColor("#1e1e1e"));

    editor.setTheme(light);
    EXPECT_EQ(editor.palette().color(QPalette::Base), QColor("#ffffff"));

    // Switch back
    editor.setTheme(dark);
    EXPECT_EQ(editor.palette().color(QPalette::Base), QColor("#1e1e1e"));
}
