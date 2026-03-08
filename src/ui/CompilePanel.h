#pragma once

#include "compiler/LogParser.h"
#include "theme/Theme.h"

#include <QPlainTextEdit>

namespace lighttex::ui {

class CompilePanel : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit CompilePanel(QWidget* parent = nullptr);

    void setTheme(const lighttex::theme::Theme& theme);
    void setMessages(const std::vector<lighttex::compiler::CompileMessage>& messages);
    void setLogOutput(const std::string& log);
    void clearMessages();
};

} // namespace lighttex::ui
