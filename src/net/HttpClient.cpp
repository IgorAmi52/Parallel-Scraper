#include <parallel-scraper/HttpClient.hpp>
#include <curl/curl.h>
#include <iostream>
#include <thread>

ps::HttpClient::HttpClient() {}
ps::HttpClient::~HttpClient() {}

std::string ps::HttpClient::get(const std::string& url, int timeout_ms, int retry_count) const {
    std::string response;
    if (url.empty()) return response;

    size_t(*write_callback)(char*, size_t, size_t, void*) = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        auto* resp = static_cast<std::string*>(userdata);
        resp->append(ptr, size * nmemb);
        return size * nmemb;
    };  

    for (int attempt = 0; attempt <= retry_count; ++attempt) {
            CURL* curl = curl_easy_init();
            if (!curl) break;

            response.clear();
            char errbuf[CURL_ERROR_SIZE] = {0};

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

            CURLcode res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);

            if (res == CURLE_OK) return response;

            std::cerr << "Attempt " << (attempt + 1)
                    << " failed: " << (errbuf[0] ? errbuf : curl_easy_strerror(res)) << "\n";
        }
}
