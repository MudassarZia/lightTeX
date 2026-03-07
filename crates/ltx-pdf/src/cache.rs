use std::collections::HashMap;

use crate::renderer::RenderedPage;

/// LRU page cache for rendered PDF pages.
pub struct PageCache {
    cache: HashMap<usize, RenderedPage>,
    order: Vec<usize>,
    max_pages: usize,
}

impl PageCache {
    pub fn new(max_pages: usize) -> Self {
        Self {
            cache: HashMap::new(),
            order: Vec::new(),
            max_pages,
        }
    }

    /// Get a cached page.
    pub fn get(&mut self, page_num: usize) -> Option<&RenderedPage> {
        if self.cache.contains_key(&page_num) {
            // Move to front (most recently used)
            self.order.retain(|&p| p != page_num);
            self.order.push(page_num);
            self.cache.get(&page_num)
        } else {
            None
        }
    }

    /// Insert a rendered page into the cache.
    pub fn insert(&mut self, page: RenderedPage) {
        let page_num = page.page_num;

        // Evict if at capacity
        if self.cache.len() >= self.max_pages
            && !self.cache.contains_key(&page_num)
            && let Some(evicted) = self.order.first().copied()
        {
            self.order.remove(0);
            self.cache.remove(&evicted);
        }

        self.order.retain(|&p| p != page_num);
        self.order.push(page_num);
        self.cache.insert(page_num, page);
    }

    /// Clear the entire cache.
    pub fn clear(&mut self) {
        self.cache.clear();
        self.order.clear();
    }

    /// Number of cached pages.
    pub fn len(&self) -> usize {
        self.cache.len()
    }

    pub fn is_empty(&self) -> bool {
        self.cache.is_empty()
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn make_page(num: usize) -> RenderedPage {
        RenderedPage {
            page_num: num,
            width: 100,
            height: 100,
            data_base64: String::new(),
        }
    }

    #[test]
    fn test_empty_cache() {
        let mut cache = PageCache::new(10);
        assert!(cache.is_empty());
        assert!(cache.get(0).is_none());
    }

    #[test]
    fn test_insert_and_get() {
        let mut cache = PageCache::new(10);
        cache.insert(make_page(0));
        assert_eq!(cache.len(), 1);
        assert!(cache.get(0).is_some());
        assert_eq!(cache.get(0).unwrap().page_num, 0);
    }

    #[test]
    fn test_lru_eviction() {
        let mut cache = PageCache::new(3);
        cache.insert(make_page(0));
        cache.insert(make_page(1));
        cache.insert(make_page(2));
        // Cache full, inserting page 3 should evict page 0
        cache.insert(make_page(3));
        assert_eq!(cache.len(), 3);
        assert!(cache.get(0).is_none());
        assert!(cache.get(3).is_some());
    }

    #[test]
    fn test_access_refreshes_lru() {
        let mut cache = PageCache::new(3);
        cache.insert(make_page(0));
        cache.insert(make_page(1));
        cache.insert(make_page(2));
        // Access page 0 to make it recently used
        cache.get(0);
        // Insert page 3, should evict page 1 (least recently used)
        cache.insert(make_page(3));
        assert!(cache.get(0).is_some());
        assert!(cache.get(1).is_none());
    }

    #[test]
    fn test_clear() {
        let mut cache = PageCache::new(10);
        cache.insert(make_page(0));
        cache.insert(make_page(1));
        cache.clear();
        assert!(cache.is_empty());
    }
}
