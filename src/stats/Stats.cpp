#include "parallel-scraper/Stats.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <nlohmann/json.hpp>

namespace fs = std::filesystem;

namespace ps {

Stats::Stats() {
  for (int i=0;i<6;++i) rating_hist_[i].store(0, std::memory_order_relaxed);
}

int64_t Stats::to_cents(double v) {
  double x = v * 100.0;
  if (x >= 0) return static_cast<int64_t>(x + 0.5);
  return static_cast<int64_t>(x - 0.5);
}

void Stats::on_item(const Item& it) {
  count_items_.fetch_add(1, std::memory_order_relaxed);
  if (it.rating == 5) count_5_.fetch_add(1, std::memory_order_relaxed);
  if (it.rating >=1 && it.rating <=5) rating_hist_[it.rating].fetch_add(1, std::memory_order_relaxed);

  int64_t c = to_cents(it.price);
  if (c > 0) {
    sum_price_cents_.fetch_add(static_cast<uint64_t>(c), std::memory_order_relaxed);

    int64_t cur_min = min_price_cents_.load(std::memory_order_relaxed);
    while (c < cur_min && !min_price_cents_.compare_exchange_weak(cur_min, c, std::memory_order_relaxed)) {}

    int64_t cur_max = max_price_cents_.load(std::memory_order_relaxed);
    while (c > cur_max && !max_price_cents_.compare_exchange_weak(cur_max, c, std::memory_order_relaxed)) {}

    std::lock_guard<std::mutex> g(prices_lock_);
    prices_cents_.push_back(c);
  }
}

double Stats::avg_price() const {
  uint64_t cnt = count_items_.load(std::memory_order_relaxed);
  if (!cnt) return 0.0;
  uint64_t sum = sum_price_cents_.load(std::memory_order_relaxed);
  return static_cast<double>(sum) / 100.0 / static_cast<double>(cnt);
}

void Stats::snapshot(const std::string& out_dir) const {
  fs::create_directories(out_dir);

  uint64_t cnt = count_items_.load(std::memory_order_relaxed);
  uint64_t five = count_5_.load(std::memory_order_relaxed);
  uint64_t sumc = sum_price_cents_.load(std::memory_order_relaxed);
  int64_t minc = min_price_cents_.load(std::memory_order_relaxed);
  int64_t maxc = max_price_cents_.load(std::memory_order_relaxed);

  std::vector<int64_t> prices;
  {
    std::lock_guard<std::mutex> g(prices_lock_);
    prices = prices_cents_;
  }
  std::sort(prices.begin(), prices.end());
  double p90 = 0.0;
  if (!prices.empty()) {
    size_t idx = static_cast<size_t>(prices.size() * 0.9);
    if (idx >= prices.size()) idx = prices.size()-1;
    p90 = static_cast<double>(prices[idx]) / 100.0;
  }

  nlohmann::json hist;
  for (int r=1;r<=5;++r) hist[std::to_string(r)] = rating_hist_[r].load(std::memory_order_relaxed);

  nlohmann::json j;
  j["items_total"] = cnt;
  j["five_star"] = five;
  j["avg_price"] = cnt ? (static_cast<double>(sumc)/100.0/static_cast<double>(cnt)) : 0.0;
  j["min_price"] = (minc == std::numeric_limits<int64_t>::max() ? 0.0 : static_cast<double>(minc)/100.0);
  j["max_price"] = (maxc == std::numeric_limits<int64_t>::min() ? 0.0 : static_cast<double>(maxc)/100.0);
  j["rating_hist"] = hist;
  j["p90_price"] = p90;

  std::ofstream f(out_dir + "/stats.json");
  f << j.dump(2) << std::endl;
}

} // namespace ps
