#pragma once

#include <optional>
#include <string>
#include <vector>

namespace techpulse::model {

struct Provenance {
    std::string source;
    std::string source_id;
    std::string url;
};

struct Metrics {
    std::optional<int> stars;
    std::optional<int> points;
    std::optional<int> comments;
};

struct RadarItem {
    std::string id;
    std::string source_type;
    std::string source_id;
    std::string title;
    std::string url;
    std::string discussion_url;
    std::string summary_raw;
    std::string published_at;
    std::string collected_at;
    std::vector<std::string> topics;
    Metrics metrics;
    std::vector<Provenance> provenance;
};

}  // namespace techpulse::model
