#include "pdf/PageCache.h"
#include <gtest/gtest.h>

using namespace lighttex::pdf;

static RenderedPage makePage(int num) {
  return {num, 100, 100, QImage(100, 100, QImage::Format_ARGB32)};
}

TEST(PageCache, EmptyCache) {
  PageCache cache(3);
  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.size(), 0u);
  EXPECT_EQ(cache.get(0), nullptr);
}

TEST(PageCache, InsertAndGet) {
  PageCache cache(3);
  cache.insert(makePage(0));
  EXPECT_EQ(cache.size(), 1u);

  auto *page = cache.get(0);
  ASSERT_NE(page, nullptr);
  EXPECT_EQ(page->pageNum, 0);
  EXPECT_EQ(page->width, 100);
}

TEST(PageCache, LruEviction) {
  PageCache cache(3);
  cache.insert(makePage(0));
  cache.insert(makePage(1));
  cache.insert(makePage(2));
  EXPECT_EQ(cache.size(), 3u);

  // Insert 4th page, should evict page 0 (least recently used)
  cache.insert(makePage(3));
  EXPECT_EQ(cache.size(), 3u);
  EXPECT_EQ(cache.get(0), nullptr); // evicted
  EXPECT_NE(cache.get(1), nullptr);
  EXPECT_NE(cache.get(2), nullptr);
  EXPECT_NE(cache.get(3), nullptr);
}

TEST(PageCache, AccessRefreshesLru) {
  PageCache cache(3);
  cache.insert(makePage(0));
  cache.insert(makePage(1));
  cache.insert(makePage(2));

  // Access page 0 to refresh it
  cache.get(0);

  // Insert page 3 — should evict page 1 (now least recently used)
  cache.insert(makePage(3));
  EXPECT_NE(cache.get(0), nullptr); // still here (refreshed)
  EXPECT_EQ(cache.get(1), nullptr); // evicted
  EXPECT_NE(cache.get(2), nullptr);
  EXPECT_NE(cache.get(3), nullptr);
}

TEST(PageCache, Clear) {
  PageCache cache(3);
  cache.insert(makePage(0));
  cache.insert(makePage(1));
  EXPECT_EQ(cache.size(), 2u);

  cache.clear();
  EXPECT_TRUE(cache.empty());
  EXPECT_EQ(cache.get(0), nullptr);
}
