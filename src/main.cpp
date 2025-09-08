#include <tbb/parallel_for.h>
#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <parallel-scraper/Config.hpp>
#include <parallel-scraper/HttpClient.hpp>
#include <parallel-scraper/FileIO.hpp>
#include <parallel-scraper/Parser.hpp>
#include <parallel-scraper/ResultStore.hpp>
#include <parallel-scraper/Orchestrator.hpp>
#include <tbb/global_control.h>

void setup_env(int tnum);
void cleanup_env();

int main(int argc, char** argv) {
    ps::Config config = ps::load_config(argc, argv);
    std::vector<std::string> urls = ps::read_lines(config.urls_path);

    setup_env(config.parallelism);

    ps::HttpClient client;
    ps::Parser parser;
    ps::ResultStore store(config.out_dir);
    ps::Stats stats;
    ps::Orchestrator orch(config, client, parser, store, stats);

    auto start = std::chrono::steady_clock::now();
    orch.run(urls);

    auto end = std::chrono::steady_clock::now();
    double elapsed = std::chrono::duration<double>(end - start).count();
    elapsed = std::round(elapsed * 100.0) / 100.0;

    store.snapshot_to_files(elapsed, orch.pages_ok(), orch.pages_err());
    stats.snapshot(config.out_dir);

    cleanup_env();
    return 0;
}


void setup_env(int tnum){
    curl_global_init(CURL_GLOBAL_DEFAULT);
    if (tnum <= 0) tnum = std::thread::hardware_concurrency();
    tbb::global_control gc(tbb::global_control::max_allowed_parallelism, tnum);

}
void cleanup_env(){
    curl_global_cleanup();
}
/*
rm -rf build
mkdir build
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
*/
