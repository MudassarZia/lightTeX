#pragma once

#include "theme/Theme.h"

#include <QObject>
#include <string>

namespace lighttex::theme {

class ThemeManager : public QObject {
    Q_OBJECT

public:
    explicit ThemeManager(QObject* parent = nullptr);

    void setTheme(const std::string& name);
    void setTheme(const Theme& theme);
    void loadFromFile(const std::string& path);

    [[nodiscard]] const Theme& current() const { return current_; }
    [[nodiscard]] const std::string& currentName() const { return current_.name; }

    void applyToApplication() const;

signals:
    void themeChanged(const lighttex::theme::Theme& theme);

private:
    Theme current_;
};

} // namespace lighttex::theme
