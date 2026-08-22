#include "techpulse/dedup/deduplicate.hpp"

#include "techpulse/normalize/normalize.hpp"

#include <algorithm>
#include <map>
#include <tuple>
#include <utility>

namespace techpulse::dedup {
namespace {

model::Provenance own_provenance(const model::RadarItem& item) {
    return {item.source_type, item.source_id, item.url};
}

void append_provenance(model::RadarItem& target, const model::RadarItem& source) {
    auto incoming = source.provenance;
    incoming.push_back(own_provenance(source));
    for (const auto& candidate : incoming) {
        const auto exists = std::any_of(target.provenance.begin(), target.provenance.end(), [&](const auto& current) {
            return std::tie(current.source, current.source_id, current.url) ==
                   std::tie(candidate.source, candidate.source_id, candidate.url);
        });
        if (!exists) target.provenance.push_back(candidate);
    }
}

int completeness(const model::RadarItem& item) {
    return static_cast<int>(!item.summary_raw.empty()) + static_cast<int>(!item.discussion_url.empty()) +
           static_cast<int>(item.metrics.stars.has_value()) + static_cast<int>(item.metrics.points.has_value()) +
           static_cast<int>(item.metrics.comments.has_value());
}

bool should_replace(const model::RadarItem& current, const model::RadarItem& candidate) {
    const auto current_key = std::tie(current.source_type, current.source_id, current.id);
    const auto candidate_key = std::tie(candidate.source_type, candidate.source_id, candidate.id);
    return completeness(candidate) > completeness(current) ||
           (completeness(candidate) == completeness(current) && candidate_key < current_key);
}

void merge_item(model::RadarItem& target, model::RadarItem candidate) {
    candidate.url = normalize::canonical_url(candidate.url);
    append_provenance(candidate, target);
    append_provenance(candidate, candidate);
    if (should_replace(target, candidate)) {
        target = std::move(candidate);
        return;
    }

    append_provenance(target, candidate);
    for (const auto& topic : candidate.topics) {
        if (std::find(target.topics.begin(), target.topics.end(), topic) == target.topics.end()) target.topics.push_back(topic);
    }
    if (!target.metrics.stars && candidate.metrics.stars) target.metrics.stars = candidate.metrics.stars;
    if (!target.metrics.points && candidate.metrics.points) target.metrics.points = candidate.metrics.points;
    if (!target.metrics.comments && candidate.metrics.comments) target.metrics.comments = candidate.metrics.comments;
}

}  // namespace

DeduplicationResult deduplicate_exact(std::vector<model::RadarItem> items) {
    DeduplicationResult result;
    std::map<std::pair<std::string, std::string>, std::size_t> source_keys;
    std::map<std::string, std::size_t> urls;

    for (auto& item : items) {
        item.url = normalize::canonical_url(item.url);
        const auto source_key = std::make_pair(item.source_type, item.source_id);
        if (const auto found = source_keys.find(source_key); found != source_keys.end()) {
            merge_item(result.items[found->second], std::move(item));
            ++result.source_key_merges;
            continue;
        }
        if (!item.url.empty()) {
            if (const auto found = urls.find(item.url); found != urls.end()) {
                merge_item(result.items[found->second], std::move(item));
                source_keys.emplace(source_key, found->second);
                ++result.url_merges;
                continue;
            }
        }

        append_provenance(item, item);
        source_keys.emplace(source_key, result.items.size());
        if (!item.url.empty()) urls.emplace(item.url, result.items.size());
        result.items.push_back(std::move(item));
    }
    return result;
}

}  // namespace techpulse::dedup
