#pragma once
#include <string>

namespace ps {

struct HttpResult {
  std::string body;
  int status = 0;
  long long bytes = 0;
  double fetch_ms = 0.0;
  std::string url;
};

class HttpClient {
public:
  HttpClient();
  ~HttpClient();
  HttpResult get(const std::string& url, int timeout_ms, int retry_count) const;
};

} 
