#pragma once

#include "editor/BracketMatcher.h"
#include "snippets/SnippetSession.h"
#include "snippets/Snippets.h"
#include "theme/Theme.h"

#include <QPlainTextEdit>
#include <QTextEdit>

namespace lighttex::editor {

class LineNumberArea;

class EditorWidget : public QPlainTextEdit {
  Q_OBJECT

public:
  explicit EditorWidget(QWidget *parent = nullptr);

  void setTheme(const lighttex::theme::Theme &theme);
  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth() const;

  // Search highlight support
  void setSearchHighlights(const QList<QTextEdit::ExtraSelection> &highlights);
  void clearSearchHighlights();

  // Snippet expansion (public so CompletionWidget can call via handler)
  bool tryExpandSnippet();

signals:
  void cursorPositionUpdated(int line, int col);
  void snippetExpandedInBraces(int bracePos);

protected:
  void resizeEvent(QResizeEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;

private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &rect, int dy);
  void onCursorMoved();

private:
  void updateExtraSelections();
  void handleEnterKey();
  void handleCloseBraceAfterBegin();
  static QString getLineIndent(const QTextBlock &block);

  void selectSnippetTabStop();
  void cancelSnippetSession();
  static std::string adjustBodyIndent(const std::string &body,
                                      const QString &baseIndent);

  LineNumberArea *lineNumberArea_;
  BracketMatcher bracketMatcher_;
  lighttex::snippets::SnippetManager snippetManager_;
  lighttex::snippets::SnippetSession snippetSession_;
  bool snippetNavigating_ = false;
  QColor lineHighlightColor_;
  QColor gutterBg_;
  QColor gutterFg_;
  QColor bracketMatchBg_ = QColor("#5a5a5a");
  QColor bracketMatchFg_ = QColor("#ffd700");
  QList<QTextEdit::ExtraSelection> searchHighlights_;
};

} // namespace lighttex::editor
