#include "shortcuts/Shortcuts.h"

#include <QStandardPaths>
#include <fstream>
#include <sstream>
#include <toml++/toml.hpp>

namespace lighttex::shortcuts {

ShortcutManager::ShortcutManager() { loadDefaults(); }

void ShortcutManager::loadDefaults() {
  bindings_.clear();
  bindings_["file.open"] = QKeySequence(QKeySequence::Open);
  bindings_["file.save"] = QKeySequence(QKeySequence::Save);
  bindings_["compile"] = QKeySequence(Qt::CTRL | Qt::Key_Return);
  bindings_["palette"] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_P);
  bindings_["find"] = QKeySequence(QKeySequence::Find);
  bindings_["find_replace"] = QKeySequence(Qt::CTRL | Qt::Key_H);
  bindings_["toggle_filetree"] = QKeySequence(Qt::CTRL | Qt::Key_B);
  bindings_["goto_definition"] = QKeySequence(Qt::Key_F12);
}

bool ShortcutManager::loadFromFile(const std::string &path) {
  try {
    auto tbl = toml::parse_file(path);

    for (auto &[key, val] : tbl) {
      if (val.is_string()) {
        std::string actionId(key.str());
        std::string seqStr(*val.value<std::string>());
        QKeySequence seq(QString::fromStdString(seqStr));
        if (!seq.isEmpty()) {
          bindings_[actionId] = seq;
        }
      }
    }
    return true;
  } catch (const toml::parse_error &) {
    return false;
  } catch (const std::exception &) {
    return false;
  }
}

QKeySequence ShortcutManager::keySequence(const std::string &actionId) const {
  auto it = bindings_.find(actionId);
  if (it != bindings_.end()) {
    return it->second;
  }
  return {};
}

QString ShortcutManager::keySequenceString(const std::string &actionId) const {
  return keySequence(actionId).toString(QKeySequence::NativeText);
}

bool ShortcutManager::hasBinding(const std::string &actionId) const {
  return bindings_.find(actionId) != bindings_.end();
}

void ShortcutManager::setBinding(const std::string &actionId,
                                 const QKeySequence &seq) {
  bindings_[actionId] = seq;
}

std::string ShortcutManager::userConfigPath() {
  QString configDir =
      QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
  return (configDir + "/keybindings.toml").toStdString();
}

} // namespace lighttex::shortcuts
