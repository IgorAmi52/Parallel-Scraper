#pragma once
#include <string>
#include <cstdint>
#include <atomic>
#include <vector>
#include <tbb/concurrent_unordered_map.h>
#include <tbb/concurrent_unordered_set.h>
#include <tbb/concurrent_vector.h>
#include <parallel-scraper/Models.hpp>

namespace ps {

  struct HttpResult;
  
  class ResultStore {
  public:
    ResultStore(std::string out_dir);

    uint64_t upsert_page(HttpResult &res);

    void add_item_for_page(uint64_t page_id, Item& it);

    bool     has_url(const std::string& url) const;
    size_t   page_count() const;
    size_t   item_count() const;

    void snapshot_to_files(double total_seconds,
                          size_t pages_ok,
                          size_t pages_err) const;

  private:
    std::atomic<uint64_t> next_page_id_{1};
    std::string out_dir_;
    tbb::concurrent_unordered_map<std::string, uint64_t> page_id_by_url_;
    tbb::concurrent_unordered_map<uint64_t, PageMeta>    pages_;
    tbb::concurrent_vector<Item>                         items_;
  };

} 
