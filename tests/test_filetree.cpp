#include "ui/FileTreeWidget.h"

#include <QApplication>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <gtest/gtest.h>

using namespace lighttex::ui;

class FileTreeTest : public ::testing::Test {
protected:
  void SetUp() override {
    if (!QApplication::instance()) {
      int argc = 0;
      app_ = new QApplication(argc, nullptr);
    }
    tree_ = new FileTreeWidget();
  }

  void TearDown() override { delete tree_; }

  QApplication *app_ = nullptr;
  FileTreeWidget *tree_ = nullptr;
};

TEST_F(FileTreeTest, InitiallyValid) { EXPECT_NE(tree_->model(), nullptr); }

TEST_F(FileTreeTest, SetRootPathDoesNotCrash) {
  QTemporaryDir tmpDir;
  ASSERT_TRUE(tmpDir.isValid());
  tree_->setRootPath(tmpDir.path());
  EXPECT_TRUE(true);
}

TEST_F(FileTreeTest, ThemeDoesNotCrash) {
  lighttex::theme::Theme theme;
  theme.ui.sidebarBg = "#252526";
  theme.ui.sidebarFg = "#cccccc";
  theme.colors.selection = "#264f78";
  tree_->setTheme(theme);
  EXPECT_TRUE(true);
}

TEST_F(FileTreeTest, FileActivatedSignalExists) {
  QSignalSpy spy(tree_, &FileTreeWidget::fileActivated);
  EXPECT_TRUE(spy.isValid());
}

TEST_F(FileTreeTest, HeaderIsHidden) { EXPECT_TRUE(tree_->isHeaderHidden()); }
