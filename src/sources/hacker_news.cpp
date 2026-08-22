#include "techpulse/sources/hacker_news.hpp"

#include "techpulse/normalize/normalize.hpp"

#include <algorithm>
#include <cctype>
#include <ctime>
#include <sstream>

namespace techpulse::sources {
namespace {
constexpr const char* kApi = "https://hacker-news.firebaseio.com/v0/";

std::size_t value_start(const std::string& json, const std::string& name) {
    const auto key = json.find("\"" + name + "\"");
    if (key == std::string::npos) return std::string::npos;
    const auto colon = json.find(':', key + name.size() + 2);
    return colon == std::string::npos ? colon : json.find_first_not_of(" \t\r\n", colon + 1);
}

std::string string_value(const std::string& json, const std::string& name) {
    auto start = value_start(json, name);
    if (start == std::string::npos || json[start] != '"') return {};
    ++start;
    std::string value;
    bool escaped = false;
    for (; start < json.size(); ++start) {
        const char current = json[start];
        if (escaped) {
            value.push_back(current == 'n' ? '\n' : current == 'r' ? '\r' : current == 't' ? '\t' : current);
            escaped = false;
        }
        else if (current == '\\') escaped = true;
        else if (current == '"') return value;
        else value.push_back(current);
    }
    return {};
}

int integer_value(const std::string& json, const std::string& name, int fallback = 0) {
    const auto start = value_start(json, name);
    if (start == std::string::npos) return fallback;
    try { return std::stoi(json.substr(start)); } catch (...) { return fallback; }
}

bool bool_value(const std::string& json, const std::string& name) {
    const auto start = value_start(json, name);
    return start != std::string::npos && json.compare(start, 4, "true") == 0;
}

std::vector<int> story_ids(const std::string& json) {
    std::vector<int> ids;
    std::size_t position = 0;
    while (position < json.size()) {
        if (!std::isdigit(static_cast<unsigned char>(json[position]))) { ++position; continue; }
        const auto end = json.find_first_not_of("0123456789", position);
        ids.push_back(std::stoi(json.substr(position, end - position)));
        position = end;
    }
    return ids;
}

std::string iso_time(int epoch) {
    const std::time_t time = epoch;
    std::tm utc{};
#ifdef _WIN32
    gmtime_s(&utc, &time);
#else
    gmtime_r(&time, &utc);
#endif
    char buffer[21]{};
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
    return buffer;
}

}  // namespace

FetchResult HackerNewsSource::fetch(const HttpGet& get) const {
    FetchResult result;
    for (const std::string feed : {"topstories", "beststories", "showstories", "jobstories"}) {
        const auto response = get(std::string(kApi) + feed + ".json");
        if (response.status != 200) { result.errors.push_back(feed + " returned HTTP " + std::to_string(response.status)); continue; }
        auto ids = story_ids(response.body);
        ids.resize(std::min(ids.size(), candidates_per_feed_));
        for (const int id : ids) {
            const auto detail = get(std::string(kApi) + "item/" + std::to_string(id) + ".json");
            if (detail.status != 200 || detail.body == "null" || bool_value(detail.body, "deleted") || bool_value(detail.body, "dead")) continue;
            const auto title = normalize::clean_title(string_value(detail.body, "title"));
            if (title.empty()) continue;
            model::RadarItem item;
            item.source_type = feed == "jobstories" ? "job" : "hacker_news";
            item.source_id = std::to_string(id);
            item.id = "hn:" + item.source_id;
            item.title = title;
            item.url = normalize::canonical_url(string_value(detail.body, "url"));
            item.discussion_url = "https://news.ycombinator.com/item?id=" + item.source_id;
            item.summary_raw = string_value(detail.body, "text");
            item.published_at = iso_time(integer_value(detail.body, "time"));
            item.metrics.points = integer_value(detail.body, "score");
            item.metrics.comments = integer_value(detail.body, "descendants");
            item.provenance.push_back({"hacker_news", item.source_id, item.discussion_url});
            result.items.push_back(std::move(item));
        }
    }
    return result;
}

}  // namespace techpulse::sources
