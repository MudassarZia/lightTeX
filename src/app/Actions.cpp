#include "app/Actions.h"
#include "ui/CommandPalette.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QTimer>
#include <QToolBar>

namespace lighttex::app {

Actions::Actions(AppState *state, lighttex::ui::MainWindow *window,
                 QObject *parent)
    : QObject(parent), state_(state), window_(window) {
  setupActions();
}

void Actions::setupActions() {
  auto &sc = state_->shortcuts();

  // Open file
  openAction_ = new QAction("Open", this);
  openAction_->setShortcut(sc.keySequence("file.open"));
  connect(openAction_, &QAction::triggered, this, &Actions::openFile);
  window_->addAction(openAction_);

  // Save file
  saveAction_ = new QAction("Save", this);
  saveAction_->setShortcut(sc.keySequence("file.save"));
  connect(saveAction_, &QAction::triggered, this, &Actions::saveFile);
  window_->addAction(saveAction_);

  // Compile
  compileAction_ = new QAction("Compile", this);
  compileAction_->setShortcut(sc.keySequence("compile"));
  connect(compileAction_, &QAction::triggered, this, &Actions::compileDocument);
  window_->addAction(compileAction_);

  // Theme switching
  themeDarkAction_ = new QAction("Dark Theme", this);
  connect(themeDarkAction_, &QAction::triggered, this,
          &Actions::switchThemeDark);

  themeLightAction_ = new QAction("Light Theme", this);
  connect(themeLightAction_, &QAction::triggered, this,
          &Actions::switchThemeLight);

  // Command palette
  paletteAction_ = new QAction("Command Palette", this);
  paletteAction_->setShortcut(sc.keySequence("palette"));
  connect(paletteAction_, &QAction::triggered, this,
          &Actions::toggleCommandPalette);
  window_->addAction(paletteAction_);

  // Find
  findAction_ = new QAction("Find", this);
  findAction_->setShortcut(sc.keySequence("find"));
  connect(findAction_, &QAction::triggered, this, &Actions::showFind);
  window_->addAction(findAction_);

  // Find & Replace
  findReplaceAction_ = new QAction("Find and Replace", this);
  findReplaceAction_->setShortcut(sc.keySequence("find_replace"));
  connect(findReplaceAction_, &QAction::triggered, this,
          &Actions::showFindReplace);
  window_->addAction(findReplaceAction_);

  // Toggle file tree
  toggleFileTreeAction_ = new QAction("Toggle File Tree", this);
  toggleFileTreeAction_->setShortcut(sc.keySequence("toggle_filetree"));
  connect(toggleFileTreeAction_, &QAction::triggered, this,
          &Actions::toggleFileTree);
  window_->addAction(toggleFileTreeAction_);

  // Add to toolbar
  QToolBar *toolbar = window_->findChild<QToolBar *>();
  if (toolbar) {
    toolbar->addAction(openAction_);
    toolbar->addAction(saveAction_);
    toolbar->addSeparator();
    toolbar->addAction(compileAction_);
  }

  // Wire up editor signals
  connect(window_->editor(),
          &lighttex::editor::EditorWidget::cursorPositionUpdated,
          window_->statusBar(),
          &lighttex::ui::LightTexStatusBar::setCursorPosition);

  // Wire up compilation signals
  connect(state_, &AppState::compilationStarted, this, [this]() {
    window_->statusBar()->setCompileStatus(
        lighttex::compiler::CompileStatus::Compiling);
    window_->compilePanel()->clearMessages();
  });

  connect(state_, &AppState::compilationFinished, this,
          [this](const lighttex::compiler::CompileResult &result) {
            window_->statusBar()->setCompileStatus(result.status);
            window_->compilePanel()->setMessages(result.messages);
            if (result.pdfPath) {
              window_->pdfWidget()->loadPdf(*result.pdfPath);
            }
          });

  // Wire up compile panel click-to-jump
  connect(window_->compilePanel(), &lighttex::ui::CompilePanel::messageClicked,
          this, [this](const lighttex::compiler::CompileMessage &msg) {
            if (msg.line) {
              auto *editor = window_->editor();
              QTextCursor cursor(
                  editor->document()->findBlockByNumber(*msg.line - 1));
              editor->setTextCursor(cursor);
              editor->setFocus();
            }
          });

  // Wire up file open
  connect(state_, &AppState::fileOpened, this,
          [this](const QString &name, const QString &content) {
            window_->editor()->setPlainText(content);
            window_->setWindowTitle(name + " - lightTex");

            // Update file tree root
            if (!state_->projectRoot().empty()) {
              window_->fileTree()->setRootPath(
                  QString::fromStdString(state_->projectRoot()));
            }
          });

  // Wire up file tree
  connect(window_->fileTree(), &lighttex::ui::FileTreeWidget::fileActivated,
          this, [this](const QString &path) {
            try {
              state_->openFile(path.toStdString());
            } catch (const lighttex::core::DocumentError &e) {
              QMessageBox::critical(
                  window_, "Error",
                  QString("Cannot open file: %1").arg(e.what()));
            }
          });

  // Wire up auto-compile on save
  connect(state_, &AppState::fileSaved, this, [this]() {
    if (state_->autoCompileEnabled()) {
      QTimer::singleShot(100, this, [this]() { compileDocument(); });
    }
  });

  // Wire up theme changes
  connect(&state_->themeManager(), &lighttex::theme::ThemeManager::themeChanged,
          this, [this](const lighttex::theme::Theme &theme) {
            window_->editor()->setTheme(theme);
            window_->commandPalette()->setTheme(theme);
            window_->compilePanel()->setTheme(theme);
            window_->findReplaceBar()->setTheme(theme);
            window_->fileTree()->setTheme(theme);
            window_->statusBar()->setEngine(state_->compiler().engine());
          });

  // Apply initial theme
  state_->themeManager().applyToApplication();
  window_->editor()->setTheme(state_->themeManager().current());
  window_->commandPalette()->setTheme(state_->themeManager().current());
  window_->compilePanel()->setTheme(state_->themeManager().current());
  window_->findReplaceBar()->setTheme(state_->themeManager().current());
  window_->fileTree()->setTheme(state_->themeManager().current());

  // Setup command palette commands
  setupCommandPalette();
}

void Actions::openFile() {
  QString path = QFileDialog::getOpenFileName(
      window_, "Open LaTeX File", QString(),
      "LaTeX Files (*.tex *.ltx *.sty *.cls);;All Files (*)");

  if (path.isEmpty())
    return;

  try {
    state_->openFile(path.toStdString());
  } catch (const lighttex::core::DocumentError &e) {
    QMessageBox::critical(window_, "Error",
                          QString("Cannot open file: %1").arg(e.what()));
  }
}

void Actions::saveFile() {
  if (!state_->document().path()) {
    QString path =
        QFileDialog::getSaveFileName(window_, "Save LaTeX File", QString(),
                                     "LaTeX Files (*.tex);;All Files (*)");
    if (path.isEmpty())
      return;
    try {
      // Update document content from editor
      state_->document().replace(
          0, state_->document().length(),
          window_->editor()->toPlainText().toStdString());
      state_->saveFileAs(path.toStdString());
    } catch (const lighttex::core::DocumentError &e) {
      QMessageBox::critical(window_, "Error",
                            QString("Cannot save file: %1").arg(e.what()));
    }
  } else {
    try {
      state_->document().replace(
          0, state_->document().length(),
          window_->editor()->toPlainText().toStdString());
      state_->saveFile();
    } catch (const lighttex::core::DocumentError &e) {
      QMessageBox::critical(window_, "Error",
                            QString("Cannot save file: %1").arg(e.what()));
    }
  }
}

void Actions::compileDocument() {
  // Sync editor content to document
  std::string editorContent = window_->editor()->toPlainText().toStdString();
  if (state_->document().length() > 0 || !editorContent.empty()) {
    state_->document().replace(0, state_->document().length(), editorContent);
  }
  state_->compile();
}

void Actions::switchThemeDark() { state_->themeManager().setTheme("dark"); }

void Actions::switchThemeLight() { state_->themeManager().setTheme("light"); }

void Actions::toggleCommandPalette() { window_->commandPalette()->toggle(); }

void Actions::toggleAutoCompile() {
  state_->setAutoCompile(!state_->autoCompileEnabled());
  window_->statusBar()->setAutoCompile(state_->autoCompileEnabled());
}

void Actions::toggleFileTree() {
  window_->fileTree()->setVisible(!window_->fileTree()->isVisible());
}

void Actions::showFind() { window_->findReplaceBar()->showFind(); }

void Actions::showFindReplace() {
  window_->findReplaceBar()->showFindReplace();
}

void Actions::setupCommandPalette() {
  auto &sc = state_->shortcuts();
  std::vector<lighttex::ui::Command> commands = {
      {"file.open", "Open File", "File",
       sc.keySequenceString("file.open").toStdString(),
       [this]() { openFile(); }},
      {"file.save", "Save File", "File",
       sc.keySequenceString("file.save").toStdString(),
       [this]() { saveFile(); }},
      {"compile", "Compile Document", "Build",
       sc.keySequenceString("compile").toStdString(),
       [this]() { compileDocument(); }},
      {"theme.dark", "Switch to Dark Theme", "Theme", "",
       [this]() { switchThemeDark(); }},
      {"theme.light", "Switch to Light Theme", "Theme", "",
       [this]() { switchThemeLight(); }},
      {"find", "Find", "Edit", sc.keySequenceString("find").toStdString(),
       [this]() { showFind(); }},
      {"find_replace", "Find and Replace", "Edit",
       sc.keySequenceString("find_replace").toStdString(),
       [this]() { showFindReplace(); }},
      {"toggle_filetree", "Toggle File Tree", "View",
       sc.keySequenceString("toggle_filetree").toStdString(),
       [this]() { toggleFileTree(); }},
      {"toggle_autocompile", "Toggle Auto-Compile", "Build", "",
       [this]() { toggleAutoCompile(); }},
  };
  window_->commandPalette()->setCommands(commands);
}

} // namespace lighttex::app
