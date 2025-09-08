#include "parallel-scraper/Orchestrator.hpp"
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/parallel_pipeline.h>
#include <atomic>

namespace ps {

    void Orchestrator::run(const std::vector<std::string>& urls) {
    if (cfg_.use_pipeline) run_pipeline(urls);
    else run_parallel(urls);
    }

    void Orchestrator::run_parallel(const std::vector<std::string>& urls) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, urls.size()),
        [&](const tbb::blocked_range<size_t>& r){
        for (size_t i=r.begin(); i<r.end(); ++i) {
            HttpResult res = http_.get(urls[i], cfg_.timeout_ms, cfg_.retries);
            if (res.status >= 200 && res.status < 300 && !res.body.empty()) {
            uint64_t page_id = store_.upsert_page(res);
            Item item = parser_.parse(res.body, res.url.empty()? urls[i] : res.url);

            stats_.on_item(item);
            store_.add_item_for_page(page_id, item);
            pages_ok_.fetch_add(1, std::memory_order_relaxed);    
            } else {
            pages_err_.fetch_add(1, std::memory_order_relaxed);
            }
        }
        }
    );
    }

    void Orchestrator::run_pipeline(const std::vector<std::string>& urls) {
    struct FetchOut { std::string url; HttpResult res; };
    struct Parsed { uint64_t page_id; Item item; };

    std::atomic<size_t> idx{0};
    auto s1 = tbb::make_filter<void, std::string>(tbb::filter_mode::serial_in_order,
        [&](tbb::flow_control& fc)->std::string{
        size_t i = idx.fetch_add(1, std::memory_order_relaxed);
        if (i >= urls.size()) { fc.stop(); return {}; }
        return urls[i];
        });

    auto s2 = tbb::make_filter<std::string, FetchOut>(tbb::filter_mode::parallel,
        [&](std::string url)->FetchOut{
        FetchOut out;
        out.url = std::move(url);
        out.res = http_.get(out.url, cfg_.timeout_ms, cfg_.retries);
        return out;
        });

    auto s3 = tbb::make_filter<FetchOut, Parsed>(tbb::filter_mode::parallel,
        [&](FetchOut in)->Parsed{
        Parsed p{0,{}};
        if (in.res.status >= 200 && in.res.status < 300 && !in.res.body.empty()) {
            p.page_id = store_.upsert_page(in.res);
            p.item = parser_.parse(in.res.body, in.res.url.empty()? in.url : in.res.url);
        }
        return p;
        });

    auto s4 = tbb::make_filter<Parsed, void>(tbb::filter_mode::parallel,
        [&](Parsed p){
        if (p.page_id != 0) {
            stats_.on_item(p.item);
            store_.add_item_for_page(p.page_id, p.item);
            pages_ok_.fetch_add(1, std::memory_order_relaxed);
        } else {
            pages_err_.fetch_add(1, std::memory_order_relaxed);
        }
        });

    tbb::parallel_pipeline((size_t)cfg_.parallelism, s1 & s2 & s3 & s4);
    }

} 