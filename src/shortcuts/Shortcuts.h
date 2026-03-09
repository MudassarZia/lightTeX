#pragma once

#include <QKeySequence>
#include <QString>

#include <map>
#include <string>

namespace lighttex::shortcuts {

class ShortcutManager {
public:
    ShortcutManager();

    void loadDefaults();
    bool loadFromFile(const std::string& path);

    [[nodiscard]] QKeySequence keySequence(const std::string& actionId) const;
    [[nodiscard]] QString keySequenceString(const std::string& actionId) const;
    [[nodiscard]] bool hasBinding(const std::string& actionId) const;

    void setBinding(const std::string& actionId, const QKeySequence& seq);
    [[nodiscard]] const std::map<std::string, QKeySequence>& bindings() const {
        return bindings_;
    }

    static std::string userConfigPath();

private:
    std::map<std::string, QKeySequence> bindings_;
};

} // namespace lighttex::shortcuts
