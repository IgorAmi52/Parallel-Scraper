#pragma once
#include <cstdint>
#include <string>

namespace ps {
  struct PageMeta {
    uint64_t     id        = 0;
    std::string  url;
    int          status    = 0;
    long long    bytes     = 0;
    double       fetch_ms  = 0.0;
    std::string  file_path;
  };

  struct Item {
    uint64_t     id        = 0;
    std::string  title;
    std::string  url;
    int          rating    = 0;
    double       price     = 0.0;
  };
}
