#include "compiler/SyncTeX.h"
#include <gtest/gtest.h>

using namespace lighttex::compiler;

TEST(SyncTeX, ParseForwardOutput) {
  std::string output = "SyncTeX result file:./main.synctex.gz\n"
                       "Page:1\nx:72.0\ny:720.5\n";
  auto pos = parseForwardOutput(output);
  ASSERT_TRUE(pos.has_value());
  EXPECT_EQ(pos->page, 1);
  EXPECT_NEAR(pos->x, 72.0, 0.01);
  EXPECT_NEAR(pos->y, 720.5, 0.01);
}

TEST(SyncTeX, ParseForwardOutputMissing) {
  std::string output = "SyncTeX result file:./main.synctex.gz\nPage:1\n";
  auto pos = parseForwardOutput(output);
  EXPECT_FALSE(pos.has_value());
}

TEST(SyncTeX, ParseInverseOutput) {
  std::string output = "Input:./main.tex\nLine:42\nColumn:7\n";
  auto pos = parseInverseOutput(output);
  ASSERT_TRUE(pos.has_value());
  EXPECT_EQ(pos->file, "./main.tex");
  EXPECT_EQ(pos->line, 42);
  EXPECT_EQ(pos->column, 7);
}

TEST(SyncTeX, ParseInverseOutputNoColumn) {
  std::string output = "Input:./main.tex\nLine:10\n";
  auto pos = parseInverseOutput(output);
  ASSERT_TRUE(pos.has_value());
  EXPECT_EQ(pos->file, "./main.tex");
  EXPECT_EQ(pos->line, 10);
  EXPECT_EQ(pos->column, 0);
}

TEST(SyncTeX, NotLoaded) {
  SyncTeX sync;
  EXPECT_FALSE(sync.isLoaded());
}
