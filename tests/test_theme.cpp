#include "theme/Theme.h"
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QFile>
#include <QTextStream>

using namespace lighttex::theme;

class ThemeTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    if (!QCoreApplication::instance()) {
      static int argc = 1;
      static char arg0[] = "test";
      static char *argv[] = {arg0, nullptr};
      static QCoreApplication app(argc, argv);
    }
  }
};

TEST_F(ThemeTest, LoadDarkTheme) {
  std::string toml = R"(
name = "Dark"
kind = "dark"

[colors]
background = "#1e1e1e"
foreground = "#d4d4d4"
cursor = "#aeafad"
selection = "#264f78"
line_highlight = "#2a2d2e"
gutter = "#1e1e1e"
gutter_foreground = "#858585"

[syntax]
command = "#569cd6"
environment = "#c586c0"
math = "#dcdcaa"
comment = "#6a9955"
string = "#ce9178"
number = "#b5cea8"
bracket = "#ffd700"
error = "#f44747"

[ui]
statusbar_bg = "#007acc"
statusbar_fg = "#ffffff"
sidebar_bg = "#252526"
sidebar_fg = "#cccccc"
panel_bg = "#1e1e1e"
panel_border = "#3c3c3c"
)";

  Theme theme = loadTheme(toml);
  EXPECT_EQ(theme.name, "Dark");
  EXPECT_EQ(theme.kind, ThemeKind::Dark);
  EXPECT_EQ(theme.colors.background, "#1e1e1e");
  EXPECT_EQ(theme.syntax.command, "#569cd6");
  EXPECT_EQ(theme.ui.statusbarBg, "#007acc");
}

TEST_F(ThemeTest, LoadLightTheme) {
  std::string toml = R"(
name = "Light"
kind = "light"

[colors]
background = "#ffffff"
foreground = "#333333"
cursor = "#000000"
selection = "#add6ff"
line_highlight = "#f5f5f5"
gutter = "#ffffff"
gutter_foreground = "#999999"

[syntax]
command = "#0000ff"

[ui]
statusbar_bg = "#007acc"
statusbar_fg = "#ffffff"
)";

  Theme theme = loadTheme(toml);
  EXPECT_EQ(theme.name, "Light");
  EXPECT_EQ(theme.kind, ThemeKind::Light);
  EXPECT_EQ(theme.colors.background, "#ffffff");
}

TEST_F(ThemeTest, LoadInvalidToml) {
  std::string toml = "name = \"Broken\"\n[colors\nthis is not valid {{{}}}";
  EXPECT_THROW(loadTheme(toml), ThemeLoadError);
}

TEST_F(ThemeTest, ThemeToColorMap) {
  Theme theme;
  theme.name = "Test";
  theme.colors.background = "#111111";
  theme.syntax.command = "#222222";
  theme.ui.statusbarBg = "#333333";

  auto map = theme.toColorMap();
  EXPECT_EQ(map["--editor-bg"], "#111111");
  EXPECT_EQ(map["--syntax-command"], "#222222");
  EXPECT_EQ(map["--ui-statusbar-bg"], "#333333");
  EXPECT_EQ(map.size(), 21u);
}

TEST_F(ThemeTest, ToStyleSheetContainsAllWidgets) {
  Theme theme;
  theme.name = "Test";
  std::string qss = theme.toStyleSheet();

  // Should contain rules for all major widget types
  EXPECT_NE(qss.find("QMainWindow"), std::string::npos);
  EXPECT_NE(qss.find("QPlainTextEdit"), std::string::npos);
  EXPECT_NE(qss.find("QStatusBar"), std::string::npos);
  EXPECT_NE(qss.find("QToolBar"), std::string::npos);
  EXPECT_NE(qss.find("QToolButton"), std::string::npos);
  EXPECT_NE(qss.find("QSplitter"), std::string::npos);
  EXPECT_NE(qss.find("QScrollBar"), std::string::npos);
  EXPECT_NE(qss.find("QScrollArea"), std::string::npos);
  EXPECT_NE(qss.find("QLabel"), std::string::npos);
}

TEST_F(ThemeTest, DarkAndLightProduceDifferentStyleSheets) {
  Theme dark = darkTheme();
  Theme light = lightTheme();

  std::string darkQss = dark.toStyleSheet();
  std::string lightQss = light.toStyleSheet();

  EXPECT_NE(darkQss, lightQss);
  // Dark should reference dark backgrounds
  EXPECT_NE(darkQss.find("#1e1e1e"), std::string::npos);
  // Light should reference light backgrounds
  EXPECT_NE(lightQss.find("#ffffff"), std::string::npos);
}

TEST_F(ThemeTest, LightThemeFallbackValues) {
  Theme light = lightTheme();
  EXPECT_EQ(light.name, "Light");
  EXPECT_EQ(light.kind, ThemeKind::Light);
  EXPECT_EQ(light.colors.background, "#ffffff");
  EXPECT_EQ(light.colors.foreground, "#333333");
  EXPECT_EQ(light.syntax.command, "#0000ff");
}

TEST_F(ThemeTest, DarkThemeFallbackValues) {
  Theme dark = darkTheme();
  EXPECT_EQ(dark.name, "Dark");
  EXPECT_EQ(dark.kind, ThemeKind::Dark);
  // Dark defaults from struct
  EXPECT_EQ(dark.colors.background, "#1e1e1e");
  EXPECT_EQ(dark.colors.foreground, "#d4d4d4");
}
