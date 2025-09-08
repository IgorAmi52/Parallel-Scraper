#include <catch2/catch_all.hpp>
#include <parallel-scraper/Parser.hpp>
#include <fstream>
#include <sstream>
#include <string>

static std::string load_file(const std::string& path) {
  std::ifstream f(path, std::ios::binary);
  INFO("Missing fixture HTML file at: " << path);
  REQUIRE(f.is_open());
  std::ostringstream oss; oss << f.rdbuf();
  return oss.str();
}

TEST_CASE("Parser extracts title, price, rating") {
  ps::Parser p;
  auto html = load_file("tests/data/sample_product.html");
  auto item = p.parse(html, "https://books.toscrape.com/catalogue/sharp-objects_997/index.html");

  REQUIRE(item.url == "https://books.toscrape.com/catalogue/sharp-objects_997/index.html");
  REQUIRE(item.title == "Sharp Objects");
  REQUIRE(item.price == Catch::Approx(51.77).margin(1e-6));
  REQUIRE(item.rating == 4);
}
