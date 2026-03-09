#include "shortcuts/Shortcuts.h"

#include <QApplication>
#include <gtest/gtest.h>

using namespace lighttex::shortcuts;

class ShortcutManagerTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (!QApplication::instance()) {
      int argc = 0;
      app_ = new QApplication(argc, nullptr);
    }
  }

  QApplication *app_ = nullptr;
};

TEST_F(ShortcutManagerTest, DefaultBindingsLoaded) {
  ShortcutManager mgr;
  EXPECT_TRUE(mgr.hasBinding("file.open"));
  EXPECT_TRUE(mgr.hasBinding("file.save"));
  EXPECT_TRUE(mgr.hasBinding("compile"));
  EXPECT_TRUE(mgr.hasBinding("palette"));
  EXPECT_TRUE(mgr.hasBinding("find"));
  EXPECT_TRUE(mgr.hasBinding("find_replace"));
  EXPECT_TRUE(mgr.hasBinding("toggle_filetree"));
  EXPECT_TRUE(mgr.hasBinding("goto_definition"));
}

TEST_F(ShortcutManagerTest, DefaultBindingsNotEmpty) {
  ShortcutManager mgr;
  EXPECT_FALSE(mgr.keySequence("file.open").isEmpty());
  EXPECT_FALSE(mgr.keySequence("compile").isEmpty());
  EXPECT_FALSE(mgr.keySequence("palette").isEmpty());
}

TEST_F(ShortcutManagerTest, UnknownBindingReturnsEmpty) {
  ShortcutManager mgr;
  EXPECT_TRUE(mgr.keySequence("nonexistent.action").isEmpty());
  EXPECT_FALSE(mgr.hasBinding("nonexistent.action"));
}

TEST_F(ShortcutManagerTest, SetBinding) {
  ShortcutManager mgr;
  QKeySequence custom(Qt::CTRL | Qt::Key_K);
  mgr.setBinding("custom.action", custom);
  EXPECT_TRUE(mgr.hasBinding("custom.action"));
  EXPECT_EQ(mgr.keySequence("custom.action"), custom);
}

TEST_F(ShortcutManagerTest, OverrideDefaultBinding) {
  ShortcutManager mgr;
  QKeySequence original = mgr.keySequence("compile");
  QKeySequence custom(Qt::CTRL | Qt::SHIFT | Qt::Key_B);
  mgr.setBinding("compile", custom);
  EXPECT_EQ(mgr.keySequence("compile"), custom);
  EXPECT_NE(mgr.keySequence("compile"), original);
}

TEST_F(ShortcutManagerTest, LoadFromInvalidFileReturnsFalse) {
  ShortcutManager mgr;
  EXPECT_FALSE(mgr.loadFromFile("/nonexistent/path/keybindings.toml"));
}

TEST_F(ShortcutManagerTest, KeySequenceStringNotEmpty) {
  ShortcutManager mgr;
  QString str = mgr.keySequenceString("file.open");
  EXPECT_FALSE(str.isEmpty());
}

TEST_F(ShortcutManagerTest, BindingsMapContainsDefaults) {
  ShortcutManager mgr;
  auto &bindings = mgr.bindings();
  EXPECT_GE(bindings.size(), 8u);
}
