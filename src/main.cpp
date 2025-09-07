#include <tbb/parallel_for.h>
#include <iostream>
#include <vector>
#include <parallel-scraper/Config.hpp>
#include <parallel-scraper/HttpClient.hpp>
#include <parallel-scraper/FileIO.hpp>
#include <curl/curl.h>
int main(int argc, char** argv) {
    setup_env();

    ps::Config config = ps::load_config(argc, argv);
    std::vector<std::string> urls = ps::read_lines(config.urls_path);
    ps::HttpClient client;
    std::string response = client.get(urls[0], config.timeout_ms, config.retries);
    std::cout << "Response from " << urls[0] << ":\n" << response << std::endl;


    cleanup_env();
    return 0;
}


void setup_env(){
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
void cleanup_env(){
    curl_global_cleanup();
}
//cmake --build build -j
