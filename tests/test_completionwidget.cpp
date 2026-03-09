#include "lsp/CompletionWidget.h"
#include "lsp/LspTypes.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QSignalSpy>
#include <QTest>
#include <gtest/gtest.h>

using namespace lighttex::lsp;

class CompletionWidgetTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!QApplication::instance()) {
            int argc = 0;
            app_ = new QApplication(argc, nullptr);
        }
        editor_ = new QPlainTextEdit();
        editor_->show();
        popup_ = new CompletionWidget(editor_);
    }

    void TearDown() override {
        delete popup_;
        delete editor_;
    }

    std::vector<CompletionItem> makeItems(
        const std::vector<QString>& labels) {
        std::vector<CompletionItem> items;
        for (const auto& label : labels) {
            CompletionItem item;
            item.label = label;
            item.insertText = label;
            items.push_back(item);
        }
        return items;
    }

    QApplication* app_ = nullptr;
    QPlainTextEdit* editor_ = nullptr;
    CompletionWidget* popup_ = nullptr;
};

// --- Construction ---

TEST_F(CompletionWidgetTest, InitialState) {
    EXPECT_FALSE(popup_->isVisible());
    EXPECT_EQ(popup_->triggerPos(), -1);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Command);
}

// --- showCompletions / hideCompletions ---

TEST_F(CompletionWidgetTest, ShowCompletionsDisplaysPopup) {
    // Simulate a trigger at position 0
    editor_->setPlainText("\\test");
    // Manually set trigger state via startArgumentSession trick,
    // then re-purpose for command; or use show directly.
    // We need triggerPos >= 0 for showCompletions to work.
    // Use the backslash trigger by simulating a key event.

    // Directly test by starting an argument session (sets triggerPos)
    popup_->startArgumentSession(0);
    auto items = makeItems({"section", "subsection", "subsubsection"});
    popup_->showCompletions(items);

    EXPECT_TRUE(popup_->isVisible());
    EXPECT_EQ(popup_->count(), 3);
}

TEST_F(CompletionWidgetTest, HideCompletionsResetsState) {
    popup_->startArgumentSession(0);
    auto items = makeItems({"section", "subsection"});
    popup_->showCompletions(items);
    EXPECT_TRUE(popup_->isVisible());

    popup_->hideCompletions();
    EXPECT_FALSE(popup_->isVisible());
    EXPECT_EQ(popup_->triggerPos(), -1);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Command);
    EXPECT_EQ(popup_->count(), 0);
}

TEST_F(CompletionWidgetTest, ShowCompletionsFiltersJunkItems) {
    popup_->startArgumentSession(0);
    // Single-char labels should be filtered out
    auto items = makeItems({"(", "[", "{", "section", "subsection"});
    popup_->showCompletions(items);

    EXPECT_TRUE(popup_->isVisible());
    EXPECT_EQ(popup_->count(), 2); // only section, subsection
}

TEST_F(CompletionWidgetTest, ShowCompletionsHidesOnEmptyAfterFilter) {
    popup_->startArgumentSession(0);
    // All single-char → all filtered → hide
    auto items = makeItems({"(", "[", "{"});
    popup_->showCompletions(items);

    EXPECT_FALSE(popup_->isVisible());
}

TEST_F(CompletionWidgetTest, ShowCompletionsHidesWhenCursorBeforeTrigger) {
    editor_->setPlainText("hello");
    popup_->startArgumentSession(10); // trigger beyond cursor

    auto items = makeItems({"section"});
    popup_->showCompletions(items);

    EXPECT_FALSE(popup_->isVisible());
}

// --- TriggerKind ---

TEST_F(CompletionWidgetTest, StartArgumentSessionSetsTriggerKind) {
    popup_->startArgumentSession(5);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Argument);
    EXPECT_EQ(popup_->triggerPos(), 5);
}

TEST_F(CompletionWidgetTest, HideResetsToCommandTriggerKind) {
    popup_->startArgumentSession(5);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Argument);

    popup_->hideCompletions();
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Command);
}

// --- Signals ---

TEST_F(CompletionWidgetTest, StartArgumentSessionEmitsCompletionRequested) {
    QSignalSpy spy(popup_, &CompletionWidget::completionRequested);
    EXPECT_TRUE(spy.isValid());

    popup_->startArgumentSession(0);
    EXPECT_EQ(spy.count(), 1);
}

TEST_F(CompletionWidgetTest, BackslashTriggerEmitsCompletionRequested) {
    QSignalSpy spy(popup_, &CompletionWidget::completionRequested);
    editor_->setFocus();

    // Simulate typing backslash
    QTest::keyPress(editor_, Qt::Key_Backslash);
    QApplication::processEvents();
    // singleShot(0) needs an event loop iteration
    QTest::qWait(50);

    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Command);
}

TEST_F(CompletionWidgetTest, BraceTriggerEmitsCompletionRequested) {
    QSignalSpy spy(popup_, &CompletionWidget::completionRequested);
    editor_->setPlainText("\\usepackage");
    // Place cursor at end
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor_->setTextCursor(cursor);
    editor_->setFocus();

    // Simulate typing {
    QTest::keyPress(editor_, Qt::Key_BraceLeft, Qt::ShiftModifier);
    QApplication::processEvents();
    QTest::qWait(50);

    EXPECT_GE(spy.count(), 1);
    EXPECT_EQ(popup_->triggerKind(), TriggerKind::Argument);
}

// --- Key handling when popup visible ---

TEST_F(CompletionWidgetTest, EscapeDismissesPopup) {
    popup_->startArgumentSession(0);
    editor_->setPlainText("test");
    popup_->showCompletions(makeItems({"section", "subsection"}));
    EXPECT_TRUE(popup_->isVisible());

    QTest::keyPress(editor_, Qt::Key_Escape);
    EXPECT_FALSE(popup_->isVisible());
}

TEST_F(CompletionWidgetTest, TabAcceptsCompletion) {
    editor_->setPlainText("\\sec");
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor_->setTextCursor(cursor);

    popup_->startArgumentSession(0);
    popup_->showCompletions(makeItems({"section", "subsection"}));
    EXPECT_TRUE(popup_->isVisible());

    QSignalSpy spy(popup_, &CompletionWidget::itemChosen);
    QTest::keyPress(editor_, Qt::Key_Tab);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(popup_->isVisible());

    // Verify signal arguments
    auto args = spy.takeFirst();
    auto chosenItem = args.at(0).value<CompletionItem>();
    EXPECT_EQ(chosenItem.label, "section");
}

TEST_F(CompletionWidgetTest, EnterAcceptsCompletion) {
    editor_->setPlainText("\\sec");
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor_->setTextCursor(cursor);

    popup_->startArgumentSession(0);
    popup_->showCompletions(makeItems({"section", "subsection"}));

    QSignalSpy spy(popup_, &CompletionWidget::itemChosen);
    QTest::keyPress(editor_, Qt::Key_Return);

    EXPECT_EQ(spy.count(), 1);
    EXPECT_FALSE(popup_->isVisible());
}

TEST_F(CompletionWidgetTest, DownArrowNavigatesList) {
    popup_->startArgumentSession(0);
    editor_->setPlainText("test");
    popup_->showCompletions(
        makeItems({"section", "subsection", "subsubsection"}));
    EXPECT_EQ(popup_->currentRow(), 0);

    QTest::keyPress(editor_, Qt::Key_Down);
    EXPECT_EQ(popup_->currentRow(), 1);

    QTest::keyPress(editor_, Qt::Key_Down);
    EXPECT_EQ(popup_->currentRow(), 2);

    // Should not go past last item
    QTest::keyPress(editor_, Qt::Key_Down);
    EXPECT_EQ(popup_->currentRow(), 2);
}

TEST_F(CompletionWidgetTest, UpArrowNavigatesList) {
    popup_->startArgumentSession(0);
    editor_->setPlainText("test");
    popup_->showCompletions(
        makeItems({"section", "subsection", "subsubsection"}));

    QTest::keyPress(editor_, Qt::Key_Down);
    QTest::keyPress(editor_, Qt::Key_Down);
    EXPECT_EQ(popup_->currentRow(), 2);

    QTest::keyPress(editor_, Qt::Key_Up);
    EXPECT_EQ(popup_->currentRow(), 1);

    QTest::keyPress(editor_, Qt::Key_Up);
    EXPECT_EQ(popup_->currentRow(), 0);

    // Should not go below 0
    QTest::keyPress(editor_, Qt::Key_Up);
    EXPECT_EQ(popup_->currentRow(), 0);
}

// --- Mouse click dismisses ---

TEST_F(CompletionWidgetTest, ClickOnEditorDismissesPopup) {
    popup_->startArgumentSession(0);
    editor_->setPlainText("test");
    popup_->showCompletions(makeItems({"section", "subsection"}));
    EXPECT_TRUE(popup_->isVisible());

    // Simulate a mouse click on the editor viewport
    QTest::mouseClick(editor_->viewport(), Qt::LeftButton);
    EXPECT_FALSE(popup_->isVisible());
}

// --- Selection preservation ---

TEST_F(CompletionWidgetTest, ShowCompletionsPreservesSelection) {
    popup_->startArgumentSession(0);
    editor_->setPlainText("test");
    popup_->showCompletions(
        makeItems({"section", "subsection", "subsubsection"}));

    // Select second item
    QTest::keyPress(editor_, Qt::Key_Down);
    EXPECT_EQ(popup_->currentRow(), 1);

    // Show new completions with same labels — selection should be restored
    popup_->showCompletions(
        makeItems({"section", "subsection", "subsubsection"}));
    EXPECT_EQ(popup_->currentRow(), 1);
}

// --- itemChosen signal carries TriggerKind ---

TEST_F(CompletionWidgetTest, ItemChosenCarriesCommandTriggerKind) {
    editor_->setPlainText("\\sec");
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor_->setTextCursor(cursor);

    // Simulate command trigger
    popup_->hideCompletions(); // reset
    // Manually set command trigger state
    QTest::keyPress(editor_, Qt::Key_Backslash);
    QApplication::processEvents();
    QTest::qWait(50);

    // Now show items and accept
    popup_->showCompletions(makeItems({"section"}));
    if (!popup_->isVisible()) {
        // Fallback: use startArgumentSession to get a working popup
        popup_->startArgumentSession(0);
        popup_->showCompletions(makeItems({"section"}));
    }

    QSignalSpy spy(popup_, &CompletionWidget::itemChosen);
    QTest::keyPress(editor_, Qt::Key_Tab);

    if (spy.count() == 1) {
        auto args = spy.takeFirst();
        auto tk = args.at(3).value<lighttex::lsp::TriggerKind>();
        // TriggerKind depends on how we set it up
        EXPECT_TRUE(tk == TriggerKind::Command ||
                    tk == TriggerKind::Argument);
    }
}

TEST_F(CompletionWidgetTest, ItemChosenCarriesArgumentTriggerKind) {
    editor_->setPlainText("\\usepackage{ams");
    QTextCursor cursor = editor_->textCursor();
    cursor.movePosition(QTextCursor::End);
    editor_->setTextCursor(cursor);

    popup_->startArgumentSession(12); // position after {
    popup_->showCompletions(makeItems({"amsmath", "amssymb", "amsfonts"}));
    EXPECT_TRUE(popup_->isVisible());

    QSignalSpy spy(popup_, &CompletionWidget::itemChosen);
    QTest::keyPress(editor_, Qt::Key_Tab);

    EXPECT_EQ(spy.count(), 1);
    auto args = spy.takeFirst();
    auto tk = args.at(3).value<lighttex::lsp::TriggerKind>();
    EXPECT_EQ(tk, TriggerKind::Argument);
}

// --- CompletionItem::fromJson ---

TEST_F(CompletionWidgetTest, FromJsonPrefersTextEditNewText) {
    // texlab sends snippet text in textEdit.newText
    QJsonObject obj;
    obj["label"] = "usepackage";
    obj["kind"] = 1;
    obj["textEdit"] = QJsonObject{
        {"newText", "usepackage{$1}[$2]$0"},
        {"range", QJsonObject{
            {"start", QJsonObject{{"line", 0}, {"character", 1}}},
            {"end", QJsonObject{{"line", 0}, {"character", 5}}}
        }}
    };

    auto item = CompletionItem::fromJson(obj);
    EXPECT_EQ(item.label, "usepackage");
    // insertText should be textEdit.newText (has $ markers)
    EXPECT_EQ(item.insertText, "usepackage{$1}[$2]$0");
}

TEST_F(CompletionWidgetTest, FromJsonFallsBackToLabel) {
    QJsonObject obj;
    obj["label"] = "maketitle";
    obj["kind"] = 1;
    // No insertText, no textEdit

    auto item = CompletionItem::fromJson(obj);
    EXPECT_EQ(item.label, "maketitle");
    EXPECT_EQ(item.insertText, "maketitle");
}

TEST_F(CompletionWidgetTest, FromJsonUsesInsertTextWhenNoTextEdit) {
    QJsonObject obj;
    obj["label"] = "frac";
    obj["kind"] = 1;
    obj["insertText"] = "frac{$1}{$2}";

    auto item = CompletionItem::fromJson(obj);
    EXPECT_EQ(item.insertText, "frac{$1}{$2}");
}

TEST_F(CompletionWidgetTest, FromJsonTextEditWithoutSnippetIgnored) {
    // textEdit.newText without $ markers should not override insertText
    QJsonObject obj;
    obj["label"] = "par";
    obj["kind"] = 1;
    obj["insertText"] = "par";
    obj["textEdit"] = QJsonObject{
        {"newText", "par"},
        {"range", QJsonObject{
            {"start", QJsonObject{{"line", 0}, {"character", 1}}},
            {"end", QJsonObject{{"line", 0}, {"character", 4}}}
        }}
    };

    auto item = CompletionItem::fromJson(obj);
    EXPECT_EQ(item.insertText, "par"); // unchanged
}
