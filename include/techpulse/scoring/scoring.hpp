#pragma once

#include "techpulse/config/radar_config.hpp"
#include "techpulse/model/scored_item.hpp"

#include <chrono>
#include <vector>

namespace techpulse::scoring {

model::ScoredItem score_item(const model::RadarItem& item, const config::RadarConfig& config,
                             std::chrono::sys_seconds now);
std::vector<model::ScoredItem> rank_items(std::vector<model::ScoredItem> items);

}  // namespace techpulse::scoring

