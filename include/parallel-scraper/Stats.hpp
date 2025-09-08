#pragma once
#include <cstdint>
#include <vector>
#include <atomic>
#include <mutex>
#include <string>
#include "parallel-scraper/Models.hpp"

namespace ps {

class Stats {
public:
  Stats();
  void on_item(const Item& it);
  double avg_price() const;
  void snapshot(const std::string& out_dir) const;

private:
  static int64_t to_cents(double v);

  std::atomic<uint64_t> count_items_{0};
  std::atomic<uint64_t> count_5_{0};
  std::atomic<uint64_t> sum_price_cents_{0};
  std::atomic<int64_t> min_price_cents_{std::numeric_limits<int64_t>::max()};
  std::atomic<int64_t> max_price_cents_{std::numeric_limits<int64_t>::min()};
  std::atomic<uint64_t> rating_hist_[6];
  mutable std::mutex prices_lock_;
  std::vector<int64_t> prices_cents_;
};

} 
