#include "techpulse/normalize/normalize.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <string_view>
#include <utility>
#include <vector>

namespace techpulse::normalize {
namespace {

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
}

bool is_tracking_parameter(std::string_view key) {
    const auto lower = lowercase(std::string(key));
    return lower.rfind("utm_", 0) == 0 || lower == "fbclid" || lower == "gclid" ||
           lower == "mc_cid" || lower == "mc_eid" || lower == "ref" || lower == "ref_src";
}

std::string normalized_query(const std::string& query) {
    std::vector<std::pair<std::string, std::string>> retained;
    std::size_t start = 0;
    while (start <= query.size()) {
        const auto end = query.find('&', start);
        const auto part = query.substr(start, end == std::string::npos ? std::string::npos : end - start);
        const auto equals = part.find('=');
        const auto key = part.substr(0, equals);
        if (!key.empty() && !is_tracking_parameter(key)) retained.emplace_back(key, equals == std::string::npos ? "" : part.substr(equals + 1));
        if (end == std::string::npos) break;
        start = end + 1;
    }
    std::sort(retained.begin(), retained.end());
    std::ostringstream output;
    for (std::size_t i = 0; i < retained.size(); ++i) {
        if (i != 0) output << '&';
        output << retained[i].first;
        if (!retained[i].second.empty()) output << '=' << retained[i].second;
    }
    return output.str();
}

}  // namespace

std::string canonical_url(const std::string& input) {
    const auto scheme_end = input.find("://");
    if (scheme_end == std::string::npos) return input;

    const auto scheme = lowercase(input.substr(0, scheme_end));
    const auto authority_start = scheme_end + 3;
    const auto path_start = input.find_first_of("/?#", authority_start);
    const auto host = lowercase(input.substr(authority_start, path_start == std::string::npos ? std::string::npos : path_start - authority_start));
    if (host.empty()) return input;

    const auto suffix = path_start == std::string::npos ? "" : input.substr(path_start);
    const auto fragment = suffix.find('#');
    const auto without_fragment = suffix.substr(0, fragment);
    const auto query_start = without_fragment.find('?');
    std::string path = query_start == std::string::npos ? without_fragment : without_fragment.substr(0, query_start);
    const auto query = query_start == std::string::npos ? "" : without_fragment.substr(query_start + 1);
    if (path.empty()) path = "/";

    if (host == "github.com") {
        const auto first = path.find_first_not_of('/');
        if (first != std::string::npos) {
            const auto slash = path.find('/', first);
            if (slash != std::string::npos && slash + 1 < path.size()) {
                const auto next = path.find('/', slash + 1);
                const auto owner = path.substr(first, slash - first);
                const auto repo = path.substr(slash + 1, next == std::string::npos ? std::string::npos : next - slash - 1);
                if (!owner.empty() && !repo.empty()) return "https://github.com/" + owner + "/" + repo;
            }
        }
    }

    const auto filtered_query = normalized_query(query);
    std::string output = scheme + "://" + host + path;
    if (!filtered_query.empty()) output += '?' + filtered_query;
    return output;
}

std::string clean_title(const std::string& title) {
    std::string output;
    bool previous_was_space = true;
    for (const unsigned char character : title) {
        if (std::isspace(character)) {
            previous_was_space = true;
        } else {
            if (!output.empty() && previous_was_space) output.push_back(' ');
            output.push_back(static_cast<char>(character));
            previous_was_space = false;
        }
    }
    return output;
}

}  // namespace techpulse::normalize

