#include "editor/EditorWidget.h"
#include "snippets/SnippetSession.h"
#include "snippets/Snippets.h"

#include <QApplication>
#include <QKeyEvent>
#include <QTest>
#include <QTextBlock>
#include <gtest/gtest.h>

namespace {

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

class SnippetExpansionTest : public ::testing::Test {
protected:
  void SetUp() override { editor = new lighttex::editor::EditorWidget(); }
  void TearDown() override { delete editor; }

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

  void pressTab() {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Tab, Qt::NoModifier);
    QApplication::sendEvent(editor, &press);
  }

  void pressShiftTab() {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Backtab, Qt::ShiftModifier);
    QApplication::sendEvent(editor, &press);
  }

  void pressEscape() {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
    QApplication::sendEvent(editor, &press);
  }

  void pressRight() {
    QKeyEvent press(QEvent::KeyPress, Qt::Key_Right, Qt::NoModifier);
    QApplication::sendEvent(editor, &press);
  }

  void typeChar(QChar ch) {
    QKeyEvent press(QEvent::KeyPress, 0, Qt::NoModifier, QString(ch));
    QApplication::sendEvent(editor, &press);
  }

  lighttex::editor::EditorWidget *editor = nullptr;
};

TEST_F(SnippetExpansionTest, TabExpandsSimpleSnippet) {
  setTextAndCursor("\\frac", 0);
  pressTab();

  QString text = editor->toPlainText();
  EXPECT_EQ(text.toStdString(), "\\frac{num}{den}");

  // "num" should be selected (first tabstop)
  QTextCursor cursor = editor->textCursor();
  EXPECT_TRUE(cursor.hasSelection());
  EXPECT_EQ(cursor.selectedText().toStdString(), "num");
}

TEST_F(SnippetExpansionTest, TabExpandsBeginSnippet) {
  setTextAndCursor("\\begin", 0);
  pressTab();

  QString text = editor->toPlainText();
  EXPECT_TRUE(text.contains("\\begin{environment}"));
  EXPECT_TRUE(text.contains("\\end{environment}"));

  // "environment" should be selected
  QTextCursor cursor = editor->textCursor();
  EXPECT_TRUE(cursor.hasSelection());
  EXPECT_EQ(cursor.selectedText().toStdString(), "environment");
}

TEST_F(SnippetExpansionTest, TabNoTriggerInsertsTab) {
  setTextAndCursor("hello", 0);
  pressTab();

  QString text = editor->toPlainText();
  EXPECT_TRUE(text.contains("\t") || text.contains("    "));
}

TEST_F(SnippetExpansionTest, TabUnknownTriggerInsertsTab) {
  setTextAndCursor("\\xyz", 0);
  pressTab();

  QString text = editor->toPlainText();
  // \xyz should still be there, plus a tab
  EXPECT_TRUE(text.startsWith("\\xyz"));
  EXPECT_GT(text.length(), 4); // has tab or spaces appended
}

TEST_F(SnippetExpansionTest, TabCyclesTabstops) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand, at $1 "num"

  QTextCursor cursor = editor->textCursor();
  EXPECT_EQ(cursor.selectedText().toStdString(), "num");

  pressTab(); // move to $2 "den"
  cursor = editor->textCursor();
  EXPECT_EQ(cursor.selectedText().toStdString(), "den");

  pressTab(); // move to $0 (final), session ends
  EXPECT_FALSE(editor->toPlainText().isEmpty());
}

TEST_F(SnippetExpansionTest, ShiftTabReversesCycle) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand, at $1 "num"
  pressTab(); // move to $2 "den"

  QTextCursor cursor = editor->textCursor();
  EXPECT_EQ(cursor.selectedText().toStdString(), "den");

  pressShiftTab(); // back to $1 "num"
  cursor = editor->textCursor();
  EXPECT_EQ(cursor.selectedText().toStdString(), "num");
}

TEST_F(SnippetExpansionTest, ArrowKeyCancelsSession) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand

  pressRight(); // should cancel session

  // Tab should now insert a tab char, not cycle
  pressTab();
  QString text = editor->toPlainText();
  // Should have a tab somewhere after pressing tab
  EXPECT_NE(text.toStdString(), "\\frac{num}{den}");
}

TEST_F(SnippetExpansionTest, EscapeCancelsSession) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand, "num" selected

  pressEscape(); // cancel session

  // Session cancelled: Tab should NOT cycle to next tabstop "den".
  // Instead it replaces the still-selected "num" with a tab char.
  pressTab();
  QString text = editor->toPlainText();
  EXPECT_FALSE(text.contains("num"))
      << "Tab should have replaced selected text, not cycled tabstop";
}

TEST_F(SnippetExpansionTest, TypingCancelsSession) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand

  typeChar(QChar('x')); // should cancel session

  // Verify session cancelled: Tab now inserts tab
  int lenBefore = editor->toPlainText().length();
  pressTab();
  EXPECT_GT(editor->toPlainText().length(), lenBefore);
}

TEST_F(SnippetExpansionTest, UndoReversesExpansion) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand

  EXPECT_EQ(editor->toPlainText().toStdString(), "\\frac{num}{den}");

  editor->undo();

  EXPECT_EQ(editor->toPlainText().toStdString(), "\\frac");
}

TEST_F(SnippetExpansionTest, IndentedSnippetExpansion) {
  setTextAndCursor("    \\sec", 0);
  pressTab();

  QString text = editor->toPlainText();
  // \section{title}\n    $0
  // The continuation line should have the 4-space indent
  QTextBlock line1 = editor->document()->findBlockByNumber(1);
  EXPECT_TRUE(line1.text().startsWith("    "))
      << "Continuation line should have base indent, got: "
      << line1.text().toStdString();
}

TEST_F(SnippetExpansionTest, SessionInactiveAfterFinal) {
  setTextAndCursor("\\frac", 0);
  pressTab(); // expand, at $1
  pressTab(); // $2
  pressTab(); // $0, session should end

  // Another Tab should insert a tab character
  int lenBefore = editor->toPlainText().length();
  pressTab();
  EXPECT_GT(editor->toPlainText().length(), lenBefore);
}

TEST_F(SnippetExpansionTest, BeginSnippetEmitsBraceSignal) {
  int signalBracePos = -1;
  QObject::connect(editor,
                   &lighttex::editor::EditorWidget::snippetExpandedInBraces,
                   [&](int pos) { signalBracePos = pos; });

  setTextAndCursor("\\begin", 0);
  pressTab();

  // Signal should fire with position after '{'
  EXPECT_GT(signalBracePos, 0);
  // Character before the signaled position should be '{'
  QChar charBefore = editor->document()->characterAt(signalBracePos - 1);
  EXPECT_EQ(charBefore, QChar('{'));
}

TEST_F(SnippetExpansionTest, FracSnippetEmitsBraceSignal) {
  int signalBracePos = -1;
  QObject::connect(editor,
                   &lighttex::editor::EditorWidget::snippetExpandedInBraces,
                   [&](int pos) { signalBracePos = pos; });

  setTextAndCursor("\\frac", 0);
  pressTab();

  // \frac first tabstop is inside {}, so signal should fire
  EXPECT_GT(signalBracePos, 0);
}

TEST_F(SnippetExpansionTest, SecSnippetNoBraceSignalIfNotInBraces) {
  // \sec expands to \section{title}\n$0
  // First tabstop "title" IS inside {}, so signal should fire
  int signalBracePos = -1;
  QObject::connect(editor,
                   &lighttex::editor::EditorWidget::snippetExpandedInBraces,
                   [&](int pos) { signalBracePos = pos; });

  setTextAndCursor("\\sec", 0);
  pressTab();

  EXPECT_GT(signalBracePos, 0);
}

} // namespace
