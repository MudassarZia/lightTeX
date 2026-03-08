#include "app/AppState.h"

#include <QFileInfo>

namespace lighttex::app {

AppState::AppState(QObject* parent) : QObject(parent) {
    connect(&compiler_, &lighttex::compiler::Compiler::compilationStarted,
            this, &AppState::compilationStarted);
    connect(&compiler_, &lighttex::compiler::Compiler::compilationFinished,
            this, &AppState::onCompilationFinished);
}

void AppState::openFile(const std::string& path) {
    document_ = lighttex::core::Document::open(path);
    QString name = QString::fromStdString(document_.displayName());
    QString content = QString::fromStdString(document_.text());
    emit fileOpened(name, content);
}

void AppState::saveFile() {
    document_.save();
    emit fileSaved();
}

void AppState::saveFileAs(const std::string& path) {
    document_.saveAs(path);
    emit fileSaved();
}

void AppState::compile() {
    auto path = document_.path();
    if (!path) return;

    // Save before compiling
    if (document_.isModified()) {
        document_.save();
    }

    compiler_.compile(*path);
}

void AppState::onCompilationFinished(lighttex::compiler::CompileResult result) {
    lastResult_ = std::move(result);
    emit compilationFinished(lastResult_);
}

} // namespace lighttex::app
