#pragma once

#include "app/AppState.h"
#include "ui/MainWindow.h"

#include <QAction>
#include <QObject>
#include <vector>

namespace lighttex::app {

class Actions : public QObject {
  Q_OBJECT

public:
  Actions(AppState *state, lighttex::ui::MainWindow *window,
          QObject *parent = nullptr);

  void setupActions();

private:
  void openFile();
  void saveFile();
  void compileDocument();
  void switchThemeDark();
  void switchThemeLight();
  void toggleCommandPalette();
  void toggleAutoCompile();
  void toggleFileTree();
  void showFind();
  void showFindReplace();
  void setupCommandPalette();

  AppState *state_;
  lighttex::ui::MainWindow *window_;

  QAction *openAction_;
  QAction *saveAction_;
  QAction *compileAction_;
  QAction *themeDarkAction_;
  QAction *themeLightAction_;
  QAction *paletteAction_;
  QAction *findAction_;
  QAction *findReplaceAction_;
  QAction *toggleFileTreeAction_;
};

} // namespace lighttex::app
