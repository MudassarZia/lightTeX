#include "ui/StatusBar.h"

namespace lighttex::ui {

LightTexStatusBar::LightTexStatusBar(QWidget* parent) : QStatusBar(parent) {
    positionLabel_ = new QLabel("Ln 1, Col 1", this);
    statusLabel_ = new QLabel("Ready", this);
    engineLabel_ = new QLabel("pdfLaTeX", this);
    autoCompileLabel_ = new QLabel("", this);
    lspLabel_ = new QLabel("", this);
    encodingLabel_ = new QLabel("UTF-8", this);

    addWidget(positionLabel_);
    addWidget(statusLabel_);
    addPermanentWidget(autoCompileLabel_);
    addPermanentWidget(lspLabel_);
    addPermanentWidget(engineLabel_);
    addPermanentWidget(encodingLabel_);
}

void LightTexStatusBar::setCursorPosition(int line, int col) {
    positionLabel_->setText(
        QString("Ln %1, Col %2").arg(line).arg(col));
}

void LightTexStatusBar::setCompileStatus(lighttex::compiler::CompileStatus status) {
    compileStatus_ = status;
    updateStatusLabel();
}

void LightTexStatusBar::setEngine(lighttex::compiler::Engine engine) {
    engineLabel_->setText(
        QString::fromStdString(std::string(
            lighttex::compiler::engineDisplayName(engine))));
}

void LightTexStatusBar::setFileName(const QString& /*name*/) {
    // File name can be shown in window title instead
}

void LightTexStatusBar::setAutoCompile(bool enabled) {
    if (enabled) {
        autoCompileLabel_->setText("Auto");
        autoCompileLabel_->setStyleSheet("QLabel { color: #4ec9b0; }");
    } else {
        autoCompileLabel_->setText("");
        autoCompileLabel_->setStyleSheet("");
    }
}

void LightTexStatusBar::setLspStatus(const QString& status) {
    lspLabel_->setText(status);
    if (status.contains("not found")) {
        lspLabel_->setStyleSheet("QLabel { color: #858585; }");
    } else {
        lspLabel_->setStyleSheet("QLabel { color: #4ec9b0; }");
    }
}

void LightTexStatusBar::updateStatusLabel() {
    switch (compileStatus_) {
        case lighttex::compiler::CompileStatus::Idle:
            statusLabel_->setText("Ready");
            statusLabel_->setStyleSheet("");
            break;
        case lighttex::compiler::CompileStatus::Compiling:
            statusLabel_->setText("Compiling...");
            statusLabel_->setStyleSheet("QLabel { color: #dcdcaa; }");
            break;
        case lighttex::compiler::CompileStatus::Success:
            statusLabel_->setText("Compiled");
            statusLabel_->setStyleSheet("QLabel { color: #4ec9b0; }");
            break;
        case lighttex::compiler::CompileStatus::Error:
            statusLabel_->setText("Error");
            statusLabel_->setStyleSheet("QLabel { color: #f44747; }");
            break;
    }
}

} // namespace lighttex::ui
