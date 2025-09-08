#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <parallel-scraper/HttpClient.hpp>
#include "parallel-scraper/ResultStore.hpp"

namespace fs = std::filesystem;

namespace ps {

ResultStore::ResultStore(std::string out_dir) {
    out_dir = out_dir.empty() ? "out" : out_dir;
    fs::create_directories(out_dir + "/pages");
}

uint64_t ResultStore::upsert_page(ps::HttpResult &res) {
  auto [it, inserted] = page_id_by_url_.insert({res.url, 0});
  if (!inserted) {
    return it->second;
  }

  const uint64_t id = next_page_id_.fetch_add(1, std::memory_order_relaxed);
  it->second = id;

  std::string file_path = (fs::path("out/pages") / ("page_" + std::to_string(id) + ".html")).string();
  std::ofstream out(file_path, std::ios::binary);
  out.write(res.body.data(), static_cast<std::streamsize>(res.body.size()));

  PageMeta meta;
  meta.id       = id;
  meta.url      = res.url;
  meta.status   = res.status;
  meta.bytes    = res.bytes;
  meta.fetch_ms = res.fetch_ms;
  meta.file_path= file_path;

  pages_.insert({id, std::move(meta)});
  return id;
}

void ResultStore::add_item_for_page(uint64_t page_id, Item& item) {
  item.id = page_id;
  items_.push_back(std::move(item));
}

bool ResultStore::has_url(const std::string& url) const {
  return page_id_by_url_.find(url) != page_id_by_url_.end();
}

size_t ResultStore::page_count() const {
  return pages_.size();
}

size_t ResultStore::item_count() const {
  return items_.size();
}

void ResultStore::snapshot_to_files(double total_seconds,
                                    size_t pages_ok,
                                    size_t pages_err) const {
  fs::create_directories(out_dir_);

  {
    nlohmann::json jr = nlohmann::json::array();
    for (const auto& it : items_) {
      jr.push_back({
        {"title",   it.title},
        {"url",     it.url},
        {"rating",  it.rating},
        {"price",   it.price},
        {"page_id", it.id}
      });
    }
    std::ofstream f(out_dir_ + "/results.json");
    f << jr.dump(2) << std::endl;
  }

  {
    nlohmann::json jp = nlohmann::json::array();
    for (const auto& kv : pages_) {
      const auto& m = kv.second;
      jp.push_back({
        {"id",        m.id},
        {"url",       m.url},
        {"status",    m.status},
        {"bytes",     m.bytes},
        {"fetch_ms",  m.fetch_ms},
        {"file_path", m.file_path}
      });
    }
    std::ofstream f(out_dir_ + "/pages.json");
    f << jp.dump(2) << std::endl;
  }

  {
    std::ofstream f(out_dir_ + "/summary.txt");
    f << "Pages OK: "  << pages_ok  << "\n";
    f << "Pages ERR: " << pages_err << "\n";
    f << "Unique URLs (pages): " << page_count() << "\n";
    f << "Items: "      << item_count() << "\n";
    f << "Time (s): "   << total_seconds << "\n";
    double thr = (total_seconds > 0.0) ? (double)pages_ok / total_seconds : 0.0;
    f << "Throughput (pages/s): " << thr << "\n";
  }
}

} 
