#include "editor/EditorWidget.h"
#include "editor/LineNumberArea.h"

#include <QFont>
#include <QPainter>
#include <QRegularExpression>
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
  if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
    cancelSnippetSession();
    handleEnterKey();
    return;
  }

  if (event->key() == Qt::Key_Tab && event->modifiers() == Qt::NoModifier) {
    if (snippetSession_.isActive()) {
      snippetSession_.nextTabStop();
      selectSnippetTabStop();
      return;
    }
    if (tryExpandSnippet()) {
      return;
    }
    // Insert tab character explicitly
    QTextCursor cursor = textCursor();
    cursor.insertText(QStringLiteral("\t"));
    setTextCursor(cursor);
    return;
  }

  if (event->key() == Qt::Key_Backtab ||
      (event->key() == Qt::Key_Tab &&
       event->modifiers() == Qt::ShiftModifier)) {
    if (snippetSession_.isActive()) {
      snippetSession_.prevTabStop();
      selectSnippetTabStop();
      return;
    }
  }

  if (event->key() == Qt::Key_Escape && snippetSession_.isActive()) {
    cancelSnippetSession();
    return;
  }

  if (snippetSession_.isActive()) {
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End) {
      cancelSnippetSession();
    } else if (!event->text().isEmpty() && event->key() != Qt::Key_Shift &&
               event->key() != Qt::Key_Control && event->key() != Qt::Key_Alt &&
               event->key() != Qt::Key_Meta) {
      cancelSnippetSession();
    }
  }

  QPlainTextEdit::keyPressEvent(event);
  if (event->text() == QLatin1String("}")) {
    handleCloseBraceAfterBegin();
  }
}

QString EditorWidget::getLineIndent(const QTextBlock &block) {
  const QString text = block.text();
  int i = 0;
  while (i < text.length() &&
         (text[i] == QLatin1Char(' ') || text[i] == QLatin1Char('\t')))
    ++i;
  return text.left(i);
}

void EditorWidget::handleEnterKey() {
  QTextCursor cursor = textCursor();
  const QTextBlock currentBlock = cursor.block();
  const QString indent = getLineIndent(currentBlock);

  // Check if current line (trimmed) ends with \begin{...}
  const QString lineText = currentBlock.text();
  static QRegularExpression beginRe(
      QStringLiteral("\\\\begin\\{([^}]+)\\}\\s*$"));
  const QString textBeforeCursor = lineText.left(cursor.positionInBlock());
  QRegularExpressionMatch match = beginRe.match(textBeforeCursor);

  cursor.beginEditBlock();

  if (match.hasMatch()) {
    // Check if next line already starts with matching \end{...}
    QTextBlock nextBlock = currentBlock.next();
    bool nextIsEnd = false;
    if (nextBlock.isValid()) {
      QString nextTrimmed = nextBlock.text().trimmed();
      nextIsEnd = nextTrimmed.startsWith(
          QStringLiteral("\\end{") + match.captured(1) + QStringLiteral("}"));
    }

    if (nextIsEnd) {
      // Insert indented blank line between \begin and \end
      // (\end already on next line, so only one newline needed)
      cursor.insertText(QStringLiteral("\n") + indent + QStringLiteral("\t"));
    } else {
      // Just add extra indent after \begin
      cursor.insertText(QStringLiteral("\n") + indent + QStringLiteral("\t"));
    }
  } else {
    // Normal case: copy current indent
    cursor.insertText(QStringLiteral("\n") + indent);
  }

  cursor.endEditBlock();
  setTextCursor(cursor);
  ensureCursorVisible();
}

void EditorWidget::handleCloseBraceAfterBegin() {
  QTextCursor cursor = textCursor();
  const QTextBlock currentBlock = cursor.block();
  const QString lineText = currentBlock.text();
  const QString textBeforeCursor = lineText.left(cursor.positionInBlock());

  static QRegularExpression beginRe(QStringLiteral("\\\\begin\\{([^}]+)\\}$"));
  QRegularExpressionMatch match = beginRe.match(textBeforeCursor);
  if (!match.hasMatch())
    return;

  const QString envName = match.captured(1);
  const QString indent = getLineIndent(currentBlock);

  // Check if next line already has \end{envName} (e.g. LSP already inserted it)
  QTextBlock nextBlock = currentBlock.next();
  if (nextBlock.isValid()) {
    QString nextTrimmed = nextBlock.text().trimmed();
    if (nextTrimmed.startsWith(QStringLiteral("\\end{") + envName +
                               QStringLiteral("}"))) {
      return;
    }
  }

  cursor.beginEditBlock();
  cursor.insertText(QStringLiteral("\n") + indent + QStringLiteral("\t") +
                    QStringLiteral("\n") + indent + QStringLiteral("\\end{") +
                    envName + QStringLiteral("}"));
  // Move cursor to the indented blank line between \begin and \end
  cursor.movePosition(QTextCursor::Up);
  cursor.movePosition(QTextCursor::EndOfBlock);
  cursor.endEditBlock();
  setTextCursor(cursor);
  ensureCursorVisible();
}

std::string EditorWidget::adjustBodyIndent(const std::string &body,
                                           const QString &baseIndent) {
  std::string indent = baseIndent.toStdString();
  std::string result;
  result.reserve(body.size());
  for (size_t i = 0; i < body.size(); ++i) {
    result += body[i];
    if (body[i] == '\n') {
      result += indent;
    }
  }
  return result;
}

bool EditorWidget::tryExpandSnippet() {
  QTextCursor cursor = textCursor();
  const QTextBlock block = cursor.block();
  const QString lineText = block.text();
  int col = cursor.positionInBlock();

  // Scan backwards for alpha chars
  int end = col;
  int pos = end - 1;
  while (pos >= 0 && lineText[pos].isLetter()) {
    --pos;
  }
  // Check for backslash before the alpha run
  if (pos < 0 || lineText[pos] != QLatin1Char('\\')) {
    return false;
  }

  QString candidate = lineText.mid(pos, end - pos);
  const lighttex::snippets::Snippet *snippet =
      snippetManager_.findByTrigger(candidate.toStdString());
  if (!snippet) {
    return false;
  }

  int triggerStartInBlock = pos;
  int triggerStartAbsolute = block.position() + triggerStartInBlock;

  QString baseIndent = getLineIndent(block);
  std::string adjustedBody = adjustBodyIndent(snippet->body, baseIndent);

  snippetNavigating_ = true;

  cursor.beginEditBlock();
  // Select trigger text
  cursor.setPosition(triggerStartAbsolute);
  cursor.setPosition(triggerStartAbsolute + candidate.length(),
                     QTextCursor::KeepAnchor);

  snippetSession_.start(adjustedBody, triggerStartAbsolute);
  cursor.insertText(QString::fromStdString(snippetSession_.expandedText()));
  cursor.endEditBlock();

  selectSnippetTabStop();
  snippetNavigating_ = false;

  // If first tabstop is inside {}, trigger LSP argument completion
  if (snippetSession_.isActive()) {
    int offset = snippetSession_.currentOffset();
    if (offset > 0 && document()->characterAt(offset - 1) == QLatin1Char('{')) {
      emit snippetExpandedInBraces(offset);
    }
  }

  return true;
}

void EditorWidget::selectSnippetTabStop() {
  if (!snippetSession_.isActive()) {
    // Place cursor at final position
    QTextCursor cursor = textCursor();
    cursor.setPosition(snippetSession_.currentOffset());
    setTextCursor(cursor);
    return;
  }
  snippetNavigating_ = true;
  QTextCursor cursor = textCursor();
  int offset = snippetSession_.currentOffset();
  int length = snippetSession_.currentLength();
  cursor.setPosition(offset);
  if (length > 0) {
    cursor.setPosition(offset + length, QTextCursor::KeepAnchor);
  }
  setTextCursor(cursor);
  snippetNavigating_ = false;
}

void EditorWidget::cancelSnippetSession() { snippetSession_.cancel(); }

void EditorWidget::onCursorMoved() {
  QTextCursor cursor = textCursor();
  int line = cursor.blockNumber() + 1;
  int col = cursor.columnNumber() + 1;
  emit cursorPositionUpdated(line, col);

  if (snippetSession_.isActive() && !snippetNavigating_) {
    cancelSnippetSession();
  }
}

} // namespace lighttex::editor
