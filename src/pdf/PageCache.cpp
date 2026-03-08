#include "pdf/PageCache.h"

#include <algorithm>

namespace lighttex::pdf {

PageCache::PageCache(size_t maxPages) : maxPages_(maxPages) {}

const RenderedPage* PageCache::get(int pageNum) {
    auto it = cache_.find(pageNum);
    if (it == cache_.end()) return nullptr;

    // Move to back of order (most recently used)
    auto orderIt = std::find(order_.begin(), order_.end(), pageNum);
    if (orderIt != order_.end()) {
        order_.erase(orderIt);
        order_.push_back(pageNum);
    }

    return &it->second;
}

void PageCache::insert(RenderedPage page) {
    int pageNum = page.pageNum;

    // If already cached, update and refresh LRU
    auto it = cache_.find(pageNum);
    if (it != cache_.end()) {
        it->second = std::move(page);
        auto orderIt = std::find(order_.begin(), order_.end(), pageNum);
        if (orderIt != order_.end()) {
            order_.erase(orderIt);
        }
        order_.push_back(pageNum);
        return;
    }

    // Evict LRU if at capacity
    if (cache_.size() >= maxPages_) {
        evict();
    }

    cache_.emplace(pageNum, std::move(page));
    order_.push_back(pageNum);
}

void PageCache::clear() {
    cache_.clear();
    order_.clear();
}

void PageCache::evict() {
    if (order_.empty()) return;
    int lruPage = order_.front();
    order_.erase(order_.begin());
    cache_.erase(lruPage);
}

} // namespace lighttex::pdf
