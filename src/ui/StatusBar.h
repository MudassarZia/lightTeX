#pragma once

#include "compiler/Engine.h"
#include "compiler/Pipeline.h"

#include <QLabel>
#include <QStatusBar>

namespace lighttex::ui {

class LightTexStatusBar : public QStatusBar {
    Q_OBJECT

public:
    explicit LightTexStatusBar(QWidget* parent = nullptr);

    void setCursorPosition(int line, int col);
    void setCompileStatus(lighttex::compiler::CompileStatus status);
    void setEngine(lighttex::compiler::Engine engine);
    void setFileName(const QString& name);
    void setAutoCompile(bool enabled);
    void setLspStatus(const QString& status);

private:
    void updateStatusLabel();

    QLabel* positionLabel_;
    QLabel* statusLabel_;
    QLabel* engineLabel_;
    QLabel* autoCompileLabel_;
    QLabel* lspLabel_;
    QLabel* encodingLabel_;

    lighttex::compiler::CompileStatus compileStatus_ =
        lighttex::compiler::CompileStatus::Idle;
};

} // namespace lighttex::ui
