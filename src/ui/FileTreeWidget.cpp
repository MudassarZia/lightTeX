#include "ui/FileTreeWidget.h"

#include <QHeaderView>

namespace lighttex::ui {

FileTreeWidget::FileTreeWidget(QWidget *parent) : QTreeView(parent) {
  model_ = new QFileSystemModel(this);
  model_->setNameFilterDisables(false);
  model_->setNameFilters(
      {"*.tex", "*.bib", "*.sty", "*.cls", "*.bst", "*.toml", "*.md", "*.txt"});

  setModel(model_);

  // Only show Name column — hide Size, Type, Date
  for (int i = 1; i < model_->columnCount(); ++i) {
    hideColumn(i);
  }

  // Minimalist style
  setHeaderHidden(true);
  setIndentation(16);
  setAnimated(false);
  setRootIsDecorated(true);

  // Monospace font
  QFont font("JetBrains Mono", 11);
  font.setStyleHint(QFont::Monospace);
  setFont(font);

  connect(this, &QTreeView::activated, this, &FileTreeWidget::onActivated);
  connect(this, &QTreeView::clicked, this, &FileTreeWidget::onActivated);
}

void FileTreeWidget::setRootPath(const QString &path) {
  QModelIndex rootIndex = model_->setRootPath(path);
  setRootIndex(rootIndex);
}

void FileTreeWidget::setTheme(const lighttex::theme::Theme &theme) {
  QString bg = QString::fromStdString(theme.ui.sidebarBg);
  QString fg = QString::fromStdString(theme.ui.sidebarFg);
  QString sel = QString::fromStdString(theme.colors.selection);

  setStyleSheet(QString("QTreeView { background: %1; color: %2; border: none; }"
                        "QTreeView::item { padding: 2px 4px; }"
                        "QTreeView::item:selected { background: %3; }"
                        "QTreeView::item:hover { background: %3; }")
                    .arg(bg, fg, sel));
}

void FileTreeWidget::onActivated(const QModelIndex &index) {
  if (!model_->isDir(index)) {
    emit fileActivated(model_->filePath(index));
  }
}

} // namespace lighttex::ui
