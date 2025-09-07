#include <string>

namespace ps{
    class HttpClient {
    public:
        HttpClient();
        ~HttpClient();

        std::string get(const std::string& url, int timeout_ms, int retry_count) const;
    };
}
