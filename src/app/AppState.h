#pragma once

#include "compiler/Pipeline.h"
#include "core/Document.h"
#include "syntax/Highlighter.h"
#include "syntax/SyntaxHighlighter.h"
#include "theme/ThemeManager.h"

#include <QObject>
#include <memory>

namespace lighttex::app {

class AppState : public QObject {
    Q_OBJECT

public:
    explicit AppState(QObject* parent = nullptr);

    lighttex::core::Document& document() { return document_; }
    lighttex::compiler::Compiler& compiler() { return compiler_; }
    lighttex::theme::ThemeManager& themeManager() { return themeManager_; }
    lighttex::syntax::Highlighter& highlighter() { return highlighter_; }

    void openFile(const std::string& path);
    void saveFile();
    void saveFileAs(const std::string& path);
    void compile();

    [[nodiscard]] const lighttex::compiler::CompileResult& lastCompileResult() const {
        return lastResult_;
    }

signals:
    void fileOpened(const QString& name, const QString& content);
    void fileSaved();
    void compilationStarted();
    void compilationFinished(lighttex::compiler::CompileResult result);

private slots:
    void onCompilationFinished(lighttex::compiler::CompileResult result);

private:
    lighttex::core::Document document_;
    lighttex::compiler::Compiler compiler_;
    lighttex::theme::ThemeManager themeManager_;
    lighttex::syntax::Highlighter highlighter_;
    lighttex::compiler::CompileResult lastResult_;
};

} // namespace lighttex::app
