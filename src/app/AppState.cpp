#include "app/AppState.h"

#include <QDir>
#include <QFileInfo>

#include <filesystem>

namespace lighttex::app {

AppState::AppState(QObject* parent) : QObject(parent) {
    connect(&compiler_, &lighttex::compiler::Compiler::compilationStarted,
            this, &AppState::compilationStarted);
    connect(&compiler_, &lighttex::compiler::Compiler::compilationFinished,
            this, &AppState::onCompilationFinished);

    // Try loading user keybindings
    std::string configPath = lighttex::shortcuts::ShortcutManager::userConfigPath();
    shortcuts_.loadFromFile(configPath);
}

void AppState::openFile(const std::string& path) {
    document_ = lighttex::core::Document::open(path);
    updateProjectRoot(path);
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

void AppState::updateProjectRoot(const std::string& filePath) {
    namespace fs = std::filesystem;

    fs::path p(filePath);
    fs::path dir = p.parent_path();
    fs::path best = dir;

    // Walk up max 5 levels looking for project markers
    for (int i = 0; i < 5 && !dir.empty(); ++i) {
        if (fs::exists(dir / ".git") || fs::exists(dir / ".latexmkrc")) {
            best = dir;
            break;
        }
        auto parent = dir.parent_path();
        if (parent == dir) break;
        dir = parent;
    }

    projectRoot_ = best.string();
}

} // namespace lighttex::app
