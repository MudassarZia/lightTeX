#pragma once

#include "editor/BracketMatcher.h"
#include "theme/Theme.h"

#include <QPlainTextEdit>

namespace lighttex::editor {

class LineNumberArea;

class EditorWidget : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit EditorWidget(QWidget* parent = nullptr);

    void setTheme(const lighttex::theme::Theme& theme);
    void lineNumberAreaPaintEvent(QPaintEvent* event);
    int lineNumberAreaWidth() const;

signals:
    void cursorPositionUpdated(int line, int col);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect& rect, int dy);
    void onCursorMoved();

private:
    LineNumberArea* lineNumberArea_;
    BracketMatcher bracketMatcher_;
    QColor lineHighlightColor_;
    QColor gutterBg_;
    QColor gutterFg_;
    QColor bracketMatchBg_ = QColor("#5a5a5a");
    QColor bracketMatchFg_ = QColor("#ffd700");
};

} // namespace lighttex::editor
