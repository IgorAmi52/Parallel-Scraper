#include <catch2/catch_all.hpp>
#include <parallel-scraper/ResultStore.hpp>
#include <parallel-scraper/HttpClient.hpp>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

static ps::HttpResult mk_http_result(std::string url, std::string body, int status=200, double ms=12.3) {
  ps::HttpResult r;
  r.body = std::move(body);
  r.status = status;
  r.bytes = static_cast<long long>(r.body.size());
  r.fetch_ms = ms;
  r.url = std::move(url);
  return r;
}

TEST_CASE("ResultStore upsert_page deduplicates by effective_url and writes HTML") {
  std::string out = "test_out";
  fs::remove_all(out);

  ps::ResultStore store(out);

  auto r1 = mk_http_result("https://example.com/a", "<html>one</html>");
  auto r2 = mk_http_result("https://example.com/a", "<html>two</html>");

  auto id1 = store.upsert_page(r1);
  auto id2 = store.upsert_page(r2);

  REQUIRE(id1 == id2); 

  fs::path p = fs::path(out) / "pages" / ("page_" + std::to_string(id1) + ".html");
  REQUIRE(fs::exists(p));
}

TEST_CASE("ResultStore add_item_for_page links item to page_id") {
  std::string out = "test_out2";
  fs::remove_all(out);
  ps::ResultStore store(out);

  auto r = mk_http_result("https://example.com/book", "<html>ok</html>");
  auto page_id = store.upsert_page(r);

  ps::Item it;
  it.title = "Book";
  it.url = "https://example.com/book";
  it.price = 12.5;
  it.rating = 5;

  store.add_item_for_page(page_id, it);
  REQUIRE(store.item_count() == 1);
  REQUIRE(store.page_count() == 1);

  fs::path p = fs::path(out) / "pages" / ("page_" + std::to_string(page_id) + ".html");
  REQUIRE(fs::exists(p));
}
