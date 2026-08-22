#pragma once

#include "techpulse/model/radar_item.hpp"

#include <string>
#include <vector>

namespace techpulse::model {

struct ScoreBreakdown {
    double total{};
    double relevance{};
    double freshness{};
    double quality{};
    double momentum{};
    double diversity{};
};

struct ScoredItem {
    RadarItem item;
    ScoreBreakdown score;
    std::vector<std::string> reasons;
    bool excluded{};
};

}  // namespace techpulse::model

