#pragma once
#include <string>
#include <vector>
#include <parallel-scraper/Models.hpp>

namespace ps {
  
  class Parser {
  public:
    Parser() = default;
    ps::Item parse(const std::string& html, const std::string& base_url) const;
  };
} 
