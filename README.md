# Parallel Scraper


Parallel Scraper is a high-performance web scraping tool developed in **C++20**, leveraging **Intel TBB** for scalable concurrency and **libcurl** for robust HTTP communication.

The application is designed to efficiently extract data from the [Books to Scrape](https://books.toscrape.com) dataset. It performs parallel downloading of book pages, parses essential metadata (title, price, rating), and stores the results in structured JSON format.

---

## 📑 Table of Contents

* [Features](#features)
* [Technical Overview](#technical-overview)
* [Prerequisites](#prerequisites)
* [Build](#build)
* [Run](#run)
* [Outputs](#outputs)
* [Benchmark](#benchmark)
* [Analysis & Conclusion](#analysis--conclusion)
* [License](#license)

## Features

* Parallel scraping with two modes:

  * **Pipeline mode** (`tbb::parallel_pipeline`)
  * **Parallel-for mode** (`tbb::parallel_for`)
* HTML parsing via regex.
* Deduplication of pages (`ResultStore::upsert_page`).
* Aggregated statistics (`Stats`) including:

  * count of 5★ books
  * average price
  * min/max price
  * rating histogram
  * P90 price
* Outputs in **JSON** (`results.json`, `pages.json`, `stats.json`) and plain text (`summary.txt`).

---

<!-- Project structure removed as requested -->

## Technical Overview

* **HttpClient** — wraps libcurl, returns `HttpResult` with status, body, and effective URL.
* **Parser** — extracts title, price, and rating from HTML into `Item`.
* **ResultStore** — stores raw HTML pages, item metadata, and writes outputs.
* **Stats** — computes global statistics from all parsed items.
* **Orchestrator** — coordinates scraping:

  * `run_parallel` → parallel\_for loop over URLs
  * `run_pipeline` → 4-stage pipeline (generator → fetch → parse → store)

Concurrency is controlled via:

* `tbb::global_control` — caps max number of threads to either `hardware_concurrency()` or the value specified by `--parallelism`.
* Pipeline tokens — limit active requests (`--parallelism`).

---

## Prerequisites

* CMake ≥ 3.16
* C++20 compiler (clang++, g++, MSVC)
* Libraries:

  * [oneTBB](https://github.com/oneapi-src/oneTBB)
  * [libcurl](https://curl.se/libcurl/)

  * [nlohmann/json](https://github.com/nlohmann/json)


### macOS (Homebrew)

```bash
brew install tbb curl nlohmann-json
```

### Windows (vcpkg)

```bash
vcpkg install tbb curl nlohmann-json
```

---

## Build

```bash
# 1. clean build dir
rm -rf build && mkdir build

# 2. configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 3. build
cmake --build build -j
```

---

## Run

1. Prepare `data/urls.txt` with product pages:

   ```txt
   https://books.toscrape.com/catalogue/a-light-in-the-attic_1000/index.html
   https://books.toscrape.com/catalogue/sharp-objects_997/index.html
   ...
   ```

   (use provided Python script in `tools/` to auto-fetch 100 URLs)

2. Run scraper:

   ```bash
   ./build/scraper --urls data/urls.txt --parallelism 8 --timeout_ms 3000 --retries 2 --out out --use-pipeline
  Options:

  * `--urls <path>` → file with URLs (default: `data/urls.txt`)
  * `--parallelism <N>` → max parallel requests (default: 32)
  * `--timeout_ms <N>` → request timeout in ms (default: 3000)
  * `--retries <N>` → retries on failure (default: 2)
  * `--out <dir>` → output directory (default: `out`)
  * `--pipeline` → enable pipeline mode (default: off)

---

## Outputs

* `out/pages/page_X.html` → raw HTML pages
* `out/results.json` → parsed items
* `out/pages.json` → metadata about fetched pages
* `out/summary.txt` → summary report (counts, throughput)
* `out/stats.json` → statistics (avg, histogram, p90, etc.)

---


## Benchmark

| Threads | Mode          | Time (s) | Throughput (pages/s) |
| ------- | ------------- | -------- | -------------------- |
| 4       | parallel_for  | 4.18     | 16.99                |
| 4       | pipeline      | 11.37    | 6.24                 |
| 8       | parallel_for  | 5.72     | 12.41                |
| 8       | pipeline      | 5.65     | 12.57                |
| 16      | parallel_for  | 4.46     | 15.92                |
| 16      | pipeline      | 5.06     | 14.03                |
| 32      | parallel_for  | 4.14     | 17.15                |
| 32      | pipeline      | 4.11     | 17.27                |

---

## Analysis & Conclusion

**Analysis:**

With 4 threads, `parallel_for` is significantly faster than pipeline mode, likely due to pipeline overhead dominating when the number of tokens is low. From 8 threads and above, the performance of `parallel_for` and pipeline mode converges, and throughput stabilizes. At 32 threads, both modes reach ~17 pages/s.

**Conclusion:**

For a small number of threads, `parallel_for` yields better results due to lower overhead. As the thread count increases, the difference between the modes diminishes—both can saturate network and CPU resources. For larger datasets (thousands of pages), pipeline mode would scale better for I/O-heavy workloads, while `parallel_for` can finish smaller batches more quickly.

**Additional Notes:**

- The configuration is flexible and can be adjusted via command-line options or by modifying defaults in `Config.hpp`.
- The scraper is designed for extensibility; additional parsing logic or output formats can be added with minimal changes.
- Error handling and retries are built-in, making the tool robust for real-world scraping tasks.

---


## License

MIT — feel free to fork and adapt.
