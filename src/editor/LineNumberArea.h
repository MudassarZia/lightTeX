#pragma once

#include <QWidget>

namespace lighttex::editor {

class EditorWidget;

class LineNumberArea : public QWidget {
  Q_OBJECT

public:
  explicit LineNumberArea(EditorWidget *editor);

  QSize sizeHint() const override;

protected:
  void paintEvent(QPaintEvent *event) override;

private:
  EditorWidget *editor_;
};

} // namespace lighttex::editor
