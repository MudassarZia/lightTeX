#include "ui/MainWindow.h"

#include <QVBoxLayout>

namespace lighttex::ui {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
  setupUi();
  setWindowTitle("lightTex");
  resize(1280, 800);
}

void MainWindow::setupUi() {
  // Central widget
  auto *centralWidget = new QWidget(this);
  auto *mainLayout = new QVBoxLayout(centralWidget);
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->setSpacing(0);

  // Toolbar
  setupToolBar();

  // Main horizontal splitter: fileTree | editor+compile | PDF
  mainSplitter_ = new QSplitter(Qt::Horizontal, centralWidget);

  // File tree sidebar (hidden by default)
  fileTree_ = new FileTreeWidget();
  fileTree_->setFixedWidth(200);
  fileTree_->hide();
  mainSplitter_->addWidget(fileTree_);

  // Vertical splitter for editor area (find bar + editor + compile panel)
  verticalSplitter_ = new QSplitter(Qt::Vertical);

  // Editor with find/replace bar overlay container
  auto *editorContainer = new QWidget();
  auto *editorLayout = new QVBoxLayout(editorContainer);
  editorLayout->setContentsMargins(0, 0, 0, 0);
  editorLayout->setSpacing(0);

  editor_ = new lighttex::editor::EditorWidget();
  findReplaceBar_ = new lighttex::editor::FindReplaceBar(editor_);
  findReplaceBar_->hide();

  editorLayout->addWidget(findReplaceBar_);
  editorLayout->addWidget(editor_);

  compilePanel_ = new CompilePanel();
  compilePanel_->setMaximumHeight(200);
  compilePanel_->hide();

  verticalSplitter_->addWidget(editorContainer);
  verticalSplitter_->addWidget(compilePanel_);
  verticalSplitter_->setStretchFactor(0, 3);
  verticalSplitter_->setStretchFactor(1, 1);

  pdfWidget_ = new lighttex::pdf::PdfWidget();

  mainSplitter_->addWidget(verticalSplitter_);
  mainSplitter_->addWidget(pdfWidget_);
  mainSplitter_->setStretchFactor(0, 0); // file tree: no stretch
  mainSplitter_->setStretchFactor(1, 1);
  mainSplitter_->setStretchFactor(2, 1);

  mainLayout->addWidget(mainSplitter_);

  setCentralWidget(centralWidget);

  // Status bar
  statusBar_ = new LightTexStatusBar(this);
  setStatusBar(statusBar_);

  // Command palette (overlay)
  commandPalette_ = new CommandPalette(this);
  commandPalette_->hide();
}

void MainWindow::setupToolBar() {
  toolBar_ = addToolBar("Main");
  toolBar_->setMovable(false);
  toolBar_->setFloatable(false);
  // Actions are added by AppState/Actions
}

} // namespace lighttex::ui
