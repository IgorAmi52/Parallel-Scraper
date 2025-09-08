#include <tbb/parallel_for.h>
#include <iostream>
#include <vector>
#include <curl/curl.h>
#include <parallel-scraper/Config.hpp>
#include <parallel-scraper/HttpClient.hpp>
#include <parallel-scraper/FileIO.hpp>
#include <parallel-scraper/Parser.hpp>
#include <parallel-scraper/ResultStore.hpp>

void setup_env();
void cleanup_env();

int main(int argc, char** argv) {
    setup_env();

    ps::Config config = ps::load_config(argc, argv);
    std::vector<std::string> urls = ps::read_lines(config.urls_path);
    ps::HttpClient client;
    ps::HttpResult response = client.get(urls[0], config.timeout_ms, config.retries);
    ps::Parser parser;
    ps::ResultStore store(config.out_dir);
    ps::Item item = parser.parse(response.body, response.url);
    store.add_item_for_page(store.upsert_page(response), item);
    std::cout << "Fetched " << store.page_count() << " pages, extracted " << store.item_count() << " items." << std::endl;
    cleanup_env();
    return 0;
}


void setup_env(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
void cleanup_env(){
    curl_global_cleanup();
}
/*
# 1. napravi clean build folder
rm -rf build
mkdir build

# 2. pripremi (configure + generate)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 3. build (kompajliraj i linkuj)
cmake --build build -j
*/
