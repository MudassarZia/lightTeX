#include "ui/CompilePanel.h"

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
    QString text;

    for (const auto& msg : messages) {
        QString prefix;
        QString color;

        switch (msg.kind) {
            case lighttex::compiler::MessageKind::Error:
                prefix = "ERROR";
                color = "#f44747";
                break;
            case lighttex::compiler::MessageKind::Warning:
                prefix = "WARN";
                color = "#dcdcaa";
                break;
            case lighttex::compiler::MessageKind::BadBox:
                prefix = "BADBOX";
                color = "#d4d4d4";
                break;
            case lighttex::compiler::MessageKind::Info:
                prefix = "INFO";
                color = "#569cd6";
                break;
        }

        QString line = QString("[%1]").arg(prefix);
        if (msg.file) {
            line += " " + QString::fromStdString(*msg.file);
        }
        if (msg.line) {
            line += ":" + QString::number(*msg.line);
        }
        line += " " + QString::fromStdString(msg.message);
        text += line + "\n";
    }

    setPlainText(text);

    if (!messages.empty()) {
        show();
    }
}

void CompilePanel::setLogOutput(const std::string& log) {
    setPlainText(QString::fromStdString(log));
    if (!log.empty()) {
        show();
    }
}

void CompilePanel::clearMessages() {
    clear();
    hide();
}

} // namespace lighttex::ui
