#pragma once

#include "techpulse/sources/source.hpp"

#include <chrono>
#include <string>

namespace techpulse::net {

class HttpClient {
public:
    explicit HttpClient(std::chrono::seconds connect_timeout = std::chrono::seconds{5},
                        std::chrono::seconds request_timeout = std::chrono::seconds{20});

    sources::HttpResponse get(const std::string& url) const;

private:
    std::chrono::seconds connect_timeout_;
    std::chrono::seconds request_timeout_;
};

}  // namespace techpulse::net

