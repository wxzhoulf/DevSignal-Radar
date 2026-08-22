#include "techpulse/scoring/scoring.hpp"

#include <chrono>
#include <iostream>

namespace {
int failures = 0;

void expect(bool condition, const char* message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

techpulse::config::RadarConfig config() {
    return {1, "Asia/Shanghai", "zh-CN", {{"cpp", "C++", 1.0, {"C++", "compiler"}, {"game tutorial"}}}, {10, 30, 45}};
}

techpulse::model::RadarItem item(const char* title, const char* published_at) {
    techpulse::model::RadarItem result;
    result.source_type = "github";
    result.source_id = title;
    result.title = title;
    result.published_at = published_at;
    result.metrics.stars = 1000;
    return result;
}
}

int main() {
    const auto now = std::chrono::sys_days(std::chrono::year{2026} / 8 / 22) + std::chrono::hours(12);
    const auto relevant = techpulse::scoring::score_item(item("C++ compiler update", "2026-08-22T10:00:00Z"), config(), now);
    const auto less_relevant = techpulse::scoring::score_item(item("Generic project", "2026-08-22T10:00:00Z"), config(), now);
    expect(relevant.score.total >= 0.0 && relevant.score.total <= 100.0, "total is bounded");
    expect(relevant.score.relevance > less_relevant.score.relevance, "topic match increases relevance");
    expect(!relevant.reasons.empty(), "scored item has an explanation");

    const auto recent = techpulse::scoring::score_item(item("C++ compiler update", "2026-08-22T10:00:00Z"), config(), now);
    const auto old = techpulse::scoring::score_item(item("C++ compiler update", "2026-08-12T10:00:00Z"), config(), now);
    expect(recent.score.freshness > old.score.freshness, "older publication reduces freshness");

    const auto excluded = techpulse::scoring::score_item(item("C++ game tutorial", "2026-08-22T10:00:00Z"), config(), now);
    expect(excluded.excluded && excluded.score.total == 0.0, "exclude terms prevent selection");
    return failures == 0 ? 0 : 1;
}

