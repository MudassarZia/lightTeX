#include "app/Actions.h"
#include "ui/CommandPalette.h"

#include <QFileDialog>
#include <QKeySequence>
#include <QMessageBox>
#include <QToolBar>

namespace lighttex::app {

Actions::Actions(AppState* state, lighttex::ui::MainWindow* window,
                 QObject* parent)
    : QObject(parent), state_(state), window_(window) {
    setupActions();
}

void Actions::setupActions() {
    // Open file
    openAction_ = new QAction("Open", this);
    openAction_->setShortcut(QKeySequence::Open);
    connect(openAction_, &QAction::triggered, this, &Actions::openFile);
    window_->addAction(openAction_);

    // Save file
    saveAction_ = new QAction("Save", this);
    saveAction_->setShortcut(QKeySequence::Save);
    connect(saveAction_, &QAction::triggered, this, &Actions::saveFile);
    window_->addAction(saveAction_);

    // Compile
    compileAction_ = new QAction("Compile", this);
    compileAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Return));
    connect(compileAction_, &QAction::triggered, this, &Actions::compileDocument);
    window_->addAction(compileAction_);

    // Theme switching
    themeDarkAction_ = new QAction("Dark Theme", this);
    connect(themeDarkAction_, &QAction::triggered, this, &Actions::switchThemeDark);

    themeLightAction_ = new QAction("Light Theme", this);
    connect(themeLightAction_, &QAction::triggered, this, &Actions::switchThemeLight);

    // Command palette
    paletteAction_ = new QAction("Command Palette", this);
    paletteAction_->setShortcut(QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P));
    connect(paletteAction_, &QAction::triggered, this, &Actions::toggleCommandPalette);
    window_->addAction(paletteAction_);

    // Add to toolbar
    QToolBar* toolbar = window_->findChild<QToolBar*>();
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
            [this](const lighttex::compiler::CompileResult& result) {
        window_->statusBar()->setCompileStatus(result.status);
        window_->compilePanel()->setMessages(result.messages);
        if (result.pdfPath) {
            window_->pdfWidget()->loadPdf(*result.pdfPath);
        }
    });

    // Wire up file open
    connect(state_, &AppState::fileOpened, this,
            [this](const QString& name, const QString& content) {
        window_->editor()->setPlainText(content);
        window_->setWindowTitle(name + " - lightTex");

        // Re-highlight
        auto events = state_->highlighter().parse(content.toStdString());
        // SyntaxHighlighter bridge handles this via QTextDocument
    });

    // Wire up theme changes
    connect(&state_->themeManager(), &lighttex::theme::ThemeManager::themeChanged,
            this, [this](const lighttex::theme::Theme& theme) {
        window_->editor()->setTheme(theme);
        window_->commandPalette()->setTheme(theme);
        window_->compilePanel()->setTheme(theme);
        window_->statusBar()->setEngine(state_->compiler().engine());
    });

    // Apply initial theme
    state_->themeManager().applyToApplication();
    window_->editor()->setTheme(state_->themeManager().current());
    window_->commandPalette()->setTheme(state_->themeManager().current());
    window_->compilePanel()->setTheme(state_->themeManager().current());

    // Setup command palette commands
    setupCommandPalette();
}

void Actions::openFile() {
    QString path = QFileDialog::getOpenFileName(
        window_, "Open LaTeX File", QString(),
        "LaTeX Files (*.tex *.ltx *.sty *.cls);;All Files (*)");

    if (path.isEmpty()) return;

    try {
        state_->openFile(path.toStdString());
    } catch (const lighttex::core::DocumentError& e) {
        QMessageBox::critical(window_, "Error",
                              QString("Cannot open file: %1").arg(e.what()));
    }
}

void Actions::saveFile() {
    if (!state_->document().path()) {
        QString path = QFileDialog::getSaveFileName(
            window_, "Save LaTeX File", QString(),
            "LaTeX Files (*.tex);;All Files (*)");
        if (path.isEmpty()) return;
        try {
            // Update document content from editor
            state_->document().replace(
                0, state_->document().length(),
                window_->editor()->toPlainText().toStdString());
            state_->saveFileAs(path.toStdString());
        } catch (const lighttex::core::DocumentError& e) {
            QMessageBox::critical(window_, "Error",
                                  QString("Cannot save file: %1").arg(e.what()));
        }
    } else {
        try {
            state_->document().replace(
                0, state_->document().length(),
                window_->editor()->toPlainText().toStdString());
            state_->saveFile();
        } catch (const lighttex::core::DocumentError& e) {
            QMessageBox::critical(window_, "Error",
                                  QString("Cannot save file: %1").arg(e.what()));
        }
    }
}

void Actions::compileDocument() {
    // Sync editor content to document
    std::string editorContent = window_->editor()->toPlainText().toStdString();
    if (state_->document().length() > 0 || !editorContent.empty()) {
        state_->document().replace(
            0, state_->document().length(), editorContent);
    }
    state_->compile();
}

void Actions::switchThemeDark() {
    state_->themeManager().setTheme("dark");
}

void Actions::switchThemeLight() {
    state_->themeManager().setTheme("light");
}

void Actions::toggleCommandPalette() {
    window_->commandPalette()->toggle();
}

void Actions::setupCommandPalette() {
    std::vector<lighttex::ui::Command> commands = {
        {"file.open", "Open File", "File", "Ctrl+O",
         [this]() { openFile(); }},
        {"file.save", "Save File", "File", "Ctrl+S",
         [this]() { saveFile(); }},
        {"compile", "Compile Document", "Build", "Ctrl+Enter",
         [this]() { compileDocument(); }},
        {"theme.dark", "Switch to Dark Theme", "Theme", "",
         [this]() { switchThemeDark(); }},
        {"theme.light", "Switch to Light Theme", "Theme", "",
         [this]() { switchThemeLight(); }},
    };
    window_->commandPalette()->setCommands(commands);
}

} // namespace lighttex::app
