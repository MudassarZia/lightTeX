#pragma once

#include "compiler/Pipeline.h"
#include "core/Document.h"
#include "shortcuts/Shortcuts.h"
#include "syntax/Highlighter.h"
#include "syntax/SyntaxHighlighter.h"
#include "theme/ThemeManager.h"

#include <QObject>
#include <memory>

namespace lighttex::app {

class AppState : public QObject {
  Q_OBJECT

public:
  explicit AppState(QObject *parent = nullptr);

  lighttex::core::Document &document() { return document_; }
  lighttex::compiler::Compiler &compiler() { return compiler_; }
  lighttex::theme::ThemeManager &themeManager() { return themeManager_; }
  lighttex::syntax::Highlighter &highlighter() { return highlighter_; }
  lighttex::shortcuts::ShortcutManager &shortcuts() { return shortcuts_; }

  void openFile(const std::string &path);
  void saveFile();
  void saveFileAs(const std::string &path);
  void compile();

  [[nodiscard]] const lighttex::compiler::CompileResult &
  lastCompileResult() const {
    return lastResult_;
  }

  // Auto-compile
  [[nodiscard]] bool autoCompileEnabled() const { return autoCompileEnabled_; }
  void setAutoCompile(bool enabled) { autoCompileEnabled_ = enabled; }

  // Project root
  [[nodiscard]] const std::string &projectRoot() const { return projectRoot_; }

signals:
  void fileOpened(const QString &name, const QString &content);
  void fileSaved();
  void compilationStarted();
  void compilationFinished(lighttex::compiler::CompileResult result);

private slots:
  void onCompilationFinished(lighttex::compiler::CompileResult result);

private:
  void updateProjectRoot(const std::string &filePath);

  lighttex::core::Document document_;
  lighttex::compiler::Compiler compiler_;
  lighttex::theme::ThemeManager themeManager_;
  lighttex::syntax::Highlighter highlighter_;
  lighttex::shortcuts::ShortcutManager shortcuts_;
  lighttex::compiler::CompileResult lastResult_;
  bool autoCompileEnabled_ = false;
  std::string projectRoot_;
};

} // namespace lighttex::app
