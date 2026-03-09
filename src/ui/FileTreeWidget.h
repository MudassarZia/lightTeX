#pragma once

#include "theme/Theme.h"

#include <QFileSystemModel>
#include <QTreeView>

namespace lighttex::ui {

class FileTreeWidget : public QTreeView {
  Q_OBJECT

public:
  explicit FileTreeWidget(QWidget *parent = nullptr);

  void setRootPath(const QString &path);
  void setTheme(const lighttex::theme::Theme &theme);

signals:
  void fileActivated(const QString &filePath);

private slots:
  void onActivated(const QModelIndex &index);

private:
  QFileSystemModel *model_;
};

} // namespace lighttex::ui
