#pragma once

#include "techpulse/model/radar_item.hpp"

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace techpulse::sources {

struct HttpResponse {
    int status{};
    std::string body;
    std::string error;
    std::map<std::string, std::string> headers;
};

using HttpGet = std::function<HttpResponse(const std::string& url)>;

struct FetchResult {
    std::vector<model::RadarItem> items;
    std::vector<std::string> errors;
};

}  // namespace techpulse::sources
