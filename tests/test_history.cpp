#include "core/History.h"
#include <gtest/gtest.h>

using namespace lighttex::core;

TEST(History, EmptyHistory) {
  History h;
  EXPECT_FALSE(h.canUndo());
  EXPECT_FALSE(h.canRedo());
  EXPECT_EQ(h.undo(), std::nullopt);
  EXPECT_EQ(h.redo(), std::nullopt);
}

TEST(History, PushAndUndo) {
  History h;
  h.push({{0, 5, "hello"}});
  EXPECT_TRUE(h.canUndo());
  EXPECT_FALSE(h.canRedo());

  auto ops = h.undo();
  ASSERT_TRUE(ops.has_value());
  EXPECT_EQ(ops->size(), 1u);
  EXPECT_EQ((*ops)[0].text, "hello");
  EXPECT_FALSE(h.canUndo());
  EXPECT_TRUE(h.canRedo());
}

TEST(History, RedoAfterUndo) {
  History h;
  h.push({{0, 3, "abc"}});
  h.undo();
  EXPECT_TRUE(h.canRedo());

  auto ops = h.redo();
  ASSERT_TRUE(ops.has_value());
  EXPECT_EQ((*ops)[0].text, "abc");
  EXPECT_TRUE(h.canUndo());
  EXPECT_FALSE(h.canRedo());
}

TEST(History, NewEditClearsRedo) {
  History h;
  h.push({{0, 3, "abc"}});
  h.undo();
  EXPECT_TRUE(h.canRedo());

  h.push({{0, 3, "xyz"}});
  EXPECT_FALSE(h.canRedo()); // Redo cleared
  EXPECT_TRUE(h.canUndo());
}

TEST(History, Clear) {
  History h;
  h.push({{0, 1, "a"}});
  h.push({{1, 2, "b"}});
  EXPECT_TRUE(h.canUndo());

  h.clear();
  EXPECT_FALSE(h.canUndo());
  EXPECT_FALSE(h.canRedo());
}
