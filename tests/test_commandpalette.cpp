#include "ui/CommandPalette.h"

#include <QApplication>
#include <QTest>
#include <gtest/gtest.h>

using namespace lighttex::ui;

class CommandPaletteTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        if (!QApplication::instance()) {
            static int argc = 1;
            static char arg0[] = "test";
            static char* argv[] = {arg0, nullptr};
            static QApplication app(argc, argv);
        }
    }
};

TEST_F(CommandPaletteTest, ShowHide) {
    CommandPalette palette;
    EXPECT_FALSE(palette.isVisible());

    palette.show();
    EXPECT_TRUE(palette.isVisible());

    palette.hide();
    EXPECT_FALSE(palette.isVisible());
}

TEST_F(CommandPaletteTest, Toggle) {
    CommandPalette palette;
    palette.toggle();
    EXPECT_TRUE(palette.isVisible());
    palette.toggle();
    EXPECT_FALSE(palette.isVisible());
}

TEST_F(CommandPaletteTest, SetCommands) {
    CommandPalette palette;
    bool executed = false;
    std::vector<Command> commands = {
        {"test.cmd", "Test Command", "Test", "Ctrl+T", [&]() { executed = true; }},
        {"test.cmd2", "Another Command", "Test", "", nullptr},
    };
    palette.setCommands(commands);
    // Commands set without crash
}

TEST_F(CommandPaletteTest, SetDarkTheme) {
    CommandPalette palette;
    lighttex::theme::Theme dark;
    dark.kind = lighttex::theme::ThemeKind::Dark;
    dark.ui.panelBg = "#1e1e1e";
    dark.colors.foreground = "#d4d4d4";
    dark.ui.panelBorder = "#3c3c3c";
    dark.colors.selection = "#264f78";
    // Should not crash
    palette.setTheme(dark);
}

TEST_F(CommandPaletteTest, SetLightTheme) {
    CommandPalette palette;
    lighttex::theme::Theme light;
    light.kind = lighttex::theme::ThemeKind::Light;
    light.ui.panelBg = "#ffffff";
    light.colors.foreground = "#333333";
    light.ui.panelBorder = "#e0e0e0";
    light.colors.selection = "#add6ff";
    // Should not crash
    palette.setTheme(light);
}
