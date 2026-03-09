#pragma once

#include "editor/BracketMatcher.h"
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

signals:
  void cursorPositionUpdated(int line, int col);

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

  LineNumberArea *lineNumberArea_;
  BracketMatcher bracketMatcher_;
  QColor lineHighlightColor_;
  QColor gutterBg_;
  QColor gutterFg_;
  QColor bracketMatchBg_ = QColor("#5a5a5a");
  QColor bracketMatchFg_ = QColor("#ffd700");
  QList<QTextEdit::ExtraSelection> searchHighlights_;
};

} // namespace lighttex::editor
