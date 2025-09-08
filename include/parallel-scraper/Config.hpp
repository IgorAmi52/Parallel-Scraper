#pragma once
#include <string>

namespace ps {
struct Config {
  int parallelism = 32;
  int timeout_ms = 3000;
  int retries = 2;
  bool use_pipeline = false;
  std::string out_dir = "out";
  std::string urls_path = "data/urls.txt";
};

Config load_config(int argc, char** argv);
} 
