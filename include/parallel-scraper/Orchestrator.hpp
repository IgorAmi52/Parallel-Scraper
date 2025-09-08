#pragma once
#include <vector>
#include <string>
#include <atomic>
#include "parallel-scraper/Config.hpp"
#include "parallel-scraper/HttpClient.hpp"
#include "parallel-scraper/Parser.hpp"
#include "parallel-scraper/ResultStore.hpp"
#include "parallel-scraper/Stats.hpp"

namespace ps {

class Orchestrator {
public:
  Orchestrator(const Config& cfg, HttpClient& http, Parser& parser, ResultStore& store, Stats& stats)
    : cfg_(cfg), http_(http), parser_(parser), store_(store), stats_(stats) {};
  void run(const std::vector<std::string>& urls);
  void run_parallel(const std::vector<std::string>& urls);
  void run_pipeline(const std::vector<std::string>& urls);
  size_t pages_ok() const { return pages_ok_.load(); }
  size_t pages_err() const { return pages_err_.load(); }

private:
  const Config& cfg_;
  HttpClient& http_;
  Parser& parser_;
  ResultStore& store_;
  Stats& stats_;
  std::atomic<size_t> pages_ok_{0};
  std::atomic<size_t> pages_err_{0};
};

} 
