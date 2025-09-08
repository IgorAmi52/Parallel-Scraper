#include <parallel-scraper/HttpClient.hpp>
#include <curl/curl.h>
#include <iostream>
#include <thread>

ps::HttpClient::HttpClient() {}
ps::HttpClient::~HttpClient() {}

ps::HttpResult ps::HttpClient::get(const std::string& url, int timeout_ms, int retry_count) const {
    HttpResult r;
    if (url.empty()) return r;

    size_t(*write_callback)(char*, size_t, size_t, void*) = [](char* ptr, size_t size, size_t nmemb, void* userdata) -> size_t {
        auto* resp = static_cast<std::string*>(userdata);
        resp->append(ptr, size * nmemb);
        return size * nmemb;
    };  

    for (int attempt = 0; attempt <= retry_count; ++attempt) {
            CURL* curl = curl_easy_init();
            if (!curl) break;

            r.body.clear();
            char errbuf[CURL_ERROR_SIZE] = {0};

            curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errbuf);
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, timeout_ms);

            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &r.body);

            auto t0 = std::chrono::steady_clock::now();
            CURLcode res = curl_easy_perform(curl);
            auto t1 = std::chrono::steady_clock::now();

            r.fetch_ms = std::chrono::duration<double,std::milli>(t1 - t0).count();

            if (res == CURLE_OK) {
                long code = 0;
                char* eff = nullptr;
                curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &code);
                curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &eff);
                r.status = (int)code;
                r.bytes = (long long)r.body.size();
                r.url = eff ? std::string(eff) : url;
                curl_easy_cleanup(curl);
                return r;
            }

            curl_easy_cleanup(curl);
            std::this_thread::sleep_for(std::chrono::milliseconds(50 * (attempt + 1)));
        }
        return r;
}
