#pragma once

#include "theme/Theme.h"

#include <QLineEdit>
#include <QListWidget>
#include <QWidget>

#include <functional>
#include <string>
#include <vector>

namespace lighttex::ui {

struct Command {
  std::string id;
  std::string label;
  std::string category;
  std::string shortcut;
  std::function<void()> action;
};

class CommandPalette : public QWidget {
  Q_OBJECT

public:
  explicit CommandPalette(QWidget *parent = nullptr);

  void setCommands(const std::vector<Command> &commands);
  void setTheme(const lighttex::theme::Theme &theme);
  void toggle();
  void show();
  void hide();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

private slots:
  void onFilterChanged(const QString &text);
  void onItemActivated(QListWidgetItem *item);

private:
  void filterCommands(const QString &filter);
  void executeSelected();
  void positionOverlay();
  void applyColors(const std::string &bg, const std::string &fg,
                   const std::string &border, const std::string &selBg);

  QLineEdit *filterInput_;
  QListWidget *listWidget_;
  std::vector<Command> commands_;
  std::vector<size_t> filteredIndices_;
};

} // namespace lighttex::ui
