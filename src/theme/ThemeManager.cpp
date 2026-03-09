#include "theme/ThemeManager.h"

#include <QApplication>

namespace lighttex::theme {

ThemeManager::ThemeManager(QObject *parent) : QObject(parent) {
  current_ = darkTheme();
}

void ThemeManager::setTheme(const std::string &name) {
  if (name == "light" || name == "Light") {
    setTheme(lightTheme());
  } else {
    setTheme(darkTheme());
  }
}

void ThemeManager::setTheme(const Theme &theme) {
  current_ = theme;
  applyToApplication();
  emit themeChanged(current_);
}

void ThemeManager::loadFromFile(const std::string &path) {
  Theme theme = loadThemeFile(path);
  setTheme(theme);
}

void ThemeManager::applyToApplication() const {
  if (qApp) {
    qApp->setStyleSheet(QString::fromStdString(current_.toStyleSheet()));
  }
}

} // namespace lighttex::theme
