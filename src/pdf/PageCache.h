#pragma once

#include "pdf/PdfRenderer.h"

#include <optional>
#include <unordered_map>
#include <vector>

namespace lighttex::pdf {

class PageCache {
public:
  explicit PageCache(size_t maxPages = 10);

  const RenderedPage *get(int pageNum);
  void insert(RenderedPage page);
  void clear();

  [[nodiscard]] size_t size() const { return cache_.size(); }
  [[nodiscard]] bool empty() const { return cache_.empty(); }

private:
  void evict();

  size_t maxPages_;
  std::unordered_map<int, RenderedPage> cache_;
  std::vector<int> order_; // LRU order: front = least recent
};

} // namespace lighttex::pdf
