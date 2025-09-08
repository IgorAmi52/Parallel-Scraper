#include <catch2/catch_all.hpp>
#include <parallel-scraper/Stats.hpp>

TEST_CASE("Stats accumulates counts, histogram and averages") {
  ps::Stats s;

  ps::Item a; a.title="A"; a.url="u/a"; a.price=10.0; a.rating=5;
  ps::Item b; b.title="B"; b.url="u/b"; b.price=20.0; b.rating=3;
  ps::Item c; c.title="C"; c.url="u/c"; c.price=0.0;  c.rating=0; 

  s.on_item(a);
  s.on_item(b);
  s.on_item(c);

  REQUIRE(s.avg_price() == Catch::Approx(10.0).margin(1e-6));
}
