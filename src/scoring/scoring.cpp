#include "techpulse/scoring/scoring.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <string>
#include <tuple>

namespace techpulse::scoring {
namespace {

constexpr double kNeutralMomentum = 50.0;

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) { return std::tolower(c); });
    return value;
}

bool contains(const std::string& text, const std::string& term) {
    return lower(text).find(lower(term)) != std::string::npos;
}

double clamp(double value) {
    return std::clamp(value, 0.0, 100.0);
}

std::chrono::sys_seconds parse_utc(const std::string& value) {
    std::tm parsed{};
    std::istringstream stream(value);
    stream >> std::get_time(&parsed, "%Y-%m-%dT%H:%M:%SZ");
    if (stream.fail()) return {};
    const auto day = std::chrono::year{parsed.tm_year + 1900} /
                     std::chrono::month{static_cast<unsigned>(parsed.tm_mon + 1)} /
                     std::chrono::day{static_cast<unsigned>(parsed.tm_mday)};
    if (!day.ok()) return {};
    return std::chrono::sys_days(day) + std::chrono::hours(parsed.tm_hour) +
           std::chrono::minutes(parsed.tm_min) + std::chrono::seconds(parsed.tm_sec);
}

}  // namespace

model::ScoredItem score_item(const model::RadarItem& item, const config::RadarConfig& config,
                             std::chrono::sys_seconds now) {
    model::ScoredItem scored{.item = item};
    const auto searchable = item.title + "\n" + item.summary_raw;
    double weighted_hits = 0.0;
    double available_weight = 0.0;

    for (const auto& topic : config.topics) {
        available_weight += topic.weight;
        for (const auto& excluded : topic.exclude) {
            if (contains(searchable, excluded)) {
                scored.excluded = true;
                scored.reasons.push_back("命中排除词 " + excluded);
            }
        }
        bool hit = false;
        for (const auto& included : topic.include) {
            if (contains(item.title, included)) {
                weighted_hits += topic.weight;
                scored.item.topics.push_back(topic.id);
                scored.reasons.push_back("标题命中高权重主题 " + topic.name);
                hit = true;
                break;
            }
            if (contains(item.summary_raw, included)) hit = true;
        }
        if (hit && std::find(scored.item.topics.begin(), scored.item.topics.end(), topic.id) == scored.item.topics.end()) {
            weighted_hits += topic.weight * 0.6;
            scored.item.topics.push_back(topic.id);
            scored.reasons.push_back("摘要命中主题 " + topic.name);
        }
    }
    scored.score.relevance = available_weight == 0.0 ? 0.0 : clamp(100.0 * weighted_hits / available_weight);

    const auto published = parse_utc(item.published_at);
    const auto age = published.time_since_epoch().count() == 0 ? std::chrono::hours(48) : now - published;
    const auto age_hours = std::max(0.0, std::chrono::duration<double, std::ratio<3600>>(age).count());
    const auto half_life = item.source_type == "github" ? 24.0 * 7.0 : (item.source_type == "job" ? 24.0 * 14.0 : 48.0);
    scored.score.freshness = clamp(100.0 * std::exp(-age_hours / half_life));
    if (age_hours <= 24.0) scored.reasons.push_back("24 小时内发布");

    if (item.source_type == "github" && item.metrics.stars) {
        scored.score.quality = clamp(100.0 * std::log1p(*item.metrics.stars) / std::log1p(100000.0));
        scored.score.momentum = kNeutralMomentum;
    } else if (item.source_type == "hacker_news" && item.metrics.points) {
        scored.score.quality = clamp(static_cast<double>(*item.metrics.points));
        scored.score.momentum = clamp(static_cast<double>(item.metrics.comments.value_or(0)) * 5.0);
    } else {
        scored.score.quality = 50.0;
        scored.score.momentum = kNeutralMomentum;
    }
    scored.score.diversity = 100.0;
    scored.score.total = clamp(scored.score.relevance * 0.40 + scored.score.freshness * 0.20 +
                               scored.score.quality * 0.15 + scored.score.momentum * 0.15 + scored.score.diversity * 0.10);
    if (scored.excluded) scored.score.total = 0.0;
    if (scored.reasons.empty()) scored.reasons.push_back("来源质量和时效性满足基础阈值");
    return scored;
}

std::vector<model::ScoredItem> rank_items(std::vector<model::ScoredItem> items) {
    std::stable_sort(items.begin(), items.end(), [](const auto& left, const auto& right) {
        if (left.score.total != right.score.total) return left.score.total > right.score.total;
        return std::tie(left.item.published_at, left.item.id, left.item.source_id) >
               std::tie(right.item.published_at, right.item.id, right.item.source_id);
    });
    return items;
}

}  // namespace techpulse::scoring
