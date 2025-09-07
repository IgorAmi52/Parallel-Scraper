#include <cstring>
#include "parallel-scraper/Config.hpp"

namespace ps {
Config load_config(int argc, char** argv) {
  Config cfg;
  auto next = [&](int& i){ return (i+1<argc) ? argv[++i] : nullptr; };
  for (int i=1;i<argc;++i){
    const char* a=argv[i];
    if (!std::strcmp(a,"--urls"))         { if (auto v=next(i)) cfg.urls_path = v; }
    else if (!std::strcmp(a,"--parallelism")){ if (auto v=next(i)) cfg.parallelism = std::stoi(v); }
    else if (!std::strcmp(a,"--timeout_ms")) { if (auto v=next(i)) cfg.timeout_ms  = std::stoi(v); }
    else if (!std::strcmp(a,"--retries"))    { if (auto v=next(i)) cfg.retries     = std::stoi(v); }
    else if (!std::strcmp(a,"--out"))        { if (auto v=next(i)) cfg.out_dir     = v; }
    else if (!std::strcmp(a,"--pipeline"))   { cfg.use_pipeline = true; }
  }
  return cfg;
}
} 
