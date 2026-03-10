#include "editor/EditorWidget.h"

#include <QApplication>
#include <QKeyEvent>
#include <QTest>
#include <QTextBlock>
#include <gtest/gtest.h>

namespace {

// Ensure a QApplication exists for the whole test suite
struct AppGuard {
  AppGuard() {
    if (!QApplication::instance()) {
      static int argc = 1;
      static char arg0[] = "test";
      static char *argv[] = {arg0, nullptr};
      app = new QApplication(argc, argv);
    }
  }
  QApplication *app = nullptr;
};
static AppGuard guard;

class AutoIndentTest : public ::testing::Test {
protected:
  void SetUp() override { editor = new lighttex::editor::EditorWidget(); }
  void TearDown() override { delete editor; }

  // Helper: set editor text and position cursor at end of given line (0-based)
  void setTextAndCursor(const QString &text, int line, int col = -1) {
    editor->setPlainText(text);
    QTextCursor cursor = editor->textCursor();
    QTextBlock block = editor->document()->findBlockByNumber(line);
    cursor.setPosition(block.position());
    if (col < 0)
      cursor.movePosition(QTextCursor::EndOfBlock);
    else
      cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor, col);
    editor->setTextCursor(cursor);
  }

  void pressEnter() {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Return, Qt::NoModifier);
    QApplication::sendEvent(editor, &press);
  }

  void typeChar(QChar ch) {
    QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, QString(ch));
    QApplication::sendEvent(editor, &press);
  }

  lighttex::editor::EditorWidget *editor = nullptr;
};

// --- Auto-indentation tests ---

TEST_F(AutoIndentTest, EnterCopiesSpaceIndent) {
  setTextAndCursor("    hello", 0);
  pressEnter();

  QTextBlock newLine = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(newLine.text().toStdString(), "    ");
}

TEST_F(AutoIndentTest, EnterCopiesTabIndent) {
  setTextAndCursor("\thello", 0);
  pressEnter();

  QTextBlock newLine = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(newLine.text().toStdString(), "\t");
}

TEST_F(AutoIndentTest, EnterOnEmptyLineNoIndent) {
  setTextAndCursor("", 0);
  pressEnter();

  QTextBlock newLine = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(newLine.text().toStdString(), "");
}

TEST_F(AutoIndentTest, EnterInMiddleOfLine) {
  setTextAndCursor("  hello world", 0, 7); // after "o" in "hello"
  pressEnter();

  QTextBlock line1 = editor->document()->findBlockByNumber(0);
  QTextBlock line2 = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(line1.text().toStdString(), "  hello");
  EXPECT_EQ(line2.text().toStdString(), "   world");
}

TEST_F(AutoIndentTest, EnterAfterBeginAddsExtraIndent) {
  setTextAndCursor("  \\begin{document}", 0);
  pressEnter();

  QTextBlock newLine = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(newLine.text().toStdString(), "  \t");
}

TEST_F(AutoIndentTest, NestedBeginDoubleIndent) {
  setTextAndCursor("\t\\begin{itemize}", 0);
  pressEnter();

  QTextBlock newLine = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(newLine.text().toStdString(), "\t\t");
}

TEST_F(AutoIndentTest, EnterBetweenBeginAndEnd) {
  setTextAndCursor("\\begin{doc}\n\\end{doc}", 0);
  pressEnter();

  // Should now be 3 lines: \begin{doc}, \t (cursor), \end{doc}
  EXPECT_EQ(editor->document()->blockCount(), 3);
  QTextBlock line1 = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(line1.text().toStdString(), "\t");

  // Cursor should be on line 1 (the indented blank line)
  QTextCursor cursor = editor->textCursor();
  EXPECT_EQ(cursor.blockNumber(), 1);
}

// --- \begin/\end auto-pairing tests ---

TEST_F(AutoIndentTest, CloseBraceAutoInsertsEnd) {
  setTextAndCursor("\\begin{document", 0);
  typeChar(QChar('}'));

  // Should now have 3 lines
  EXPECT_EQ(editor->document()->blockCount(), 3);
  QTextBlock line0 = editor->document()->findBlockByNumber(0);
  QTextBlock line2 = editor->document()->findBlockByNumber(2);
  EXPECT_EQ(line0.text().toStdString(), "\\begin{document}");
  EXPECT_EQ(line2.text().toStdString(), "\\end{document}");

  // Cursor on indented blank line
  QTextCursor cursor = editor->textCursor();
  EXPECT_EQ(cursor.blockNumber(), 1);
}

TEST_F(AutoIndentTest, CloseBraceWithIndent) {
  setTextAndCursor("    \\begin{itemize", 0);
  typeChar(QChar('}'));

  EXPECT_EQ(editor->document()->blockCount(), 3);
  QTextBlock line0 = editor->document()->findBlockByNumber(0);
  QTextBlock line1 = editor->document()->findBlockByNumber(1);
  QTextBlock line2 = editor->document()->findBlockByNumber(2);
  EXPECT_EQ(line0.text().toStdString(), "    \\begin{itemize}");
  EXPECT_EQ(line1.text().toStdString(), "    \t");
  EXPECT_EQ(line2.text().toStdString(), "    \\end{itemize}");
}

TEST_F(AutoIndentTest, NoDuplicateEndInsertion) {
  setTextAndCursor("\\begin{doc\n\\end{doc}", 0);
  typeChar(QChar('}'));

  // Should still have 2 lines — no extra \end inserted
  EXPECT_EQ(editor->document()->blockCount(), 2);
  QTextBlock line0 = editor->document()->findBlockByNumber(0);
  QTextBlock line1 = editor->document()->findBlockByNumber(1);
  EXPECT_EQ(line0.text().toStdString(), "\\begin{doc}");
  EXPECT_EQ(line1.text().toStdString(), "\\end{doc}");
}

TEST_F(AutoIndentTest, CloseBraceNonBeginNoTrigger) {
  setTextAndCursor("hello{world", 0);
  typeChar(QChar('}'));

  // Should remain 1 line — no \end inserted
  EXPECT_EQ(editor->document()->blockCount(), 1);
  EXPECT_EQ(editor->document()->findBlockByNumber(0).text().toStdString(),
            "hello{world}");
}

TEST_F(AutoIndentTest, UndoRevertsEntireAutoEnd) {
  setTextAndCursor("\\begin{document", 0);
  typeChar(QChar('}'));

  // Verify auto-pair happened
  EXPECT_EQ(editor->document()->blockCount(), 3);

  // Single undo should revert the auto-pair insertion (the \end + newlines)
  editor->undo();

  // After undo, the '}' insertion and auto-pair should be reverted
  // The original text was "\\begin{document" before the '}' typed,
  // so undo reverses the entire edit block
  QString text = editor->toPlainText();
  EXPECT_TRUE(text.indexOf("\\end{document}") == -1)
      << "\\end{document} should be removed after undo";
}

} // namespace
