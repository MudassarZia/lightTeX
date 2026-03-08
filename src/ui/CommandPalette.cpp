#include "ui/CommandPalette.h"

#include <QKeyEvent>
#include <QPainter>
#include <QVBoxLayout>

namespace lighttex::ui {

CommandPalette::CommandPalette(QWidget* parent) : QWidget(parent) {
    setWindowFlags(Qt::Widget);
    setAttribute(Qt::WA_TranslucentBackground);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    filterInput_ = new QLineEdit(this);
    filterInput_->setPlaceholderText("Type a command...");
    layout->addWidget(filterInput_);

    listWidget_ = new QListWidget(this);
    listWidget_->setMaximumHeight(300);
    layout->addWidget(listWidget_);

    // Apply default dark theme colors
    applyColors("#252526", "#d4d4d4", "#3c3c3c", "#094771");

    connect(filterInput_, &QLineEdit::textChanged,
            this, &CommandPalette::onFilterChanged);
    connect(listWidget_, &QListWidget::itemActivated,
            this, &CommandPalette::onItemActivated);

    filterInput_->installEventFilter(this);
    listWidget_->installEventFilter(this);
}

void CommandPalette::setCommands(const std::vector<Command>& commands) {
    commands_ = commands;
}

void CommandPalette::setTheme(const lighttex::theme::Theme& theme) {
    std::string bg = theme.ui.panelBg;
    std::string fg = theme.colors.foreground;
    std::string border = theme.ui.panelBorder;
    std::string selBg = theme.colors.selection;
    applyColors(bg, fg, border, selBg);
}

void CommandPalette::applyColors(const std::string& bg, const std::string& fg,
                                  const std::string& border, const std::string& selBg) {
    QString qbg = QString::fromStdString(bg);
    QString qfg = QString::fromStdString(fg);
    QString qborder = QString::fromStdString(border);
    QString qselBg = QString::fromStdString(selBg);

    filterInput_->setStyleSheet(
        QString("QLineEdit { padding: 8px; font-size: 14px; "
                "background: %1; color: %2; border: 1px solid %3; }")
            .arg(qbg, qfg, qborder));
    listWidget_->setStyleSheet(
        QString("QListWidget { background: %1; color: %2; border: 1px solid %3; }"
                "QListWidget::item { padding: 6px; }"
                "QListWidget::item:selected { background: %4; }")
            .arg(qbg, qfg, qborder, qselBg));
}

void CommandPalette::toggle() {
    if (isVisible()) {
        hide();
    } else {
        show();
    }
}

void CommandPalette::show() {
    positionOverlay();
    filterInput_->clear();
    filterCommands("");
    QWidget::show();
    filterInput_->setFocus();
    raise();
}

void CommandPalette::hide() {
    QWidget::hide();
}

bool CommandPalette::eventFilter(QObject* obj, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        auto* keyEvent = static_cast<QKeyEvent*>(event);

        if (keyEvent->key() == Qt::Key_Escape) {
            hide();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Return ||
            keyEvent->key() == Qt::Key_Enter) {
            executeSelected();
            return true;
        }

        if (keyEvent->key() == Qt::Key_Down) {
            int row = listWidget_->currentRow();
            if (row < listWidget_->count() - 1) {
                listWidget_->setCurrentRow(row + 1);
            }
            return true;
        }

        if (keyEvent->key() == Qt::Key_Up) {
            int row = listWidget_->currentRow();
            if (row > 0) {
                listWidget_->setCurrentRow(row - 1);
            }
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void CommandPalette::paintEvent(QPaintEvent* /*event*/) {
    // Draw semi-transparent backdrop
    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 100));
}

void CommandPalette::onFilterChanged(const QString& text) {
    filterCommands(text);
}

void CommandPalette::onItemActivated(QListWidgetItem* /*item*/) {
    executeSelected();
}

void CommandPalette::filterCommands(const QString& filter) {
    listWidget_->clear();
    filteredIndices_.clear();

    QString lowerFilter = filter.toLower();

    for (size_t i = 0; i < commands_.size(); ++i) {
        const auto& cmd = commands_[i];
        QString label = QString::fromStdString(cmd.label);
        QString category = QString::fromStdString(cmd.category);

        if (filter.isEmpty() ||
            label.toLower().contains(lowerFilter) ||
            category.toLower().contains(lowerFilter)) {
            QString display = label;
            if (!cmd.shortcut.empty()) {
                display += "  (" + QString::fromStdString(cmd.shortcut) + ")";
            }
            if (!cmd.category.empty()) {
                display = "[" + category + "] " + display;
            }
            listWidget_->addItem(display);
            filteredIndices_.push_back(i);
        }
    }

    if (listWidget_->count() > 0) {
        listWidget_->setCurrentRow(0);
    }
}

void CommandPalette::executeSelected() {
    int row = listWidget_->currentRow();
    if (row >= 0 && row < static_cast<int>(filteredIndices_.size())) {
        size_t cmdIdx = filteredIndices_[static_cast<size_t>(row)];
        hide();
        if (commands_[cmdIdx].action) {
            commands_[cmdIdx].action();
        }
    }
}

void CommandPalette::positionOverlay() {
    if (parentWidget()) {
        QRect parentRect = parentWidget()->rect();
        int w = qMin(500, parentRect.width() - 40);
        int x = (parentRect.width() - w) / 2;
        setGeometry(x, 50, w, 400);
    }
}

} // namespace lighttex::ui
