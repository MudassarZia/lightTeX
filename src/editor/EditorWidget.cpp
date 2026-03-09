#include "editor/EditorWidget.h"
#include "editor/LineNumberArea.h"

#include <QFont>
#include <QPainter>
#include <QTextBlock>

namespace lighttex::editor {

EditorWidget::EditorWidget(QWidget *parent) : QPlainTextEdit(parent) {
  lineNumberArea_ = new LineNumberArea(this);

  // Set monospace font
  QFont font("JetBrains Mono", 12);
  font.setStyleHint(QFont::Monospace);
  font.setFixedPitch(true);
  setFont(font);
  setTabStopDistance(fontMetrics().horizontalAdvance(' ') * 4);

  // Default colors (dark theme)
  lineHighlightColor_ = QColor("#2a2d2e");
  gutterBg_ = QColor("#1e1e1e");
  gutterFg_ = QColor("#858585");

  connect(this, &QPlainTextEdit::blockCountChanged, this,
          &EditorWidget::updateLineNumberAreaWidth);
  connect(this, &QPlainTextEdit::updateRequest, this,
          &EditorWidget::updateLineNumberArea);
  connect(this, &QPlainTextEdit::cursorPositionChanged, this,
          &EditorWidget::highlightCurrentLine);
  connect(this, &QPlainTextEdit::cursorPositionChanged, this,
          &EditorWidget::onCursorMoved);

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();
}

void EditorWidget::setTheme(const lighttex::theme::Theme &theme) {
  lineHighlightColor_ =
      QColor(QString::fromStdString(theme.colors.lineHighlight));
  gutterBg_ = QColor(QString::fromStdString(theme.colors.gutter));
  gutterFg_ = QColor(QString::fromStdString(theme.colors.gutterForeground));

  // Bracket match colors adapt to theme
  if (theme.kind == lighttex::theme::ThemeKind::Light) {
    bracketMatchBg_ = QColor("#d0d0d0");
    bracketMatchFg_ = QColor("#0431fa");
  } else {
    bracketMatchBg_ = QColor("#5a5a5a");
    bracketMatchFg_ = QColor("#ffd700");
  }

  QPalette p = palette();
  p.setColor(QPalette::Base,
             QColor(QString::fromStdString(theme.colors.background)));
  p.setColor(QPalette::Text,
             QColor(QString::fromStdString(theme.colors.foreground)));
  p.setColor(QPalette::Highlight,
             QColor(QString::fromStdString(theme.colors.selection)));
  setPalette(p);

  highlightCurrentLine();
  lineNumberArea_->update();
}

int EditorWidget::lineNumberAreaWidth() const {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }
  digits = qMax(digits, 3); // Minimum 3 digits width
  int space = 10 + fontMetrics().horizontalAdvance(QLatin1Char('9')) * digits;
  return space;
}

void EditorWidget::updateLineNumberAreaWidth(int /*newBlockCount*/) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void EditorWidget::updateLineNumberArea(const QRect &rect, int dy) {
  if (dy) {
    lineNumberArea_->scroll(0, dy);
  } else {
    lineNumberArea_->update(0, rect.y(), lineNumberArea_->width(),
                            rect.height());
  }
  if (rect.contains(viewport()->rect())) {
    updateLineNumberAreaWidth(0);
  }
}

void EditorWidget::resizeEvent(QResizeEvent *event) {
  QPlainTextEdit::resizeEvent(event);
  QRect cr = contentsRect();
  lineNumberArea_->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

void EditorWidget::highlightCurrentLine() { updateExtraSelections(); }

void EditorWidget::updateExtraSelections() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;
    selection.format.setBackground(lineHighlightColor_);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);
  }

  // Bracket matching using QTextDocument (no full text copy)
  auto brackets =
      bracketMatcher_.findMatchingBrackets(document(), textCursor().position());
  for (int pos : brackets) {
    QTextEdit::ExtraSelection sel;
    sel.format.setBackground(bracketMatchBg_);
    sel.format.setForeground(bracketMatchFg_);
    sel.cursor = textCursor();
    sel.cursor.setPosition(pos);
    sel.cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor);
    extraSelections.append(sel);
  }

  // Merge search highlights
  extraSelections.append(searchHighlights_);

  setExtraSelections(extraSelections);
}

void EditorWidget::setSearchHighlights(
    const QList<QTextEdit::ExtraSelection> &highlights) {
  searchHighlights_ = highlights;
  updateExtraSelections();
}

void EditorWidget::clearSearchHighlights() {
  searchHighlights_.clear();
  updateExtraSelections();
}

void EditorWidget::lineNumberAreaPaintEvent(QPaintEvent *event) {
  QPainter painter(lineNumberArea_);
  painter.fillRect(event->rect(), gutterBg_);

  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top =
      qRound(blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + qRound(blockBoundingRect(block).height());

  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(gutterFg_);
      painter.drawText(0, top, lineNumberArea_->width() - 5,
                       fontMetrics().height(), Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + qRound(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

void EditorWidget::keyPressEvent(QKeyEvent *event) {
  QPlainTextEdit::keyPressEvent(event);
}

void EditorWidget::onCursorMoved() {
  QTextCursor cursor = textCursor();
  int line = cursor.blockNumber() + 1;
  int col = cursor.columnNumber() + 1;
  emit cursorPositionUpdated(line, col);
}

} // namespace lighttex::editor
