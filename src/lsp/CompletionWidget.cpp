#include "lsp/CompletionWidget.h"

#include <QApplication>
#include <QKeyEvent>
#include <QPlainTextEdit>
#include <QScreen>
#include <QScrollBar>
#include <QTextDocument>
#include <QTimer>
#include <QWheelEvent>

namespace lighttex::lsp {

CompletionWidget::CompletionWidget(QPlainTextEdit *editor)
    : QListWidget(nullptr), editor_(editor) {
  setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
  setAttribute(Qt::WA_ShowWithoutActivating);
  setFocusPolicy(Qt::NoFocus);

  setMaximumHeight(200);
  setMaximumWidth(300);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  QFont font("JetBrains Mono", 11);
  font.setStyleHint(QFont::Monospace);
  setFont(font);

  setStyleSheet("QListWidget { background: #252526; color: #d4d4d4; "
                "border: 1px solid #3c3c3c; }"
                "QListWidget::item { padding: 2px 4px; }"
                "QListWidget::item:selected { background: #094771; }");

  if (editor_) {
    editor_->installEventFilter(this);
    editor_->viewport()->installEventFilter(this);
    if (auto *topLevel = editor_->window()) {
      topLevel->installEventFilter(this);
    }
  }
}

void CompletionWidget::showCompletions(
    const std::vector<CompletionItem> &items) {
  if (triggerPos_ < 0)
    return;

  // Verify cursor is still at or after the trigger
  int cursorPos = editor_->textCursor().position();
  if (cursorPos < triggerPos_) {
    hideCompletions();
    return;
  }

  // Remember current selection so we can restore it
  QString prevLabel;
  if (currentRow() >= 0 && currentRow() < static_cast<int>(items_.size())) {
    prevLabel = items_[static_cast<size_t>(currentRow())].label;
  }

  // Filter out junk items (single-char labels)
  items_.clear();
  for (const auto &item : items) {
    if (item.label.size() > 1) {
      items_.push_back(item);
    }
  }

  if (items_.empty()) {
    hideCompletions();
    return;
  }

  // Rebuild list, restoring previous selection if possible
  bool wasVisible = isVisible();
  clear();
  int restoreRow = 0;
  for (size_t i = 0; i < items_.size(); ++i) {
    const auto &item = items_[i];
    QString display = item.label;
    if (!item.detail.isEmpty()) {
      display += "  " + item.detail;
    }
    addItem(display);
    if (item.label == prevLabel) {
      restoreRow = static_cast<int>(i);
    }
  }

  setCurrentRow(restoreRow);
  positionPopup();
  show();

  // If this is the first display and cursor has moved past the trigger,
  // the items might be stale. Request a refresh for the current position.
  if (!wasVisible && cursorPos > triggerPos_ + 1) {
    QTimer::singleShot(0, this, [this]() {
      if (isVisible())
        emit refreshRequested();
    });
  }
}

void CompletionWidget::hideCompletions() {
  hide();
  items_.clear();
  triggerPos_ = -1;
  triggerKind_ = TriggerKind::Command;
  clear();
}

void CompletionWidget::startArgumentSession(int triggerPos) {
  triggerKind_ = TriggerKind::Argument;
  triggerPos_ = triggerPos;
  emit completionRequested();
}

void CompletionWidget::positionPopup() {
  if (!editor_ || triggerPos_ < 0)
    return;

  QTextCursor tc = editor_->textCursor();
  tc.setPosition(triggerPos_);
  QRect cursorRect = editor_->cursorRect(tc);
  QPoint globalPos = editor_->mapToGlobal(cursorRect.bottomLeft());

  if (auto *screen = QApplication::screenAt(globalPos)) {
    QRect geom = screen->availableGeometry();
    int h = sizeHintForRow(0) * qMin(count(), 10) + 2 * frameWidth();
    int w = qMin(maximumWidth(), sizeHintForColumn(0) + 20);

    if (globalPos.y() + h > geom.bottom()) {
      globalPos.setY(editor_->mapToGlobal(cursorRect.topLeft()).y() - h);
    }
    if (globalPos.x() + w > geom.right()) {
      globalPos.setX(geom.right() - w);
    }
  }

  move(globalPos);
}

bool CompletionWidget::eventFilter(QObject *obj, QEvent *event) {
  // Window deactivate → hide
  if (editor_ && obj == editor_->window() &&
      event->type() == QEvent::WindowDeactivate) {
    if (isVisible())
      hideCompletions();
    return false;
  }

  // Click on editor → hide
  if ((obj == editor_ || obj == editor_->viewport()) &&
      event->type() == QEvent::MouseButtonPress) {
    if (isVisible())
      hideCompletions();
    return false;
  }

  // Scroll wheel → scroll popup list
  if (obj == editor_ && event->type() == QEvent::Wheel && isVisible()) {
    auto *we = static_cast<QWheelEvent *>(event);
    int step = (we->angleDelta().y() > 0) ? -3 : 3;
    setCurrentRow(qBound(0, currentRow() + step, count() - 1));
    return true;
  }

  // Key presses on editor
  if (obj == editor_ && event->type() == QEvent::KeyPress) {
    auto *ke = static_cast<QKeyEvent *>(event);

    if (isVisible()) {
      // ── Popup is visible ──
      switch (ke->key()) {
      case Qt::Key_Escape:
        hideCompletions();
        return true;

      case Qt::Key_Return:
      case Qt::Key_Enter:
      case Qt::Key_Tab: {
        int row = currentRow();
        if (row >= 0 && row < static_cast<int>(items_.size())) {
          CompletionItem item = items_[static_cast<size_t>(row)];
          int tp = triggerPos_;
          int cp = editor_->textCursor().position();
          TriggerKind tk = triggerKind_;
          hideCompletions();
          emit itemChosen(item, tp, cp, tk);
        } else {
          hideCompletions();
        }
        return true;
      }

      case Qt::Key_Down:
        if (currentRow() < count() - 1)
          setCurrentRow(currentRow() + 1);
        return true;

      case Qt::Key_Up:
        if (currentRow() > 0)
          setCurrentRow(currentRow() - 1);
        return true;

      case Qt::Key_Shift:
      case Qt::Key_Control:
      case Qt::Key_Alt:
      case Qt::Key_Meta:
        return false;

      default:
        if (!ke->text().isEmpty() || ke->key() == Qt::Key_Backspace ||
            ke->key() == Qt::Key_Delete) {
          // Let editor handle the key, then ask for a refresh
          QTimer::singleShot(0, this, [this]() {
            if (!isVisible())
              return;
            int pos = editor_->textCursor().position();
            if (pos <= triggerPos_) {
              hideCompletions();
              return;
            }
            emit refreshRequested();
          });
          return false;
        }
        // Anything else (Ctrl combos, arrows, etc.) → dismiss
        hideCompletions();
        return false;
      }
    } else {
      // ── Popup not visible: detect trigger characters ──

      // Backslash → command completion
      if (ke->text() == "\\") {
        QTimer::singleShot(0, this, [this]() {
          if (!editor_)
            return;
          int pos = editor_->textCursor().position();
          if (pos <= 0)
            return;
          if (editor_->document()->characterAt(pos - 1) != '\\')
            return;
          triggerKind_ = TriggerKind::Command;
          triggerPos_ = pos - 1;
          emit completionRequested();
        });
      }

      // Open brace → argument completion
      if (ke->text() == "{") {
        QTimer::singleShot(0, this, [this]() {
          if (!editor_)
            return;
          int pos = editor_->textCursor().position();
          if (pos <= 0)
            return;
          if (editor_->document()->characterAt(pos - 1) != '{')
            return;
          triggerKind_ = TriggerKind::Argument;
          triggerPos_ = pos; // position after {
          emit completionRequested();
        });
      }

      return false;
    }
  }

  return QListWidget::eventFilter(obj, event);
}

} // namespace lighttex::lsp
