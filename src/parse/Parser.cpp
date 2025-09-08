#include <parallel-scraper/Parser.hpp>
#include <regex>
#include <algorithm>

namespace {
  static inline std::string trim(std::string s) {
    auto ws = [](unsigned char c){ return std::isspace(c); };
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [&](unsigned char c){ return !ws(c); }));
    s.erase(std::find_if(s.rbegin(), s.rend(), [&](unsigned char c){ return !ws(c); }).base(), s.end());
    return s;
  }

  static inline std::string html_entity_decode_basic(std::string s) {
    size_t p=0;
    while ((p = s.find("&amp;", p)) != std::string::npos) s.replace(p, 5, "&");
    p=0;
    while ((p = s.find("&lt;", p)) != std::string::npos) s.replace(p, 4, "<");
    p=0;
    while ((p = s.find("&gt;", p)) != std::string::npos) s.replace(p, 4, ">");
    p=0;
    while ((p = s.find("&quot;", p)) != std::string::npos) s.replace(p, 6, "\"");
    p=0;
    while ((p = s.find("&#39;", p)) != std::string::npos) s.replace(p, 5, "'");
    return s;
  }

  static inline int map_star_rating(const std::string& klass) {
    if (klass.find("Five")   != std::string::npos) return 5;
    if (klass.find("Four")   != std::string::npos) return 4;
    if (klass.find("Three")  != std::string::npos) return 3;
    if (klass.find("Two")    != std::string::npos) return 2;
    if (klass.find("One")    != std::string::npos) return 1;
    return 0;
  }
  static inline double parse_price(const std::string& s) {
    std::string t; t.reserve(s.size());
    for (char c: s) if ((c>='0'&&c<='9') || c=='.' || c==',') t.push_back(c==','?'.':c);
    try { return t.empty()?0.0:std::stod(t); } catch (...) { return 0.0; }
  }
}

namespace ps {

  Item Parser::parse(const std::string& html, const std::string& base_url) const {
    std::regex re_title_h1(R"(<h1[^>]*>(.*?)</h1>)", std::regex::icase);
    std::regex re_title_tag(R"(<title[^>]*>(.*?)</title>)", std::regex::icase);
    std::regex re_price(R"(<p[^>]*class=\"[^\"]*price_color[^\"]*\"[^>]*>\s*([^<]+)\s*</p>)", std::regex::icase);
    std::regex re_rating(R"(<p[^>]*class=\"[^\"]*star-rating\s+([A-Za-z]+)[^\"]*\"[^>]*>)", std::regex::icase);

    std::smatch m;
    std::string title;
    if (std::regex_search(html, m, re_title_h1)) title = m[1].str();
    else if (std::regex_search(html, m, re_title_tag)) title = m[1].str();
    title = trim(html_entity_decode_basic(title));

    std::string price_s;
    if (std::regex_search(html, m, re_price)) price_s = trim(m[1].str());
    double price = parse_price(price_s);

    int rating = 0;
    if (std::regex_search(html, m, re_rating)) rating = map_star_rating(m[1].str());

    Item it;
    if (!title.empty() || price > 0.0 || rating > 0) {
      it.title = title;
      it.url = base_url;
      it.rating = rating;
      it.price = price;
    }
    return it;
  }
}
