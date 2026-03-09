#include "core/Selection.h"
#include <gtest/gtest.h>

using namespace lighttex::core;

TEST(Selection, Cursor) {
  Selection sel = Selection::cursor(5);
  EXPECT_TRUE(sel.isCursor());
  EXPECT_TRUE(sel.isEmpty());
  EXPECT_EQ(sel.start(), 5u);
  EXPECT_EQ(sel.end(), 5u);
  EXPECT_EQ(sel.length(), 0u);
}

TEST(Selection, Range) {
  Selection sel = Selection::range(2, 8);
  EXPECT_FALSE(sel.isCursor());
  EXPECT_EQ(sel.start(), 2u);
  EXPECT_EQ(sel.end(), 8u);
  EXPECT_EQ(sel.length(), 6u);
}

TEST(Selection, BackwardSelection) {
  Selection sel = Selection::range(8, 2);
  EXPECT_EQ(sel.start(), 2u);
  EXPECT_EQ(sel.end(), 8u);
  EXPECT_EQ(sel.length(), 6u);
  EXPECT_EQ(sel.anchor, 8u);
  EXPECT_EQ(sel.head, 2u);
}

TEST(Selection, Contains) {
  Selection sel = Selection::range(3, 7);
  EXPECT_TRUE(sel.contains(3));
  EXPECT_TRUE(sel.contains(5));
  EXPECT_TRUE(sel.contains(6));
  EXPECT_FALSE(sel.contains(7)); // exclusive end
  EXPECT_FALSE(sel.contains(2));
  EXPECT_FALSE(sel.contains(10));
}

TEST(Selection, Overlaps) {
  Selection a = Selection::range(2, 6);
  Selection b = Selection::range(4, 8);
  Selection c = Selection::range(7, 10);

  EXPECT_TRUE(a.overlaps(b));
  EXPECT_TRUE(b.overlaps(a));
  EXPECT_FALSE(a.overlaps(c));
  EXPECT_TRUE(b.overlaps(c));
}

TEST(Selection, NormalizeSelections) {
  std::vector<Selection> sels = {
      Selection::range(5, 10),
      Selection::range(0, 6), // overlaps with first after sort
      Selection::range(15, 20),
  };

  normalizeSelections(sels);
  EXPECT_EQ(sels.size(), 2u);
  EXPECT_EQ(sels[0].start(), 0u);
  EXPECT_EQ(sels[0].end(), 10u);
  EXPECT_EQ(sels[1].start(), 15u);
  EXPECT_EQ(sels[1].end(), 20u);
}

TEST(Selection, Clip) {
  Selection sel = Selection::range(5, 100);
  Selection clipped = sel.clip(20);
  EXPECT_EQ(clipped.anchor, 5u);
  EXPECT_EQ(clipped.head, 20u);

  Selection sel2 = Selection::range(50, 100);
  Selection clipped2 = sel2.clip(20);
  EXPECT_EQ(clipped2.anchor, 20u);
  EXPECT_EQ(clipped2.head, 20u);
}
