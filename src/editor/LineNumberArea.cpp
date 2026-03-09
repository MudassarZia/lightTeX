#include "editor/LineNumberArea.h"
#include "editor/EditorWidget.h"

namespace lighttex::editor {

LineNumberArea::LineNumberArea(EditorWidget *editor)
    : QWidget(editor), editor_(editor) {}

QSize LineNumberArea::sizeHint() const {
  return QSize(editor_->lineNumberAreaWidth(), 0);
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
  editor_->lineNumberAreaPaintEvent(event);
}

} // namespace lighttex::editor
