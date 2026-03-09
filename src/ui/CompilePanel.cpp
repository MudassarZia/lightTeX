#include "ui/CompilePanel.h"

#include <QMouseEvent>
#include <QTextBlock>
#include <QTextCursor>

namespace lighttex::ui {

CompilePanel::CompilePanel(QWidget* parent) : QPlainTextEdit(parent) {
    setReadOnly(true);
    setMaximumHeight(200);

    QFont font("JetBrains Mono", 10);
    font.setStyleHint(QFont::Monospace);
    setFont(font);

    setStyleSheet(
        "QPlainTextEdit { background: #1e1e1e; color: #d4d4d4; "
        "border-top: 1px solid #3c3c3c; }");

    // Make clickable
    setCursor(Qt::PointingHandCursor);
}

void CompilePanel::setTheme(const lighttex::theme::Theme& theme) {
    QString bg = QString::fromStdString(theme.ui.panelBg);
    QString fg = QString::fromStdString(theme.colors.foreground);
    QString border = QString::fromStdString(theme.ui.panelBorder);
    setStyleSheet(
        QString("QPlainTextEdit { background: %1; color: %2; "
                "border-top: 1px solid %3; }")
            .arg(bg, fg, border));
}

void CompilePanel::setMessages(
    const std::vector<lighttex::compiler::CompileMessage>& messages) {
    clear();
    messages_ = messages;

    QTextCursor cursor(document());
    cursor.beginEditBlock();

    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& msg = messages[i];
        QString prefix;
        QColor color;

        switch (msg.kind) {
            case lighttex::compiler::MessageKind::Error:
                prefix = "ERROR";
                color = QColor("#f44747");
                break;
            case lighttex::compiler::MessageKind::Warning:
                prefix = "WARN";
                color = QColor("#dcdcaa");
                break;
            case lighttex::compiler::MessageKind::BadBox:
                prefix = "BADBOX";
                color = QColor("#d4d4d4");
                break;
            case lighttex::compiler::MessageKind::Info:
                prefix = "INFO";
                color = QColor("#569cd6");
                break;
        }

        if (i > 0) {
            cursor.insertText("\n");
        }

        QTextCharFormat fmt;
        fmt.setForeground(color);

        QString line = QString("[%1]").arg(prefix);
        if (msg.file) {
            line += " " + QString::fromStdString(*msg.file);
        }
        if (msg.line) {
            line += ":" + QString::number(*msg.line);
        }
        line += " " + QString::fromStdString(msg.message);

        cursor.insertText(line, fmt);
    }

    cursor.endEditBlock();

    if (!messages.empty()) {
        show();
    }
}

void CompilePanel::setLogOutput(const std::string& log) {
    setPlainText(QString::fromStdString(log));
    messages_.clear();
    if (!log.empty()) {
        show();
    }
}

void CompilePanel::clearMessages() {
    clear();
    messages_.clear();
    hide();
}

void CompilePanel::mousePressEvent(QMouseEvent* event) {
    QPlainTextEdit::mousePressEvent(event);

    QTextCursor cursor = cursorForPosition(event->pos());
    int lineNum = cursor.blockNumber();

    if (lineNum >= 0 && lineNum < static_cast<int>(messages_.size())) {
        emit messageClicked(messages_[static_cast<size_t>(lineNum)]);
    }
}

} // namespace lighttex::ui
