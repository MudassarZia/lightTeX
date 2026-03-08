#pragma once

#include <map>
#include <stdexcept>
#include <string>

namespace lighttex::theme {

enum class ThemeKind { Dark, Light };

struct ThemeColors {
    std::string background = "#1e1e1e";
    std::string foreground = "#d4d4d4";
    std::string cursor = "#aeafad";
    std::string selection = "#264f78";
    std::string lineHighlight = "#2a2d2e";
    std::string gutter = "#1e1e1e";
    std::string gutterForeground = "#858585";
};

struct SyntaxColors {
    std::string command = "#569cd6";
    std::string environment = "#c586c0";
    std::string math = "#dcdcaa";
    std::string comment = "#6a9955";
    std::string string = "#ce9178";
    std::string number = "#b5cea8";
    std::string bracket = "#ffd700";
    std::string error = "#f44747";
};

struct UiColors {
    std::string statusbarBg = "#007acc";
    std::string statusbarFg = "#ffffff";
    std::string sidebarBg = "#252526";
    std::string sidebarFg = "#cccccc";
    std::string panelBg = "#1e1e1e";
    std::string panelBorder = "#3c3c3c";
};

struct Theme {
    std::string name;
    ThemeKind kind = ThemeKind::Dark;
    ThemeColors colors;
    SyntaxColors syntax;
    UiColors ui;

    [[nodiscard]] std::string toStyleSheet() const;
    [[nodiscard]] std::map<std::string, std::string> toColorMap() const;
};

class ThemeLoadError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};

Theme loadTheme(const std::string& tomlStr);
Theme loadThemeFile(const std::string& path);
Theme darkTheme();
Theme lightTheme();

} // namespace lighttex::theme
