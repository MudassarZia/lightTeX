#include "theme/Theme.h"

#include <QFile>
#include <QTextStream>

#include <toml++/toml.hpp>

namespace lighttex::theme {

static std::string getStr(const toml::table &tbl, const std::string &key,
                          const std::string &defaultVal) {
  auto node = tbl[key];
  if (node.is_string()) {
    return std::string(node.as_string()->get());
  }
  return defaultVal;
}

Theme loadTheme(const std::string &tomlStr) {
  toml::table tbl;
  try {
    tbl = toml::parse(tomlStr);
  } catch (const toml::parse_error &e) {
    throw ThemeLoadError(std::string("TOML parse error: ") + e.what());
  }

  Theme theme;
  theme.name = getStr(tbl, "name", "Unknown");

  std::string kindStr = getStr(tbl, "kind", "dark");
  theme.kind = (kindStr == "light") ? ThemeKind::Light : ThemeKind::Dark;

  if (auto colors = tbl["colors"].as_table()) {
    theme.colors.background =
        getStr(*colors, "background", theme.colors.background);
    theme.colors.foreground =
        getStr(*colors, "foreground", theme.colors.foreground);
    theme.colors.cursor = getStr(*colors, "cursor", theme.colors.cursor);
    theme.colors.selection =
        getStr(*colors, "selection", theme.colors.selection);
    theme.colors.lineHighlight =
        getStr(*colors, "line_highlight", theme.colors.lineHighlight);
    theme.colors.gutter = getStr(*colors, "gutter", theme.colors.gutter);
    theme.colors.gutterForeground =
        getStr(*colors, "gutter_foreground", theme.colors.gutterForeground);
  }

  if (auto syntax = tbl["syntax"].as_table()) {
    theme.syntax.command = getStr(*syntax, "command", theme.syntax.command);
    theme.syntax.environment =
        getStr(*syntax, "environment", theme.syntax.environment);
    theme.syntax.math = getStr(*syntax, "math", theme.syntax.math);
    theme.syntax.comment = getStr(*syntax, "comment", theme.syntax.comment);
    theme.syntax.string = getStr(*syntax, "string", theme.syntax.string);
    theme.syntax.number = getStr(*syntax, "number", theme.syntax.number);
    theme.syntax.bracket = getStr(*syntax, "bracket", theme.syntax.bracket);
    theme.syntax.error = getStr(*syntax, "error", theme.syntax.error);
  }

  if (auto ui = tbl["ui"].as_table()) {
    theme.ui.statusbarBg = getStr(*ui, "statusbar_bg", theme.ui.statusbarBg);
    theme.ui.statusbarFg = getStr(*ui, "statusbar_fg", theme.ui.statusbarFg);
    theme.ui.sidebarBg = getStr(*ui, "sidebar_bg", theme.ui.sidebarBg);
    theme.ui.sidebarFg = getStr(*ui, "sidebar_fg", theme.ui.sidebarFg);
    theme.ui.panelBg = getStr(*ui, "panel_bg", theme.ui.panelBg);
    theme.ui.panelBorder = getStr(*ui, "panel_border", theme.ui.panelBorder);
  }

  return theme;
}

Theme loadThemeFile(const std::string &path) {
  QFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    throw ThemeLoadError("Cannot open theme file: " + path);
  }
  QTextStream stream(&file);
  QString content = stream.readAll();
  file.close();
  return loadTheme(content.toStdString());
}

Theme darkTheme() {
  QFile file(":/themes/dark.toml");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    return loadTheme(stream.readAll().toStdString());
  }
  // Fallback: return default dark theme (struct defaults are dark)
  Theme theme;
  theme.name = "Dark";
  theme.kind = ThemeKind::Dark;
  return theme;
}

Theme lightTheme() {
  QFile file(":/themes/light.toml");
  if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    QTextStream stream(&file);
    return loadTheme(stream.readAll().toStdString());
  }
  // Fallback
  Theme theme;
  theme.name = "Light";
  theme.kind = ThemeKind::Light;
  theme.colors.background = "#ffffff";
  theme.colors.foreground = "#333333";
  theme.colors.cursor = "#000000";
  theme.colors.selection = "#add6ff";
  theme.colors.lineHighlight = "#f5f5f5";
  theme.colors.gutter = "#ffffff";
  theme.colors.gutterForeground = "#999999";
  theme.syntax.command = "#0000ff";
  theme.syntax.environment = "#af00db";
  theme.syntax.math = "#795e26";
  theme.syntax.comment = "#008000";
  theme.syntax.string = "#a31515";
  theme.syntax.number = "#098658";
  theme.syntax.bracket = "#0431fa";
  theme.syntax.error = "#e51400";
  theme.ui.statusbarBg = "#007acc";
  theme.ui.statusbarFg = "#ffffff";
  theme.ui.sidebarBg = "#f3f3f3";
  theme.ui.sidebarFg = "#333333";
  theme.ui.panelBg = "#ffffff";
  theme.ui.panelBorder = "#e0e0e0";
  return theme;
}

std::string Theme::toStyleSheet() const {
  std::string qss;

  // Main window and central widget
  qss += "QMainWindow { background: " + colors.background + "; }\n";
  qss += "QWidget { background: " + colors.background +
         "; color: " + colors.foreground + "; }\n";

  // Editor and text areas
  qss += "QPlainTextEdit { background: " + colors.background +
         "; color: " + colors.foreground +
         "; selection-background-color: " + colors.selection + "; }\n";

  // Status bar
  qss += "QStatusBar { background: " + ui.statusbarBg +
         "; color: " + ui.statusbarFg + "; }\n";
  qss +=
      "QStatusBar QLabel { background: transparent; color: " + ui.statusbarFg +
      "; }\n";

  // Toolbar
  qss += "QToolBar { background: " + ui.panelBg +
         "; border-bottom: 1px solid " + ui.panelBorder +
         "; color: " + colors.foreground + "; }\n";
  qss += "QToolButton { background: transparent; color: " + colors.foreground +
         "; padding: 4px 8px; border: none; }\n";
  qss += "QToolButton:hover { background: " + colors.selection + "; }\n";

  // Menus
  qss += "QMenuBar { background: " + ui.panelBg +
         "; color: " + colors.foreground + "; }\n";
  qss += "QMenu { background: " + ui.panelBg + "; color: " + colors.foreground +
         "; border: 1px solid " + ui.panelBorder + "; }\n";
  qss += "QMenu::item:selected { background: " + colors.selection + "; }\n";

  // Splitters
  qss += "QSplitter::handle { background: " + ui.panelBorder + "; }\n";
  qss += "QSplitter::handle:horizontal { width: 2px; }\n";
  qss += "QSplitter::handle:vertical { height: 2px; }\n";

  // Scrollbars
  qss += "QScrollBar:vertical { background: " + ui.panelBg +
         "; width: 12px; margin: 0; }\n";
  qss += "QScrollBar::handle:vertical { background: " + ui.panelBorder +
         "; min-height: 20px; border-radius: 3px; margin: 2px; }\n";
  qss += "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical "
         "{ height: 0; }\n";
  qss += "QScrollBar:horizontal { background: " + ui.panelBg +
         "; height: 12px; margin: 0; }\n";
  qss += "QScrollBar::handle:horizontal { background: " + ui.panelBorder +
         "; min-width: 20px; border-radius: 3px; margin: 2px; }\n";
  qss += "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal "
         "{ width: 0; }\n";

  // Tooltips
  qss += "QToolTip { background: " + ui.panelBg +
         "; color: " + colors.foreground + "; border: 1px solid " +
         ui.panelBorder + "; }\n";

  // Labels (generic)
  qss += "QLabel { background: transparent; }\n";

  // Scroll area (PDF widget)
  qss +=
      "QScrollArea { background: " + colors.background + "; border: none; }\n";

  return qss;
}

std::map<std::string, std::string> Theme::toColorMap() const {
  return {
      {"--editor-bg", colors.background},
      {"--editor-fg", colors.foreground},
      {"--editor-cursor", colors.cursor},
      {"--editor-selection", colors.selection},
      {"--editor-line-highlight", colors.lineHighlight},
      {"--editor-gutter", colors.gutter},
      {"--editor-gutter-fg", colors.gutterForeground},
      {"--syntax-command", syntax.command},
      {"--syntax-environment", syntax.environment},
      {"--syntax-math", syntax.math},
      {"--syntax-comment", syntax.comment},
      {"--syntax-string", syntax.string},
      {"--syntax-number", syntax.number},
      {"--syntax-bracket", syntax.bracket},
      {"--syntax-error", syntax.error},
      {"--ui-statusbar-bg", ui.statusbarBg},
      {"--ui-statusbar-fg", ui.statusbarFg},
      {"--ui-sidebar-bg", ui.sidebarBg},
      {"--ui-sidebar-fg", ui.sidebarFg},
      {"--ui-panel-bg", ui.panelBg},
      {"--ui-panel-border", ui.panelBorder},
  };
}

} // namespace lighttex::theme
